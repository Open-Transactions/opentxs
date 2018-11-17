// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Threads.hpp"

#include "storage/Plugin.hpp"
#include "Thread.hpp"

#include <utility>
#include <string>
#include <memory>
#include <functional>
#include <map>

//#define OT_METHOD "opentxs::storage::Threads::"

namespace opentxs
{
namespace storage
{
Threads::Threads(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash,
    Mailbox& mailInbox,
    Mailbox& mailOutbox)
    : Node(storage, hash)
    , mail_inbox_(mailInbox)
    , mail_outbox_(mailOutbox)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        version_ = 2;
        root_ = Node::BLANK_HASH;
    }
}

std::string Threads::create(
    const Lock& lock,
    const std::string& id,
    const std::set<std::string>& participants)
{
    OT_ASSERT(verify_write_lock(lock));

    std::unique_ptr<class Thread> newThread(
        new class Thread(driver_, id, participants, mail_inbox_, mail_outbox_));

    if (!newThread) {
        std::cerr << __FUNCTION__ << ": Failed to instantiate thread."
                  << std::endl;
        abort();
    }

    const auto index = item_map_[id];
    const auto hash = std::get<0>(index);
    const auto alias = std::get<1>(index);
    auto& node = threads_[id];

    if (false == bool(node)) {
        Lock threadLock(newThread->write_lock_);
        newThread->save(threadLock);
        node.swap(newThread);
        save(lock);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Thread already exists.").Flush();
    }

    return id;
}

std::string Threads::Create(
    const std::string& id,
    const std::set<std::string>& participants)
{
    Lock lock(write_lock_);

    return create(lock, id, participants);
}

bool Threads::Exists(const std::string& id) const
{
    std::unique_lock<std::mutex> lock(write_lock_);

    return item_map_.find(id) != item_map_.end();
}

bool Threads::FindAndDeleteItem(const std::string& itemID)
{
    std::unique_lock<std::mutex> lock(write_lock_);

    bool found = false;

    for (const auto index : item_map_) {
        const auto& id = index.first;
        auto& node = *thread(id, lock);
        const bool hasItem = node.Check(itemID);

        if (hasItem) {
            node.Remove(itemID);
            found = true;
        }
    }

    if (found) { save(lock); }

    return found;
}

void Threads::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNymList> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __FUNCTION__ << ": Failed to load thread list index file."
                  << std::endl;
        abort();
    }

    version_ = serialized->version();

    // Upgrade to version 2
    if (2 > version_) { version_ = 2; }

    for (const auto& it : serialized->nym()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

ObjectList Threads::List(const bool unreadOnly) const
{
    if (false == unreadOnly) { return ot_super::List(); }

    ObjectList output{};
    Lock lock(write_lock_);

    for (const auto& it : item_map_) {
        const auto& threadID = it.first;
        const auto& alias = std::get<1>(it.second);
        auto thread = Threads::thread(threadID, lock);

        OT_ASSERT(nullptr != thread);

        if (0 < thread->UnreadCount()) { output.push_back({threadID, alias}); }
    }

    return output;
}

bool Threads::Migrate(const opentxs::api::storage::Driver& to) const
{
    bool output{true};

    for (const auto index : item_map_) {
        const auto& id = index.first;
        const auto& node = *thread(id);
        output &= node.Migrate(to);
    }

    output &= migrate(root_, to);

    return output;
}

Editor<class Thread> Threads::mutable_Thread(const std::string& id)
{
    std::function<void(class Thread*, std::unique_lock<std::mutex>&)> callback =
        [&](class Thread* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, id);
    };

    return Editor<class Thread>(write_lock_, thread(id), callback);
}

class Thread* Threads::thread(const std::string& id) const
{
    std::unique_lock<std::mutex> lock(write_lock_);

    return thread(id, lock);
}

class Thread* Threads::thread(
    const std::string& id,
    const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    const auto index = item_map_[id];
    const auto hash = std::get<0>(index);
    const auto alias = std::get<1>(index);
    auto& node = threads_[id];

    if (!node) {
        node.reset(new class Thread(
            driver_, id, hash, alias, mail_inbox_, mail_outbox_));

        if (!node) {
            std::cerr << __FUNCTION__ << ": Failed to instantiate thread."
                      << std::endl;
            abort();
        }
    }

    return node.get();
}

const class Thread& Threads::Thread(const std::string& id) const
{
    return *thread(id);
}

bool Threads::Rename(const std::string& existingID, const std::string& newID)
{
    Lock lock(write_lock_);

    auto it = item_map_.find(existingID);

    if (item_map_.end() == it) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Thread ")(existingID)(
            " does not exist.")
            .Flush();

        return false;
    }

    auto meta = it->second;

    if (nullptr == thread(existingID, lock)) { return false; }

    auto threadItem = threads_.find(existingID);

    OT_ASSERT(threads_.end() != threadItem);

    auto& oldThread = threadItem->second;

    OT_ASSERT(oldThread);

    std::unique_ptr<class Thread> newThread{nullptr};

    if (false == oldThread->Rename(newID)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to rename thread ")(
            existingID)(".")
            .Flush();

        return false;
    }

    newThread.reset(oldThread.release());
    threads_.erase(threadItem);
    threads_.emplace(
        newID, std::unique_ptr<opentxs::storage::Thread>(newThread.release()));
    item_map_.erase(it);
    item_map_.emplace(newID, meta);

    return save(lock);
}

bool Threads::save(const std::unique_lock<std::mutex>& lock) const
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

void Threads::save(
    class Thread* nym,
    const std::unique_lock<std::mutex>& lock,
    const std::string& id)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __FUNCTION__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == nym) {
        std::cerr << __FUNCTION__ << ": Null target" << std::endl;
        abort();
    }

    auto& index = item_map_[id];
    auto& hash = std::get<0>(index);
    auto& alias = std::get<1>(index);
    hash = nym->Root();
    alias = nym->Alias();

    if (!save(lock)) {
        std::cerr << __FUNCTION__ << ": Save error" << std::endl;
        abort();
    }
}

proto::StorageNymList Threads::serialize() const
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

    return serialized;
}
}  // namespace storage
}  // namespace opentxs
