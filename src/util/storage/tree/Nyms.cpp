// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "1_Internal.hpp"              // IWYU pragma: associated
#include "util/storage/tree/Nyms.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <functional>
#include <mutex>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/Nym.hpp"
#include "internal/serialization/protobuf/verify/StorageNymList.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "serialization/protobuf/Nym.pb.h"
#include "serialization/protobuf/StorageItemHash.pb.h"
#include "serialization/protobuf/StorageNymList.pb.h"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"
#include "util/storage/tree/Nym.hpp"
#include "util/storage/tree/Thread.hpp"
#include "util/storage/tree/Threads.hpp"

namespace opentxs::storage
{
Nyms::Nyms(
    const Driver& storage,
    const UnallocatedCString& hash,
    const api::session::Factory& factory)
    : Node(storage, hash)
    , factory_(factory)
    , nyms_()
    , local_nyms_()
    , default_local_nym_(factory_.NymID())
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(current_version_);
    }
}

auto Nyms::Default() const -> OTNymID
{
    auto lock = Lock{write_lock_};
    LogTrace()(OT_PRETTY_CLASS())("Default nym is ")(default_local_nym_)
        .Flush();

    return default_local_nym_;
}

auto Nyms::Exists(const UnallocatedCString& id) const -> bool
{
    auto lock = Lock{write_lock_};

    return nyms_.find(id) != nyms_.end();
}

void Nyms::init(const UnallocatedCString& hash)
{
    std::shared_ptr<proto::StorageNymList> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        LogError()(OT_PRETTY_CLASS())("Failed to load nym list index file.")
            .Flush();
        abort();
    }

    init_version(current_version_, *serialized);

    for (const auto& it : serialized->nym()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    for (const auto& nymID : serialized->localnymid()) {
        local_nyms_.emplace(nymID);
    }

    if (serialized->has_defaultlocalnym()) {
        default_local_nym_ =
            factory_.InternalSession().NymID(serialized->defaultlocalnym());
    }

    if (default_local_nym_->empty() && (1u == local_nyms_.size())) {
        default_local_nym_ = factory_.NymID(*local_nyms_.begin());
        auto lock = Lock{write_lock_};
        save(lock);
    }
}

auto Nyms::LocalNyms() const -> const UnallocatedSet<UnallocatedCString>
{
    return local_nyms_;
}

void Nyms::Map(NymLambda lambda) const
{
    auto lock = Lock{write_lock_};
    const auto copy = item_map_;
    lock.unlock();

    for (const auto& it : copy) {
        const auto& id = it.first;
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
        const auto& id = index.first;
        const auto& node = *nym(id);
        output &= node.Migrate(to);
    }

    output &= migrate(root_, to);

    return output;
}

auto Nyms::mutable_Nym(const UnallocatedCString& id) -> Editor<storage::Nym>
{
    std::function<void(storage::Nym*, Lock&)> callback =
        [&](storage::Nym* in, Lock& lock) -> void { this->save(in, lock, id); };

    return Editor<storage::Nym>(write_lock_, nym(id), callback);
}

auto Nyms::nym(const UnallocatedCString& id) const -> storage::Nym*
{
    auto lock = Lock{write_lock_};

    return nym(lock, id);
}

auto Nyms::nym(const Lock& lock, const UnallocatedCString& id) const
    -> storage::Nym*
{
    OT_ASSERT(verify_write_lock(lock))

    const auto index = item_map_[id];
    const auto hash = std::get<0>(index);
    const auto alias = std::get<1>(index);
    auto& node = nyms_[id];

    if (!node) {
        node.reset(new storage::Nym(driver_, id, hash, alias));

        if (!node) {
            LogError()(OT_PRETTY_CLASS())("Failed to instantiate nym.").Flush();
            abort();
        }
    }

    return node.get();
}

auto Nyms::Nym(const UnallocatedCString& id) const -> const storage::Nym&
{
    return *nym(id);
}

auto Nyms::RelabelThread(
    const UnallocatedCString& threadID,
    const UnallocatedCString label) -> bool
{
    auto lock = Lock{write_lock_};
    UnallocatedSet<UnallocatedCString> nyms{};

    for (const auto& it : item_map_) {
        const auto& nymID = it.first;
        auto nym = Nyms::nym(lock, nymID);

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
        LogError()(OT_PRETTY_CLASS())("Lock failure.").Flush();
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    OT_ASSERT(current_version_ == serialized.version())

    return driver_.StoreProto(serialized, root_);
}

void Nyms::save(
    storage::Nym* nym,
    const Lock& lock,
    const UnallocatedCString& id)
{
    if (!verify_write_lock(lock)) {
        LogError()(OT_PRETTY_CLASS())("Lock failure.").Flush();
        abort();
    }

    if (nullptr == nym) {
        LogError()(OT_PRETTY_CLASS())("Null target.").Flush();
        abort();
    }

    auto& index = item_map_[id];
    auto& hash = std::get<0>(index);
    auto& alias = std::get<1>(index);
    hash = nym->Root();
    alias = nym->Alias();

    if (nym->private_.get()) { local_nyms_.emplace(nym->nymid_); }

    if (!save(lock)) {
        LogError()(OT_PRETTY_CLASS())("Save error.").Flush();
        abort();
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

    for (const auto& nymID : local_nyms_) { output.add_localnymid(nymID); }

    default_local_nym_->Serialize(*output.mutable_defaultlocalnym());

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

void Nyms::UpgradeLocalnym()
{
    auto lock = Lock{write_lock_};

    for (const auto& index : item_map_) {
        const auto& id = index.first;
        const auto& node = *nym(lock, id);
        auto credentials = std::make_shared<proto::Nym>();
        UnallocatedCString alias{};
        const auto loaded = node.Load(credentials, alias, false);

        if (false == loaded) { continue; }

        OT_ASSERT(node.checked_.get())

        if (node.private_.get()) {
            LogError()(OT_PRETTY_CLASS())("Adding nym ")(
                id)(" to local nym list.")
                .Flush();
            local_nyms_.emplace(id);
        } else {
            LogError()(OT_PRETTY_CLASS())("Nym ")(id)(" is not local.").Flush();
        }
    }

    save(lock);
}
}  // namespace opentxs::storage
