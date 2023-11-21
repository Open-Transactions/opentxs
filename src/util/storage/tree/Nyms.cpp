// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Nyms.hpp"  // IWYU pragma: associated

#include <Nym.pb.h>
#include <StorageNymList.pb.h>
#include <atomic>
#include <functional>
#include <source_location>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <variant>

#include "internal/core/identifier/Identifier.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/StorageNymList.hpp"
#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/storage/Types.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Log.hpp"
#include "util/storage/tree/Node.hpp"
#include "util/storage/tree/Nym.hpp"
#include "util/storage/tree/Thread.hpp"
#include "util/storage/tree/Threads.hpp"

namespace opentxs::storage::tree
{
using namespace std::literals;

Nyms::Nyms(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const driver::Plugin& storage,
    const Hash& hash)
    : Node(
          crypto,
          factory,
          storage,
          hash,
          std::source_location::current().function_name(),
          current_version_)
    , nyms_()
    , local_nyms_()
    , default_local_nym_()
{
    if (is_valid(hash)) {
        init(hash);
    } else {
        blank();
    }
}

auto Nyms::Default() const -> identifier::Nym
{
    auto lock = Lock{write_lock_};
    LogTrace()()("Default nym is ")(default_local_nym_, crypto_).Flush();

    return default_local_nym_;
}

auto Nyms::dump(const Lock& lock, const Log& log, Vector<Hash>& out)
    const noexcept -> bool
{
    if (false == is_valid(root_)) { return true; }

    if (false == Node::dump(lock, log, out)) { return false; }

    for (const auto& index : item_map_) {
        const auto& id = factory_.Internal().NymIDConvertSafe(index.first);
        const auto& node = *nym(id);

        if (false == node.dump(lock, log, out)) { return false; }
    }

    return true;
}

auto Nyms::Exists(const identifier::Nym& id) const -> bool
{
    auto lock = Lock{write_lock_};

    return nyms_.find(id) != nyms_.end();
}

auto Nyms::init(const Hash& hash) noexcept(false) -> void
{
    auto p = std::shared_ptr<proto::StorageNymList>{};

    if (LoadProto(hash, p, verbose) && p) {
        const auto& proto = *p;

        switch (set_original_version(proto.version())) {
            case 5u:
            case 4u:
            case 3u:
            case 2u:
            case 1u:
            default: {
                init_map(proto.nym());

                for (const auto& nymID : proto.localnymid()) {
                    local_nyms_.emplace(factory_.NymIDFromBase58(nymID));
                }

                if (proto.has_defaultlocalnym()) {
                    auto nym = factory_.Internal().Session().NymID(
                        proto.defaultlocalnym());
                    default_local_nym_ = std::move(nym);
                }
            }
        }
    } else {
        throw std::runtime_error{"failed to load root object file in "s.append(
            std::source_location::current().function_name())};
    }
}

auto Nyms::LocalNyms() const noexcept -> Set<identifier::Nym>
{
    auto lock = Lock{write_lock_};

    return local_nyms_;
}

auto Nyms::mutable_Nym(const identifier::Nym& id) -> Editor<tree::Nym>
{
    std::function<void(tree::Nym*, Lock&)> callback =
        [&](tree::Nym* in, Lock& lock) -> void { this->save(in, lock, id); };

    return {write_lock_, nym(id), callback};
}

auto Nyms::NeedUpgrade() const noexcept -> bool { return UpgradeLevel() < 3u; }

auto Nyms::nym(const identifier::Nym& id) const -> tree::Nym*
{
    auto lock = Lock{write_lock_};

    return nym(lock, id);
}

auto Nyms::nym(const Lock& lock, const identifier::Nym& id) const -> tree::Nym*
{
    assert_true(verify_write_lock(lock));

    const auto& index = item_map_[id];
    const auto hash = std::get<0>(index);
    const auto alias = std::get<1>(index);
    auto& nym = nyms_[id];

    if (false == nym.operator bool()) {
        nym = std::make_unique<tree::Nym>(
            crypto_, factory_, plugin_, id, hash, alias);

        if (false == nym.operator bool()) {
            LogAbort()()("failed to instantiate storage nym ")(id, crypto_)
                .Abort();
        }
    }

    return nym.get();
}

auto Nyms::Nym(const identifier::Nym& id) const -> const tree::Nym&
{
    return *nym(id);
}

auto Nyms::RelabelThread(
    const identifier::Generic& threadID,
    const UnallocatedCString label) -> bool
{
    auto lock = Lock{write_lock_};
    auto nyms = Set<identifier::Nym>{};

    for (const auto& it : item_map_) {
        const auto nymID = factory_.Internal().NymIDConvertSafe(it.first);
        auto* nym = Nyms::nym(lock, nymID);

        assert_true(nym);

        const auto& threads = nym->Threads();

        if (threads.Exists(threadID)) { nyms.insert(nymID); }
    }

    lock.unlock();
    bool output{false};

    for (const auto& nymID : nyms) {
        auto nym = mutable_Nym(nymID);
        output |= nym.get()
                      .mutable_Threads()
                      .get()
                      .mutable_Thread(threadID)
                      .get()
                      .SetAlias(label);
    }

    // The for loop above takes care of saving

    return output;
}

auto Nyms::save(const Lock& lock) const -> bool
{
    if (!verify_write_lock(lock)) { LogAbort()()("Lock failure.").Abort(); }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    assert_true(current_version_ == serialized.version());

    return StoreProto(serialized, root_);
}

auto Nyms::save(tree::Nym* nym, const Lock& lock, const identifier::Nym& id)
    -> void
{
    if (!verify_write_lock(lock)) { LogAbort()()("Lock failure").Abort(); }

    if (nullptr == nym) { LogAbort()()("Null target").Abort(); }

    auto& index = item_map_[id];
    auto& hash = std::get<0>(index);
    auto& alias = std::get<1>(index);
    hash = nym->Root();
    alias = nym->Alias();

    if (nym->private_.get()) { local_nyms_.emplace(id); }

    if (false == save(lock)) { LogAbort()()("failed to save nym").Abort(); }
}

auto Nyms::serialize() const -> proto::StorageNymList
{
    auto output = proto::StorageNymList{};
    output.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = is_valid(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *output.add_nym());
        }
    }

    for (const auto& nymID : local_nyms_) {
        output.add_localnymid(nymID.asBase58(crypto_));
    }

    default_local_nym_.Internal().Serialize(*output.mutable_defaultlocalnym());

    return output;
}

