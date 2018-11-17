// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Root.hpp"

#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/Proto.hpp"

#include "storage/Plugin.hpp"
#include "BlockchainTransactions.hpp"
#include "Contacts.hpp"
#include "Credentials.hpp"
#include "Node.hpp"
#include "Nym.hpp"
#include "Nyms.hpp"
#include "Seeds.hpp"
#include "Servers.hpp"
#include "Tree.hpp"
#include "Units.hpp"

#define CURRENT_VERSION 2

//#define OT_METHOD "opentxs::storage::Root::"

namespace opentxs
{
namespace storage
{
Root::Root(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash,
    const std::int64_t interval,
    Flag& bucket)
    : ot_super(storage, hash)
    , gc_interval_(interval)
    , current_bucket_(bucket)
    , gc_running_(Flag::Factory(false))
    , gc_resume_(Flag::Factory(false))
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = CURRENT_VERSION;
        root_ = Node::BLANK_HASH;
        gc_root_ = Node::BLANK_HASH;
        current_bucket_.Off();
        last_gc_.store(static_cast<std::int64_t>(std::time(nullptr)));
        sequence_.store(0);
        tree_root_ = Node::BLANK_HASH;
    }
}

void Root::cleanup() const
{
    Lock gclock(gc_lock_);

    if (gc_thread_) {
        if (gc_thread_->joinable()) { gc_thread_->join(); }

        gc_thread_.reset();
    }
}

void Root::collect_garbage(const opentxs::api::storage::Driver* to) const
{
    Lock lock(write_lock_);
    LogOutput(OT_METHOD)(__FUNCTION__)(": Beginning garbage collection.")
        .Flush();
    const auto resume = gc_resume_->Set(false);
    bool oldLocation = false;

    if (resume) {
        oldLocation = !current_bucket_;
    } else {
        gc_root_ = tree()->Root();
        oldLocation = current_bucket_.Toggle();
        save(lock);
        driver_.StoreRoot(true, root_);
    }

    lock.unlock();
    bool success{false};

    if (Node::check_hash(gc_root_)) {
        const class Tree tree(driver_, gc_root_);
        success = tree.Migrate(*to);
    }

    if (success) {
        driver_.EmptyBucket(oldLocation);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Garbage collection failed. "
                                           "Will retry next cycle.")
            .Flush();
    }

    Lock gcLock(gc_lock_, std::defer_lock);
    std::lock(gcLock, lock);
    gc_running_->Off();
    gc_root_ = "";
    last_gc_.store(std::time(nullptr));
    save(lock);
    driver_.StoreRoot(true, root_);
    lock.unlock();
    gcLock.unlock();
    LogOutput(OT_METHOD)(__FUNCTION__)(": Finished garbage collection.")
        .Flush();
}

void Root::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageRoot> serialized;

    if (!driver_.LoadProto(hash, serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load root object file.")
            .Flush();
        OT_FAIL;
    }

    version_ = serialized->version();

    // Upgrade version
    if (CURRENT_VERSION > version_) { version_ = CURRENT_VERSION; }

    gc_root_ = normalize_hash(serialized->gcroot());
    current_bucket_.Set(serialized->altlocation());
    gc_running_->Off();
    gc_resume_->Set(serialized->gc());
    last_gc_.store(serialized->lastgc());
    sequence_.store(serialized->sequence());
    tree_root_ = normalize_hash(serialized->items());
}

bool Root::Migrate(const opentxs::api::storage::Driver& to) const
{
    if (0 == gc_interval_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Garbage collection disabled.")
            .Flush();

        return false;
    }

    const std::uint64_t time = std::time(nullptr);
    const bool intervalExceeded = ((time - last_gc_.load()) > gc_interval_);
    const bool resume = gc_resume_.get();
    const bool needToCollectGarbage = resume || intervalExceeded;

    if (needToCollectGarbage) {
        const auto running = gc_running_->Set(true);

        if (!running) {
            cleanup();
            gc_thread_.reset(
                new std::thread(&Root::collect_garbage, this, &to));

            return true;
        }
    }

    return false;
}

Editor<class Tree> Root::mutable_Tree()
{
    std::function<void(class Tree*, Lock&)> callback =
        [&](class Tree* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<class Tree>(write_lock_, tree(), callback);
}

bool Root::save(const Lock& lock, const opentxs::api::storage::Driver& to) const
{
    OT_ASSERT(verify_write_lock(lock));

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return to.StoreProto(serialized, root_);
}

bool Root::save(const Lock& lock) const
{
    OT_ASSERT(verify_write_lock(lock));

    sequence_++;

    return save(lock, driver_);
}

void Root::save(class Tree* tree, const Lock& lock)
{
    OT_ASSERT(verify_write_lock(lock));

    OT_ASSERT(nullptr != tree);

    Lock treeLock(tree_lock_);
    tree_root_ = tree->Root();
    treeLock.unlock();

    const bool saved = save(lock);

    OT_ASSERT(saved);
}

bool Root::Save(const opentxs::api::storage::Driver& to) const
{
    Lock lock(write_lock_);

    return save(lock, to);
}

std::uint64_t Root::Sequence() const { return sequence_.load(); }

proto::StorageRoot Root::serialize() const
{
    proto::StorageRoot output;
    output.set_version(version_);
    output.set_items(tree_root_);
    output.set_altlocation(current_bucket_);
    output.set_lastgc(last_gc_.load());
    output.set_gc(gc_running_.get());
    output.set_gcroot(gc_root_);
    output.set_sequence(sequence_);

    return output;
}

class Tree* Root::tree() const
{
    Lock lock(tree_lock_);

    if (!tree_) { tree_.reset(new class Tree(driver_, tree_root_)); }

    OT_ASSERT(tree_);

    lock.unlock();

    return tree_.get();
}

const class Tree& Root::Tree() const { return *tree(); }
}  // namespace storage
}  // namespace opentxs
