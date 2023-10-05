// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Threads.hpp"  // IWYU pragma: associated

#include <StorageBlockchainTransactions.pb.h>
#include <StorageNymList.pb.h>
#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <variant>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/StorageBlockchainTransactions.hpp"
#include "internal/serialization/protobuf/verify/StorageNymList.hpp"
#include "internal/util/DeferredConstruction.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/storage/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/storage/tree/Node.hpp"
#include "util/storage/tree/Thread.hpp"

namespace opentxs::storage::tree
{
using namespace std::literals;

Threads::Threads(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const driver::Plugin& storage,
    const Hash& hash,
    Mailbox& mailInbox,
    Mailbox& mailOutbox)
    : Node(crypto, factory, storage, hash, OT_PRETTY_CLASS(), 3)
    , threads_()
    , mail_inbox_(mailInbox)
    , mail_outbox_(mailOutbox)
    , blockchain_()
{
    if (is_valid(hash)) {
        init(hash);
    } else {
        blank();
    }
}

auto Threads::AddIndex(
    const blockchain::block::TransactionHash& txid,
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

auto Threads::BlockchainThreadMap(
    const blockchain::block::TransactionHash& txid) const noexcept
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
        [&](const auto& in) -> auto { return in.first; });

    return output;
}

auto Threads::create(
    const Lock& lock,
    const identifier::Generic& id,
    const UnallocatedSet<identifier::Generic>& participants)
    -> identifier::Generic
{
    OT_ASSERT(verify_write_lock(lock));

    std::unique_ptr<tree::Thread> newThread(new tree::Thread(
        crypto_,
        factory_,
        plugin_,
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
    const identifier::Generic& id,
    const UnallocatedSet<identifier::Generic>& participants)
    -> identifier::Generic
{
    Lock lock(write_lock_);

    return create(lock, id, participants);
}

auto Threads::dump(const Lock& lock, const Log& log, Vector<Hash>& out)
    const noexcept -> bool
{
    if (false == is_valid(root_)) { return true; }

    if (false == Node::dump(lock, log, out)) { return false; }

    for (const auto& index : item_map_) {
        const auto& id = index.first;
        const auto& node = *thread(id);

        if (false == node.dump(lock, log, out)) { return false; }
    }

    auto index = std::shared_ptr<proto::StorageNymList>{};

    if (LoadProto(root_, index, verbose) && index) {
        out.reserve(out.size() + index->localnymid_size());

        for (const auto& id : index->localnymid()) {
            const auto hash = read(id);
            log(OT_PRETTY_CLASS())(name_)(
                ": adding blockchain transaction index hash ")(hash)
                .Flush();
            out.emplace_back(hash);
        }
    } else {
        LogError()(OT_PRETTY_CLASS())("failed to load root object file ")(root_)
            .Flush();

        return false;
    }

    return true;
}

auto Threads::Exists(const identifier::Generic& id) const -> bool
{
    std::unique_lock<std::mutex> lock(write_lock_);

    return item_map_.contains(id);
}

auto Threads::FindAndDeleteItem(const identifier::Generic& itemID) -> bool
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

auto Threads::init(const Hash& hash) noexcept(false) -> void
{
    auto p = std::shared_ptr<proto::StorageNymList>{};

    if (LoadProto(hash, p, verbose) && p) {
        const auto& proto = *p;

        switch (set_original_version(proto.version())) {
            case 3u:
            case 2u:
            case 1u:
            default: {
                init_map(proto.nym());

                for (const auto& id : proto.localnymid()) {
                    auto index =
                        std::shared_ptr<proto::StorageBlockchainTransactions>{};

                    if (LoadProto(read(id), index, verbose) && index) {
                        auto txid = ByteArray{};
                        txid.Assign(index->txid());

                        if (txid.empty()) {
                            throw std::runtime_error{"empty txid "s};
                        }

                        auto& data = blockchain_.map_[std::move(txid)];

                        for (const auto& thread : index->thread()) {
                            data.emplace(factory_.IdentifierFromHash(thread));
                        }
                    } else {
                        throw std::runtime_error{
                            "failed to load blockchain index object with hash "s
                                .append(to_string(read(id)))};
                    }
                }
            }
        }
    } else {
        throw std::runtime_error{
            "failed to load root object file in "s.append(OT_PRETTY_CLASS())};
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

        if (0 < thread->UnreadCount()) {
            output.emplace_back(threadID.asBase58(crypto_), alias);
        }
    }

    return output;
}

auto Threads::mutable_Thread(const identifier::Generic& id)
    -> Editor<tree::Thread>
{
    std::function<void(tree::Thread*, std::unique_lock<std::mutex>&)> callback =
        [&](tree::Thread* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, id);
    };

    return {write_lock_, thread(id), callback};
}

auto Threads::thread(const identifier::Generic& id) const -> tree::Thread*
{
    std::unique_lock<std::mutex> lock(write_lock_);

    return thread(id, lock);
}

auto Threads::thread(
    const identifier::Generic& id,
    const std::unique_lock<std::mutex>& lock) const -> tree::Thread*
{
    if (!verify_write_lock(lock)) {
        std::cerr << __func__ << ": Lock failure." << std::endl;
        abort();
    }

    const auto& index = item_map_[id];
    const auto hash = std::get<0>(index);
    const auto alias = std::get<1>(index);
    auto& node = threads_[id];

    if (!node) {
        node.reset(new tree::Thread(
            crypto_,
            factory_,
            plugin_,
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

auto Threads::Thread(const identifier::Generic& id) const -> const tree::Thread&
{
    return *thread(id);
}

auto Threads::Rename(
    const identifier::Generic& existingID,
    const identifier::Generic& newID) -> bool
{
    Lock lock(write_lock_);

    auto it = item_map_.find(existingID);

    if (item_map_.end() == it) {
        LogError()(OT_PRETTY_CLASS())("Thread ")(existingID, crypto_)(
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

    std::unique_ptr<tree::Thread> newThread{nullptr};

    if (false == oldThread->Rename(newID)) {
        LogError()(OT_PRETTY_CLASS())("Failed to rename thread ")(
            existingID, crypto_)(".")
            .Flush();

        return false;
    }

    newThread.reset(oldThread.release());
    threads_.erase(threadItem);
    threads_.emplace(newID, std::unique_ptr<tree::Thread>(newThread.release()));
    item_map_.erase(it);
    item_map_.emplace(newID, meta);

    return save(lock);
}

auto Threads::RemoveIndex(
    const blockchain::block::TransactionHash& txid,
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

    return StoreProto(serialized, root_);
}

void Threads::save(
    tree::Thread* nym,
    const std::unique_lock<std::mutex>& lock,
    const identifier::Generic& id)
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
        const bool goodHash = is_valid(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(item.first, item.second, *output.add_nym());
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

        auto hash = Hash{};
        success = StoreProto(index, hash);

        OT_ASSERT(success);

        write(hash, *output.add_localnymid());
    }

    return output;
}

auto Threads::upgrade(const Lock& lock) noexcept -> bool
{
    auto changed = Node::upgrade(lock);

    switch (original_version_.get()) {
        case 1u:
        case 2u:
        case 3u:
        default: {
        }
    }

    for (auto& [id, meta] : item_map_) {
        auto& hash = std::get<0>(meta);

        if (auto* node = thread(id, lock); node->Upgrade()) {
            hash = node->root_;
            changed = true;
        }
    }

    return changed;
}
}  // namespace opentxs::storage::tree