auto Nyms::SetDefault(const identifier::Nym& id) -> bool
{
    auto lock = Lock{write_lock_};
    set_default(lock, id);

    return save(lock);
}

auto Nyms::set_default(const Lock&, const identifier::Nym& id) -> void
{
    LogTrace()()("Default nym is ")(id, crypto_).Flush();
    default_local_nym_ = id;
}

auto Nyms::upgrade(const Lock& lock) noexcept -> bool
{
    auto changed = Node::upgrade(lock);

    switch (original_version_.get()) {
        case 1u:
        case 2u: {
            upgrade_create_local_nym_index(lock);
        } break;
        case 3u:
        case 4u:
        case 5u:
        default: {
            if (default_local_nym_.empty() && (1_uz == local_nyms_.size())) {
                const auto& nymID = *local_nyms_.begin();
                set_default(lock, nymID);
                changed = true;
            }
        }
    }

    for (auto& [id, meta] : item_map_) {
        const auto nymID = factory_.Internal().NymIDConvertSafe(id);
        auto& hash = std::get<0>(meta);

        if (auto* node = nym(lock, nymID); node->Upgrade()) {
            hash = node->root_;
            changed = true;
        }
    }

    return changed;
}

auto Nyms::upgrade_create_local_nym_index(const Lock& lock) noexcept -> void
{
    for (const auto& index : item_map_) {
        const auto id = factory_.Internal().NymIDConvertSafe(index.first);
        const auto& node = *nym(lock, id);
        auto credentials = std::make_shared<proto::Nym>();
        auto alias = UnallocatedCString{};
        using enum ErrorReporting;
        const auto loaded = node.Load(credentials, alias, verbose);

        if (false == loaded) { continue; }

        assert_true(node.checked_.get());

        if (node.private_.get()) {
            LogError()()("Adding nym ")(id, crypto_)(" to local nym list.")
                .Flush();
            local_nyms_.emplace(id);
        }
    }
}

Nyms::~Nyms() = default;
}  // namespace opentxs::storage::tree
