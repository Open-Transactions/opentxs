// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Root.hpp"  // IWYU pragma: associated

#include <StorageRoot.pb.h>
#include <functional>
#include <memory>
#include <stdexcept>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/StorageRoot.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"
#include "util/storage/tree/Tree.hpp"

namespace opentxs::storage
{
Root::Root(
    const api::network::Asio& asio,
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const Driver& storage,
    const UnallocatedCString& hash,
    const std::int64_t interval,
    Flag& bucket)
    : ot_super(crypto, factory, storage, hash)
    , current_bucket_(bucket)
    , sequence_()
    , gc_(asio, crypto_, factory_, driver_, interval)
    , tree_root_()
    , tree_lock_()
    , tree_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(current_version_);
    }
}

void Root::blank(const VersionNumber version)
{
    Node::blank(version);
    current_bucket_.Off();
    sequence_.store(0);
    tree_root_ = Node::BLANK_HASH;
}

void Root::cleanup() const { gc_.Cleanup(); }

void Root::init(const UnallocatedCString& hash)
{
    auto data = std::shared_ptr<proto::StorageRoot>{};

    if (!driver_.LoadProto(hash, data)) {
        LogError()(OT_PRETTY_CLASS())("Failed to load root object file.")
            .Flush();
        OT_FAIL;
    }

    init_version(current_version_, *data);
    current_bucket_.Set(data->altlocation());
    sequence_.store(data->sequence());
    tree_root_ = normalize_hash(data->items());

    if (auto root = normalize_hash(data->gcroot()); Node::check_hash(root)) {
        gc_.Init(root, data->gc(), data->lastgc());
    } else {
        gc_.Init({}, false, data->lastgc());
    }
}

auto Root::Migrate(const Driver& to) const -> bool
{
    try {
        const auto bucket = [&] {
            auto lock = Lock{write_lock_};
            auto out{false};

            switch (gc_.Check(tree()->Root())) {
                case GC::CheckState::Resume: {
                    out = !current_bucket_;
                } break;
                case GC::CheckState::Start: {
                    out = current_bucket_.Toggle();
                    save(lock);
                    driver_.StoreRoot(true, root_);
                } break;
                case GC::CheckState::Skip:
                default: {

                    throw std::runtime_error{
                        "garbage collection not necessary"};
                }
            }

            return out;
        }();

        return gc_.Run(bucket, to, [this] {
            auto lock = Lock{write_lock_};
            save(lock);
            driver_.StoreRoot(true, root_);
        });
    } catch (const std::exception& e) {
        LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Root::mutable_Tree() -> Editor<storage::Tree>
{
    std::function<void(storage::Tree*, Lock&)> callback =
        [&](storage::Tree* in, Lock& lock) -> void { this->save(in, lock); };

    return {write_lock_, tree(), callback};
}

auto Root::save(const Lock& lock, const Driver& to) const -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    auto serialized = serialize(lock);

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return to.StoreProto(serialized, root_);
}

auto Root::save(const Lock& lock) const -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    sequence_++;

    return save(lock, driver_);
}

void Root::save(storage::Tree* tree, const Lock& lock)
{
    OT_ASSERT(verify_write_lock(lock));

    OT_ASSERT(nullptr != tree);

    Lock treeLock(tree_lock_);
    tree_root_ = tree->root_;
    treeLock.unlock();

    const bool saved = save(lock);

    OT_ASSERT(saved);
}

auto Root::Save(const Driver& to) const -> bool
{
    Lock lock(write_lock_);

    return save(lock, to);
}

auto Root::Sequence() const -> std::uint64_t { return sequence_.load(); }

auto Root::serialize(const Lock&) const -> proto::StorageRoot
{
    auto output = proto::StorageRoot{};
    output.set_version(version_);
    output.set_items(tree_root_);
    output.set_altlocation(current_bucket_);
    output.set_sequence(sequence_);
    gc_.Serialize(output);

    return output;
}

auto Root::tree() const -> storage::Tree*
{
    Lock lock(tree_lock_);

    if (!tree_) {
        tree_.reset(new storage::Tree(crypto_, factory_, driver_, tree_root_));
    }

    OT_ASSERT(tree_);

    lock.unlock();

    return tree_.get();
}

auto Root::Tree() const -> const storage::Tree& { return *tree(); }
}  // namespace opentxs::storage
