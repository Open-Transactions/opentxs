// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "util/storage/tree/Nyms.hpp"  // IWYU pragma: associated

#include <Nym.pb.h>
#include <StorageItemHash.pb.h>
#include <StorageNymList.pb.h>
#include <functional>
#include <tuple>
#include <utility>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/Nym.hpp"
#include "internal/serialization/protobuf/verify/StorageNymList.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"
#include "util/storage/tree/Nym.hpp"
#include "util/storage/tree/Thread.hpp"
#include "util/storage/tree/Threads.hpp"

namespace opentxs::storage
{
Nyms::Nyms(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const Driver& storage,
    const UnallocatedCString& hash)
    : Node(crypto, factory, storage, hash)
    , nyms_()
    , local_nyms_()
    , default_local_nym_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(current_version_);
    }
}

auto Nyms::Default() const -> identifier::Nym
{
    auto lock = Lock{write_lock_};
    LogTrace()(OT_PRETTY_CLASS())("Default nym is ")(default_local_nym_)
        .Flush();

    return default_local_nym_;
}

auto Nyms::Exists(const identifier::Nym& id) const -> bool
{
    auto lock = Lock{write_lock_};

    return nyms_.find(id) != nyms_.end();
}

auto Nyms::init(const UnallocatedCString& hash) -> void
{
    const auto& log = LogTrace();
    auto serialized = std::shared_ptr<proto::StorageNymList>{};
    driver_.LoadProto(hash, serialized);

    if (false == serialized.operator bool()) {
        LogAbort()(OT_PRETTY_CLASS())("Failed to load nym list index file")
            .Abort();
    }

    const auto& proto = *serialized;
    init_version(current_version_, proto);
    log(OT_PRETTY_CLASS())("found ")(proto.nym().size())(" nyms").Flush();

    for (const auto& it : proto.nym()) {
        log(OT_PRETTY_CLASS())("loaded nym ")(it.itemid()).Flush();
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    log(OT_PRETTY_CLASS())("found ")(proto.localnymid().size())(" local nyms")
        .Flush();

    for (const auto& nymID : proto.localnymid()) {
        log(OT_PRETTY_CLASS())("indexed local nym ")(nymID).Flush();
        local_nyms_.emplace(factory_.NymIDFromBase58(nymID));
    }

    if (proto.has_defaultlocalnym()) {
        auto nym = factory_.InternalSession().NymID(proto.defaultlocalnym());
        log(OT_PRETTY_CLASS())("found default local nym ")(nym).Flush();
        default_local_nym_ = std::move(nym);
    }

    if (default_local_nym_.empty() && (1_uz == local_nyms_.size())) {
        const auto& nymID = *local_nyms_.begin();
        log(OT_PRETTY_CLASS())("setting default local nym to ")(nymID).Flush();
        auto lock = Lock{write_lock_};
        set_default(lock, nymID);
        save(lock);
    }
}

auto Nyms::LocalNyms() const noexcept -> Set<identifier::Nym>
{
    auto lock = Lock{write_lock_};

    return local_nyms_;
}

auto Nyms::Map(NymLambda lambda) const -> void
{
    auto lock = Lock{write_lock_};
    const auto copy = item_map_;
    lock.unlock();

    for (const auto& it : copy) {
        const auto id = factory_.NymIDFromBase58(it.first);
        const auto& node = *nym(id);
        const auto& hash = node.credentials_;

        std::shared_ptr<proto::Nym> serialized;

        if (Node::BLANK_HASH == hash) { continue; }

        if (driver_.LoadProto(hash, serialized, false)) { lambda(*serialized); }
    }
}

auto Nyms::Migrate(const Driver& to) const -> bool
{
    bool output{true};

    for (const auto& index : item_map_) {
        const auto id = factory_.NymIDFromBase58(index.first);
        const auto& node = *nym(id);
        output &= node.Migrate(to);
    }

    output &= migrate(root_, to);

    return output;
}

auto Nyms::mutable_Nym(const identifier::Nym& id) -> Editor<storage::Nym>
{
    std::function<void(storage::Nym*, Lock&)> callback =
        [&](storage::Nym* in, Lock& lock) -> void { this->save(in, lock, id); };

    return {write_lock_, nym(id), callback};
}

auto Nyms::NeedUpgrade() const noexcept -> bool { return UpgradeLevel() < 3u; }

auto Nyms::nym(const identifier::Nym& id) const -> storage::Nym*
{
    auto lock = Lock{write_lock_};

    return nym(lock, id);
}

auto Nyms::nym(const Lock& lock, const identifier::Nym& id) const
    -> storage::Nym*
{
    OT_ASSERT(verify_write_lock(lock));

    const auto& index = item_map_[id.asBase58(crypto_)];
    const auto hash = std::get<0>(index);
    const auto alias = std::get<1>(index);
    auto& nym = nyms_[id];

    if (false == nym.operator bool()) {
        nym = std::make_unique<storage::Nym>(
            crypto_, factory_, driver_, id.asBase58(crypto_), hash, alias);

        if (false == nym.operator bool()) {
            LogAbort()(OT_PRETTY_CLASS())("failed to instantiate storage nym ")(
                id)
                .Abort();
        }
    }

    return nym.get();
}

auto Nyms::Nym(const identifier::Nym& id) const -> const storage::Nym&
{
    return *nym(id);
}

auto Nyms::RelabelThread(
    const UnallocatedCString& threadID,
    const UnallocatedCString label) -> bool
{
    auto lock = Lock{write_lock_};
    auto nyms = Set<identifier::Nym>{};

    for (const auto& it : item_map_) {
        const auto nymID = factory_.NymIDFromBase58(it.first);
        auto* nym = Nyms::nym(lock, nymID);

        OT_ASSERT(nym);

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
    if (!verify_write_lock(lock)) {
        LogAbort()(OT_PRETTY_CLASS())("Lock failure.").Abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    OT_ASSERT(current_version_ == serialized.version());

    return driver_.StoreProto(serialized, root_);
}

auto Nyms::save(storage::Nym* nym, const Lock& lock, const identifier::Nym& id)
    -> void
{
    if (!verify_write_lock(lock)) {
        LogAbort()(OT_PRETTY_CLASS())("Lock failure").Abort();
    }

    if (nullptr == nym) {
        LogAbort()(OT_PRETTY_CLASS())("Null target").Abort();
    }

    auto& index = item_map_[id.asBase58(crypto_)];
    auto& hash = std::get<0>(index);
    auto& alias = std::get<1>(index);
    hash = nym->Root();
    alias = nym->Alias();

    if (nym->private_.get()) { local_nyms_.emplace(id); }

    if (false == save(lock)) {
        LogAbort()(OT_PRETTY_CLASS())("failed to save nym").Abort();
    }
}

auto Nyms::serialize() const -> proto::StorageNymList
{
    auto output = proto::StorageNymList{};
    output.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *output.add_nym());
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
    LogTrace()(OT_PRETTY_CLASS())("Default nym is ")(id).Flush();
    default_local_nym_ = id;
}

auto Nyms::Upgrade() noexcept -> void
{
    auto lock = Lock{write_lock_};

    switch (UpgradeLevel()) {
        case 1:
        case 2: {
            upgrade_create_local_nym_index(lock);
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("no upgrades needed").Flush();
        }
    }

    save(lock);
}

auto Nyms::upgrade_create_local_nym_index(const Lock& lock) noexcept -> void
{
    for (const auto& index : item_map_) {
        const auto id = factory_.NymIDFromBase58(index.first);
        const auto& node = *nym(lock, id);
        auto credentials = std::make_shared<proto::Nym>();
        auto alias = UnallocatedCString{};
        const auto loaded = node.Load(credentials, alias, false);

        if (false == loaded) { continue; }

        OT_ASSERT(node.checked_.get());

        if (node.private_.get()) {
            LogError()(OT_PRETTY_CLASS())("Adding nym ")(
                id)(" to local nym list.")
                .Flush();
            local_nyms_.emplace(id);
        }
    }
}

Nyms::~Nyms() = default;
}  // namespace opentxs::storage
