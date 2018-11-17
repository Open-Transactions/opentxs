// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Nyms.hpp"

#include "storage/Plugin.hpp"
#include "Contexts.hpp"
#include "Mailbox.hpp"
#include "Nym.hpp"
#include "Thread.hpp"
#include "Threads.hpp"

#include <functional>

#define CURRENT_VERSION 3

//#define OT_METHOD "opentxs::storage::Nyms::"

namespace opentxs
{
namespace storage
{
Nyms::Nyms(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
    , nyms_()
    , local_nyms_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = CURRENT_VERSION;
        root_ = Node::BLANK_HASH;
    }
}

bool Nyms::Exists(const std::string& id) const
{
    Lock lock(write_lock_);

    return nyms_.find(id) != nyms_.end();
}

void Nyms::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNymList> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to load nym list index file.")
            .Flush();
        abort();
    }

    original_version_ = serialized->version();

    // Upgrade version
    if (CURRENT_VERSION > original_version_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Upgrading to version ")(
            CURRENT_VERSION)(".")
            .Flush();
        version_ = CURRENT_VERSION;
    } else {
        version_ = original_version_;
    }

    for (const auto& it : serialized->nym()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    for (const auto& nymID : serialized->localnymid()) {
        local_nyms_.emplace(nymID);
    }
}

const std::set<std::string> Nyms::LocalNyms() const { return local_nyms_; }

void Nyms::Map(NymLambda lambda) const
{
    Lock lock(write_lock_);
    const auto copy = item_map_;
    lock.unlock();

    for (const auto it : copy) {
        const auto& id = it.first;
        const auto& node = *nym(id);
        const auto& hash = node.credentials_;

        std::shared_ptr<proto::CredentialIndex> serialized;

        if (Node::BLANK_HASH == hash) { continue; }

        if (driver_.LoadProto(hash, serialized, false)) { lambda(*serialized); }
    }
}

bool Nyms::Migrate(const opentxs::api::storage::Driver& to) const
{
    bool output{true};

    for (const auto index : item_map_) {
        const auto& id = index.first;
        const auto& node = *nym(id);
        output &= node.Migrate(to);
    }

    output &= migrate(root_, to);

    return output;
}

Editor<class Nym> Nyms::mutable_Nym(const std::string& id)
{
    std::function<void(class Nym*, Lock&)> callback =
        [&](class Nym* in, Lock& lock) -> void { this->save(in, lock, id); };

    return Editor<class Nym>(write_lock_, nym(id), callback);
}

class Nym* Nyms::nym(const std::string& id) const
{
    Lock lock(write_lock_);

    return nym(lock, id);
}

class Nym* Nyms::nym(const Lock& lock, const std::string& id) const
{
    OT_ASSERT(verify_write_lock(lock))

    const auto index = item_map_[id];
    const auto hash = std::get<0>(index);
    const auto alias = std::get<1>(index);
    auto& node = nyms_[id];

    if (!node) {
        node.reset(new class Nym(driver_, id, hash, alias));

        if (!node) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate nym.")
                .Flush();
            abort();
        }
    }

    return node.get();
}

const class Nym& Nyms::Nym(const std::string& id) const { return *nym(id); }

bool Nyms::RelabelThread(const std::string& threadID, const std::string label)
{
    Lock lock(write_lock_);
    std::set<std::string> nyms{};

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
        output |= nym.It()
                      .mutable_Threads()
                      .It()
                      .mutable_Thread(threadID)
                      .It()
                      .SetAlias(label);
    }

    // The for loop above takes care of saving

    return output;
}

bool Nyms::save(const Lock& lock) const
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    OT_ASSERT(CURRENT_VERSION == serialized.version())

    return driver_.StoreProto(serialized, root_);
}

void Nyms::save(class Nym* nym, const Lock& lock, const std::string& id)
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Lock failure.").Flush();
        abort();
    }

    if (nullptr == nym) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Null target.").Flush();
        abort();
    }

    auto& index = item_map_[id];
    auto& hash = std::get<0>(index);
    auto& alias = std::get<1>(index);
    hash = nym->Root();
    alias = nym->Alias();

    if (nym->private_.get()) { local_nyms_.emplace(nym->nymid_); }

    if (!save(lock)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Save error.").Flush();
        abort();
    }
}

proto::StorageNymList Nyms::serialize() const
{
    proto::StorageNymList serialized;
    serialized.set_version(version_);

    for (const auto item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *serialized.add_nym());
        }
    }

    for (const auto& nymID : local_nyms_) { serialized.add_localnymid(nymID); }

    return serialized;
}

void Nyms::UpgradeLocalnym()
{
    Lock lock(write_lock_);

    for (const auto index : item_map_) {
        const auto& id = index.first;
        const auto& node = *nym(lock, id);
        auto credentials = std::make_shared<proto::CredentialIndex>();
        std::string alias{};
        const auto loaded = node.Load(credentials, alias, false);

        if (false == loaded) { continue; }

        OT_ASSERT(node.checked_.get())

        if (node.private_.get()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Adding nym ")(id)(
                " to local nym list.")
                .Flush();
            local_nyms_.emplace(id);
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Nym ")(id)(" is not local.")
                .Flush();
        }
    }

    save(lock);
}
}  // namespace storage
}  // namespace opentxs
