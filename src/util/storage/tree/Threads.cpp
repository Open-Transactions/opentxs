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
#include <iterator>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <variant>

#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/verify/StorageBlockchainTransactions.hpp"
#include "internal/serialization/protobuf/verify/StorageNymList.hpp"
#include "internal/util/DeferredConstruction.hpp"
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
    : Node(
          crypto,
          factory,
          storage,
          hash,
          std::source_location::current().function_name(),
          3)
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
    const auto lock = Lock{blockchain_.lock_};

    assert_false(txid.empty());

    auto& vector = blockchain_.map_[txid];

    if (thread.empty()) {
        if (0 < vector.size()) { vector.clear(); }

        assert_true(0 == vector.size());
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
    const auto lock = Lock{blockchain_.lock_};

    try {
        const auto& data = blockchain_.map_.at(txid);
        std::ranges::copy(data, std::back_inserter(output));
    } catch (...) {
    }

    return output;
}

auto Threads::BlockchainTransactionList() const noexcept
    -> UnallocatedVector<ByteArray>
{
    auto output = UnallocatedVector<ByteArray>{};
    const auto lock = Lock{blockchain_.lock_};
    std::ranges::transform(
        blockchain_.map_,
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
    assert_true(verify_write_lock(lock));

    std::unique_ptr<tree::Thread> newThread(new tree::Thread(
        crypto_,
        factory_,
        plugin_,
        id,
        participants,
        mail_inbox_,
        mail_outbox_));

    if (!newThread) { LogAbort()()("failed to instantiate thread").Abort(); }

    const auto index = item_map_[id];
    const auto hash = std::get<0>(index);
    const auto alias = std::get<1>(index);
    auto& node = threads_[id];

    if (false == bool(node)) {
        const auto threadLock = Lock{newThread->write_lock_};
        newThread->save(threadLock);
        node.swap(newThread);
        save(lock);
    } else {
        LogError()()("Thread already exists.").Flush();
    }

    return id;
}

auto Threads::Create(
    const identifier::Generic& id,
    const UnallocatedSet<identifier::Generic>& participants)
    -> identifier::Generic
{
    const auto lock = Lock{write_lock_};

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
            log()(name_)(": adding blockchain transaction index hash ")(hash)
                .Flush();
            out.emplace_back(hash);
        }
    } else {
        LogError()()("failed to load root object file ")(root_).Flush();

        return false;
    }

    return true;
}

auto Threads::Exists(const identifier::Generic& id) const -> bool
{
    const auto lock = Lock{write_lock_};

    return item_map_.contains(id);
}

auto Threads::FindAndDeleteItem(const identifier::Generic& itemID) -> bool
{
    const auto lock = Lock{write_lock_};

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
        throw std::runtime_error{"failed to load root object file in "s.append(
            std::source_location::current().function_name())};
    }
}

auto Threads::List(const bool unreadOnly) const -> ObjectList
{
    if (false == unreadOnly) { return ot_super::List(); }

    ObjectList output{};
    const auto lock = Lock{write_lock_};

    for (const auto& it : item_map_) {
        const auto& threadID = it.first;
        const auto& alias = std::get<1>(it.second);
        auto* thread = Threads::thread(threadID, lock);

        assert_false(nullptr == thread);

        if (0 < thread->UnreadCount()) {
            output.emplace_back(threadID.asBase58(crypto_), alias);
        }
    }

    return output;
}

auto Threads::mutable_Thread(const identifier::Generic& id)
    -> Editor<tree::Thread>
{
    const std::function<void(tree::Thread*, std::unique_lock<std::mutex>&)>
        callback =
            [&](tree::Thread* in, std::unique_lock<std::mutex>& lock) -> void {
        this->save(in, lock, id);
    };

    return {write_lock_, thread(id), callback};
}

auto Threads::thread(const identifier::Generic& id) const -> tree::Thread*
{
    const auto lock = Lock{write_lock_};

    return thread(id, lock);
}

auto Threads::thread(
    const identifier::Generic& id,
    const std::unique_lock<std::mutex>& lock) const -> tree::Thread*
{
    if (!verify_write_lock(lock)) { LogAbort()()("Lock failure").Abort(); }

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

        if (!node) { LogAbort()()("failed to instantiate thread").Abort(); }
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
    const auto lock = Lock{write_lock_};

    auto it = item_map_.find(existingID);

    if (item_map_.end() == it) {
        LogError()()("Thread ")(existingID, crypto_)(" does not exist.")
            .Flush();

        return false;
    }

    auto meta = it->second;

    if (nullptr == thread(existingID, lock)) { return false; }

    auto threadItem = threads_.find(existingID);

    assert_true(threads_.end() != threadItem);

    auto& oldThread = threadItem->second;

    assert_false(nullptr == oldThread);

    std::unique_ptr<tree::Thread> newThread{nullptr};

    if (false == oldThread->Rename(newID)) {
        LogError()()("Failed to rename thread ")(existingID, crypto_)(".")
            .Flush();

        return false;
    }

    newThread = std::move(oldThread);
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
    const auto lock = Lock{blockchain_.lock_};
    auto it = blockchain_.map_.find(txid);

    if (blockchain_.map_.end() != it) {
        auto& data = it->second;
        data.erase(thread);

        if (data.empty()) { blockchain_.map_.erase(it); }
    }
}

auto Threads::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (!verify_write_lock(lock)) { LogAbort()()("Lock failure").Abort(); }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return StoreProto(serialized, root_);
}

void Threads::save(
    tree::Thread* nym,
    const std::unique_lock<std::mutex>& lock,
    const identifier::Generic& id)
{
    if (!verify_write_lock(lock)) { LogAbort()()("Lock failure").Abort(); }

    if (nullptr == nym) { LogAbort()()("null target").Abort(); }

    auto& index = item_map_[id];
    auto& hash = std::get<0>(index);
    auto& alias = std::get<1>(index);
    hash = nym->Root();
    alias = nym->Alias();

    if (!save(lock)) { LogAbort()()("save error").Abort(); }
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

    const auto lock = Lock{blockchain_.lock_};

    for (const auto& [txid, data] : blockchain_.map_) {
        if (data.empty()) { continue; }

        auto index = proto::StorageBlockchainTransactions{};
        index.set_version(1);
        index.set_txid(UnallocatedCString{txid.Bytes()});
        std::ranges::for_each(data, [&](const auto& id) {
            assert_false(id.empty());

            index.add_thread(UnallocatedCString{id.Bytes()});
        });

        assert_true(
            static_cast<std::size_t>(index.thread_size()) == data.size());

        auto success = proto::Validate(index, VERBOSE);

        assert_true(success);

        auto hash = Hash{};
        success = StoreProto(index, hash);

        assert_true(success);

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
