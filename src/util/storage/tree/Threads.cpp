// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "util/storage/tree/Threads.hpp"  // IWYU pragma: associated

#include <StorageBlockchainTransactions.pb.h>
#include <StorageItemHash.pb.h>
#include <StorageNymList.pb.h>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/StorageBlockchainTransactions.hpp"
#include "internal/serialization/protobuf/verify/StorageNymList.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"
#include "util/storage/tree/Node.hpp"
#include "util/storage/tree/Thread.hpp"

namespace opentxs::storage
{
Threads::Threads(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const Driver& storage,
    const UnallocatedCString& hash,
    Mailbox& mailInbox,
    Mailbox& mailOutbox)
    : Node(crypto, factory, storage, hash)
    , threads_()
    , mail_inbox_(mailInbox)
    , mail_outbox_(mailOutbox)
    , blockchain_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(3);
    }
}

auto Threads::AddIndex(
    const Data& txid,
    const identifier::Generic& thread) noexcept -> bool
{
    Lock lock(blockchain_.lock_);

    OT_ASSERT(false == txid.empty());

    auto& vector = blockchain_.map_[txid];

    if (thread.empty()) {
        if (0 < vector.size()) { vector.clear(); }

        OT_ASSERT(0 == vector.size());
    } else {
        for (auto i = vector.begin(); i != vector.end();) {
            if ((*i).empty()) {
                i = vector.erase(i);
            } else {
                ++i;
            }
        }

        vector.emplace(thread);
    }

    return true;
}

auto Threads::BlockchainThreadMap(const Data& txid) const noexcept
    -> UnallocatedVector<identifier::Generic>
{
    auto output = UnallocatedVector<identifier::Generic>{};
    Lock lock(blockchain_.lock_);

    try {
        const auto& data = blockchain_.map_.at(txid);
        std::copy(std::begin(data), std::end(data), std::back_inserter(output));
    } catch (...) {
    }

    return output;
}

auto Threads::BlockchainTransactionList() const noexcept
    -> UnallocatedVector<ByteArray>
{
    auto output = UnallocatedVector<ByteArray>{};
    Lock lock(blockchain_.lock_);
    std::transform(
        std::begin(blockchain_.map_),
        std::end(blockchain_.map_),
        std::back_inserter(output),
        [&](const auto& in) -> auto{ return in.first; });

    return output;
}

auto Threads::create(
    const Lock& lock,
    const UnallocatedCString& id,
    const UnallocatedSet<UnallocatedCString>& participants)
    -> UnallocatedCString
{
    OT_ASSERT(verify_write_lock(lock));

    std::unique_ptr<storage::Thread> newThread(new storage::Thread(
        crypto_,
        factory_,
        driver_,
        id,
        participants,
        mail_inbox_,
        mail_outbox_));

    if (!newThread) {
        std::cerr << __func__ << ": Failed to instantiate thread." << std::endl;
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
        LogError()(OT_PRETTY_CLASS())("Thread already exists.").Flush();
    }

    return id;
}

auto Threads::Create(
    const UnallocatedCString& id,
    const UnallocatedSet<UnallocatedCString>& participants)
    -> UnallocatedCString
{
    Lock lock(write_lock_);

    return create(lock, id, participants);
}

auto Threads::Exists(const UnallocatedCString& id) const -> bool
{
    std::unique_lock<std::mutex> lock(write_lock_);

    return item_map_.find(id) != item_map_.end();
}

