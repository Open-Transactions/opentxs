// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "storage/tree/Nyms.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <functional>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "internal/protobuf/Check.hpp"
#include "internal/protobuf/verify/Nym.hpp"
#include "internal/protobuf/verify/StorageNymList.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/storage/Driver.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/Nym.pb.h"
#include "serialization/protobuf/StorageItemHash.pb.h"
#include "serialization/protobuf/StorageNymList.pb.h"
#include "storage/Plugin.hpp"
#include "storage/tree/Node.hpp"
#include "storage/tree/Nym.hpp"
#include "storage/tree/Thread.hpp"
#include "storage/tree/Threads.hpp"

#define CURRENT_VERSION 3

namespace opentxs
{
namespace storage
{
Nyms::Nyms(const Driver& storage, const std::string& hash)
    : Node(storage, hash)
    , nyms_()
    , local_nyms_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(CURRENT_VERSION);
    }
}

auto Nyms::Exists(const std::string& id) const -> bool
{
    Lock lock(write_lock_);

    return nyms_.find(id) != nyms_.end();
}

void Nyms::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNymList> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        LogError()(OT_PRETTY_CLASS())("Failed to load nym list index file.")
            .Flush();
        abort();
    }

    init_version(CURRENT_VERSION, *serialized);

    for (const auto& it : serialized->nym()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    for (const auto& nymID : serialized->localnymid()) {
        local_nyms_.emplace(nymID);
    }
}

auto Nyms::LocalNyms() const -> const std::pmr::set<std::string>
{
    return local_nyms_;
}

void Nyms::Map(NymLambda lambda) const
{
    Lock lock(write_lock_);
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

auto Nyms::mutable_Nym(const std::string& id) -> Editor<storage::Nym>
{
    std::function<void(storage::Nym*, Lock&)> callback =
        [&](storage::Nym* in, Lock& lock) -> void { this->save(in, lock, id); };

    return Editor<storage::Nym>(write_lock_, nym(id), callback);
}

auto Nyms::nym(const std::string& id) const -> storage::Nym*
{
    Lock lock(write_lock_);

    return nym(lock, id);
}

auto Nyms::nym(const Lock& lock, const std::string& id) const -> storage::Nym*
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

auto Nyms::Nym(const std::string& id) const -> const storage::Nym&
{
    return *nym(id);
}

auto Nyms::RelabelThread(const std::string& threadID, const std::string label)
    -> bool
{
    Lock lock(write_lock_);
    std::pmr::set<std::string> nyms{};

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

    OT_ASSERT(CURRENT_VERSION == serialized.version())

    return driver_.StoreProto(serialized, root_);
}

void Nyms::save(storage::Nym* nym, const Lock& lock, const std::string& id)
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
    proto::StorageNymList serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_nym());
        }
    }

    for (const auto& nymID : local_nyms_) { serialized.add_localnymid(nymID); }

    return serialized;
}

void Nyms::UpgradeLocalnym()
{
    Lock lock(write_lock_);

    for (const auto& index : item_map_) {
        const auto& id = index.first;
        const auto& node = *nym(lock, id);
        auto credentials = std::make_shared<proto::Nym>();
        std::string alias{};
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
}  // namespace storage
}  // namespace opentxs