auto Threads::FindAndDeleteItem(const UnallocatedCString& itemID) -> bool
{
    std::unique_lock<std::mutex> lock(write_lock_);

    bool found = false;

    for (const auto& index : item_map_) {
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

void Threads::init(const UnallocatedCString& hash)
{
    auto input = std::shared_ptr<proto::StorageNymList>{};
    driver_.LoadProto(hash, input);

    if (!input) {
        std::cerr << __func__ << ": Failed to load thread list index file."
                  << std::endl;
        abort();
    }

    init_version(3, *input);

    for (const auto& it : input->nym()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    Lock lock(blockchain_.lock_);

    for (const auto& hash : input->localnymid()) {
        try {
            auto index =
                std::shared_ptr<proto::StorageBlockchainTransactions>{};
            auto loaded = driver_.LoadProto(hash, index, false);

            OT_ASSERT(loaded && index);

            auto txid = ByteArray{};
            txid.Assign(index->txid());

            OT_ASSERT(false == txid.empty());

            auto& data = blockchain_.map_[std::move(txid)];

            for (const auto& thread : index->thread()) {
                auto threadID = identifier::Generic{};
                threadID.Assign(thread);
                data.emplace(std::move(threadID));
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            continue;
        }
    }
}

auto Threads::List(const bool unreadOnly) const -> ObjectList
{
    if (false == unreadOnly) { return ot_super::List(); }

    ObjectList output{};
    Lock lock(write_lock_);

    for (const auto& it : item_map_) {
        const auto& threadID = it.first;
        const auto& alias = std::get<1>(it.second);
        auto* thread = Threads::thread(threadID, lock);

        OT_ASSERT(nullptr != thread);

        if (0 < thread->UnreadCount()) { output.push_back({threadID, alias}); }
    }

    return output;
}

auto Threads::Migrate(const Driver& to) const -> bool
{
    bool output{true};

    for (const auto& index : item_map_) {
        const auto& id = index.first;
        const auto& node = *thread(id);
        output &= node.Migrate(to);
    }

    output &= migrate(root_, to);

    return output;
}

auto Threads::mutable_Thread(const UnallocatedCString& id)
    -> Editor<storage::Thread>
{
    std::function<void(storage::Thread*, std::unique_lock<std::mutex>&)>
        callback = [&](storage::Thread* in,
                       std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, id);
    };

    return {write_lock_, thread(id), callback};
}

auto Threads::thread(const UnallocatedCString& id) const -> storage::Thread*
{
    std::unique_lock<std::mutex> lock(write_lock_);

    return thread(id, lock);
}

auto Threads::thread(
    const UnallocatedCString& id,
    const std::unique_lock<std::mutex>& lock) const -> storage::Thread*
{
    if (!verify_write_lock(lock)) {
        std::cerr << __func__ << ": Lock failure." << std::endl;
        abort();
    }

    const auto index = item_map_[id];
    const auto hash = std::get<0>(index);
    const auto alias = std::get<1>(index);
    auto& node = threads_[id];

    if (!node) {
        node.reset(new storage::Thread(
            crypto_,
            factory_,
            driver_,
            id,
            hash,
            alias,
            mail_inbox_,
            mail_outbox_));

        if (!node) {
            std::cerr << __func__ << ": Failed to instantiate thread."
                      << std::endl;
            abort();
        }
    }

    return node.get();
}

auto Threads::Thread(const UnallocatedCString& id) const
    -> const storage::Thread&
{
    return *thread(id);
}

auto Threads::Rename(
    const UnallocatedCString& existingID,
    const UnallocatedCString& newID) -> bool
{
    Lock lock(write_lock_);

    auto it = item_map_.find(existingID);

    if (item_map_.end() == it) {
        LogError()(OT_PRETTY_CLASS())("Thread ")(existingID)(" does not exist.")
            .Flush();

        return false;
    }

    auto meta = it->second;

    if (nullptr == thread(existingID, lock)) { return false; }

    auto threadItem = threads_.find(existingID);

    OT_ASSERT(threads_.end() != threadItem);

    auto& oldThread = threadItem->second;

    OT_ASSERT(oldThread);

    std::unique_ptr<storage::Thread> newThread{nullptr};

    if (false == oldThread->Rename(newID)) {
        LogError()(OT_PRETTY_CLASS())("Failed to rename thread ")(
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

auto Threads::RemoveIndex(
    const Data& txid,
    const identifier::Generic& thread) noexcept -> void
{
    Lock lock(blockchain_.lock_);
    auto it = blockchain_.map_.find(txid);

    if (blockchain_.map_.end() != it) {
        auto& data = it->second;
        data.erase(thread);

        if (data.empty()) { blockchain_.map_.erase(it); }
    }
}

auto Threads::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        std::cerr << __func__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

void Threads::save(
    storage::Thread* nym,
    const std::unique_lock<std::mutex>& lock,
    const UnallocatedCString& id)
{
    if (!verify_write_lock(lock)) {
        std::cerr << __func__ << ": Lock failure." << std::endl;
        abort();
    }

    if (nullptr == nym) {
        std::cerr << __func__ << ": Null target" << std::endl;
        abort();
    }

    auto& index = item_map_[id];
    auto& hash = std::get<0>(index);
    auto& alias = std::get<1>(index);
    hash = nym->Root();
    alias = nym->Alias();

    if (!save(lock)) {
        std::cerr << __func__ << ": Save error" << std::endl;
        abort();
    }
}

auto Threads::serialize() const -> proto::StorageNymList
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

    Lock lock(blockchain_.lock_);

    for (const auto& [txid, data] : blockchain_.map_) {
        if (data.empty()) { continue; }

        auto index = proto::StorageBlockchainTransactions{};
        index.set_version(1);
        index.set_txid(UnallocatedCString{txid.Bytes()});
        std::for_each(std::begin(data), std::end(data), [&](const auto& id) {
            OT_ASSERT(false == id.empty());

            index.add_thread(UnallocatedCString{id.Bytes()});
        });

        OT_ASSERT(static_cast<std::size_t>(index.thread_size()) == data.size());

        auto success = proto::Validate(index, VERBOSE);

        OT_ASSERT(success);

        auto hash = UnallocatedCString{};
        success = driver_.StoreProto(index, hash);

        OT_ASSERT(success);

        output.add_localnymid(hash);
    }

    return output;
}
}  // namespace opentxs::storage
