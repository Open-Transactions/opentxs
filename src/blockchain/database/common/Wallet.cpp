// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/common/Wallet.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <algorithm>
#include <iterator>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

#include "blockchain/bitcoin/block/transaction/TransactionPrivate.hpp"
#include "blockchain/database/common/Bulk.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/storage/file/Index.hpp"
#include "internal/util/storage/file/Mapped.hpp"
#include "internal/util/storage/file/Types.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"  // IWYU pragma: keep
#include "util/Container.hpp"

namespace opentxs::blockchain::database::common
{
Wallet::Wallet(
    const api::Session& api,
    const api::crypto::Blockchain& blockchain,
    storage::lmdb::Database& lmdb,
    Bulk& bulk) noexcept(false)
    : api_(api)
    , blockchain_(blockchain)
    , lmdb_(lmdb)
    , bulk_(bulk)
    , transaction_table_(Table::TransactionIndex)
    , lock_()
    , contact_to_element_()
    , element_to_contact_()
    , transaction_to_patterns_()
    , pattern_to_transactions_()
{
}

auto Wallet::AssociateTransaction(
    const block::TransactionHash& txid,
    const ElementHashes& in) const noexcept -> bool
{
    LogTrace()(OT_PRETTY_CLASS())("Transaction ")(txid.asHex())(
        " is associated with patterns:")
        .Flush();
    // TODO transaction data never changes so indexing should only happen
    // once.
    auto incoming = UnallocatedSet<ElementHash>{};
    std::for_each(std::begin(in), std::end(in), [&](auto& pattern) {
        incoming.emplace(pattern);
        LogTrace()("    * ")(pattern).Flush();
    });
    Lock lock(lock_);
    auto& existing = transaction_to_patterns_[txid];
    auto newElements = UnallocatedVector<ElementHash>{};
    auto removedElements = UnallocatedVector<ElementHash>{};
    std::set_difference(
        std::begin(incoming),
        std::end(incoming),
        std::begin(existing),
        std::end(existing),
        std::back_inserter(newElements));
    std::set_difference(
        std::begin(existing),
        std::end(existing),
        std::begin(incoming),
        std::end(incoming),
        std::back_inserter(removedElements));

    if (0 < newElements.size()) {
        LogTrace()(OT_PRETTY_CLASS())("New patterns:").Flush();
    }

    std::for_each(
        std::begin(newElements),
        std::end(newElements),
        [&](const auto& element) {
            pattern_to_transactions_[element].insert(txid);
            LogTrace()("    * ")(element).Flush();
        });

    if (0 < removedElements.size()) {
        LogTrace()(OT_PRETTY_CLASS())("Obsolete patterns:").Flush();
    }

    std::for_each(
        std::begin(removedElements),
        std::end(removedElements),
        [&](const auto& element) {
            pattern_to_transactions_[element].erase(txid);
            LogTrace()("    * ")(element).Flush();
        });
    existing.swap(incoming);
    // TODO retain this information

    return true;
}

auto Wallet::ForgetTransaction(
    const block::TransactionHash& txid) const noexcept -> bool
{
    return lmdb_.Delete(transaction_table_, txid.Bytes());
}

auto Wallet::LoadTransaction(
    const block::TransactionHash& txid,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> block::Transaction
{
    auto proto = proto::BlockchainTransaction{};

    return LoadTransaction(txid, proto, alloc, monotonic);
}

auto Wallet::LoadTransaction(
    const block::TransactionHash& txid,
    proto::BlockchainTransaction& proto,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> block::Transaction
{
    try {
        proto = [&] {
            const auto indices = [&] {
                auto out = Vector<storage::file::Index>{monotonic};
                out.clear();
                auto cb = [&out](const auto in) {
                    auto& index = out.emplace_back();
                    index.Deserialize(in);
                };
                lmdb_.Load(transaction_table_, txid.Bytes(), cb);

                if (out.empty() || out.front().empty()) {
                    throw std::out_of_range("Transaction not found");
                }

                return out;
            }();
            const auto files = bulk_.Read(indices, monotonic);

            OT_ASSERT(false == files.empty());

            const auto bytes = files.front();

            if (false == valid(bytes)) {
                ForgetTransaction(txid);

                throw std::out_of_range(
                    "failed to load serialized transaction");
            }

            return proto::Factory<proto::BlockchainTransaction>(bytes);
        }();

        return factory::BitcoinTransaction(
            api_.Crypto().Blockchain(), api_.Factory(), proto, alloc);
    } catch (const std::exception& e) {
        LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    }
}

auto Wallet::LookupContact(const Data& pubkeyHash) const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    Lock lock(lock_);

    return element_to_contact_[pubkeyHash];
}

auto Wallet::LookupTransactions(const ElementHash pattern) const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    auto output = UnallocatedVector<block::TransactionHash>{};

    try {
        const auto& data = pattern_to_transactions_.at(pattern);
        std::transform(
            std::begin(data),
            std::end(data),
            std::back_inserter(output),
            [](const auto& txid) -> auto { return txid; });

    } catch (...) {
    }

    return output;
}

auto Wallet::StoreTransaction(const block::Transaction& in) const noexcept
    -> bool
{
    auto out = proto::BlockchainTransaction{};

    return StoreTransaction(in, out);
}

auto Wallet::StoreTransaction(
    const block::Transaction& in,
    proto::BlockchainTransaction& proto) const noexcept -> bool
{
    try {
        proto = [&] {
            auto out = in.Internal().asBitcoin().Serialize(api_);

            if (false == out.has_value()) {
                throw std::runtime_error{"Failed to serialize transaction"};
            }

            return out.value();
        }();
        const auto& hash = proto.txid();
        const auto bytes = proto.ByteSizeLong();
        auto tx = lmdb_.TransactionRW();
        auto write = bulk_.Write(tx, {bytes});
        auto& [index, location] = write.at(0);
        const auto& [_, view] = location;
        const auto cb = storage::file::SourceData{std::make_pair(
            [&](auto&& writer) {
                return proto::write(proto, std::move(writer));
            },
            bytes)};

        if (view.size() != bytes) {

            throw std::runtime_error{
                "failed to get write position for transaction"};
        }

        // TODO monotonic allocator
        const auto written = storage::file::Mapped::Write(cb, location, {});

        if (false == written) {
            throw std::runtime_error{"Failed to write transaction to storage"};
        }

        const auto sIndex = index.Serialize();
        const auto result =
            lmdb_.Store(transaction_table_, hash, sIndex.Bytes(), tx);

        if (result.first) {
            LogTrace()(OT_PRETTY_CLASS())("saved ")(
                bytes)(" bytes at position ")(index.MemoryPosition())(
                " for transaction ")
                .asHex(hash)
                .Flush();
        } else {
            throw std::runtime_error{"Failed to update index for transaction"};
        }

        if (false == tx.Finalize(true)) {
            throw std::runtime_error{"Database update error"};
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Wallet::update_contact(
    const Lock& lock,
    const UnallocatedSet<ByteArray>& existing,
    const UnallocatedSet<ByteArray>& incoming,
    const identifier::Generic& contactID) const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    auto newAddresses = UnallocatedVector<ByteArray>{};
    auto removedAddresses = UnallocatedVector<ByteArray>{};
    auto output = UnallocatedVector<block::TransactionHash>{};
    std::set_difference(
        std::begin(incoming),
        std::end(incoming),
        std::begin(existing),
        std::end(existing),
        std::back_inserter(newAddresses));
    std::set_difference(
        std::begin(existing),
        std::end(existing),
        std::begin(incoming),
        std::end(incoming),
        std::back_inserter(removedAddresses));
    std::for_each(
        std::begin(removedAddresses),
        std::end(removedAddresses),
        [&](const auto& element) {
            element_to_contact_[element].erase(contactID);
            const auto pattern =
                blockchain_.Internal().IndexItem(element.Bytes());

            try {
                const auto& transactions = pattern_to_transactions_.at(pattern);
                std::copy(
                    std::begin(transactions),
                    std::end(transactions),
                    std::back_inserter(output));
            } catch (...) {
            }
        });
    std::for_each(
        std::begin(newAddresses),
        std::end(newAddresses),
        [&](const auto& element) {
            element_to_contact_[element].insert(contactID);
            const auto pattern =
                blockchain_.Internal().IndexItem(element.Bytes());

            try {
                const auto& transactions = pattern_to_transactions_.at(pattern);
                std::copy(
                    std::begin(transactions),
                    std::end(transactions),
                    std::back_inserter(output));
            } catch (...) {
            }
        });
    dedup(output);

    return output;
}

auto Wallet::UpdateContact(const opentxs::Contact& contact) const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    auto incoming = UnallocatedSet<ByteArray>{};

    {
        auto data = contact.BlockchainAddresses();
        std::for_each(std::begin(data), std::end(data), [&](auto& in) {
            auto& [bytes, style, type] = in;
            incoming.emplace(std::move(bytes));
        });
    }

    Lock lock(lock_);
    const auto& contactID = contact.ID();
    auto& existing = contact_to_element_[contactID];
    auto output = update_contact(lock, existing, incoming, contactID);
    existing.swap(incoming);

    return output;
}

auto Wallet::UpdateMergedContact(
    const opentxs::Contact& parent,
    const opentxs::Contact& child) const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    auto deleted = UnallocatedSet<ByteArray>{};
    auto incoming = UnallocatedSet<ByteArray>{};

    {
        auto data = child.BlockchainAddresses();
        std::for_each(std::begin(data), std::end(data), [&](auto& in) {
            auto& [bytes, style, type] = in;
            deleted.emplace(std::move(bytes));
        });
    }

    {
        auto data = parent.BlockchainAddresses();
        std::for_each(std::begin(data), std::end(data), [&](auto& in) {
            auto& [bytes, style, type] = in;
            incoming.emplace(std::move(bytes));
        });
    }

    Lock lock(lock_);
    const auto& contactID = parent.ID();
    const auto& deletedID = child.ID();
    auto& existing = contact_to_element_[contactID];
    contact_to_element_.erase(deletedID);
    auto output = update_contact(lock, existing, incoming, contactID);
    std::for_each(
        std::begin(deleted), std::end(deleted), [&](const auto& element) {
            element_to_contact_[element].erase(deletedID);
            const auto pattern =
                blockchain_.Internal().IndexItem(element.Bytes());

            try {
                const auto& transactions = pattern_to_transactions_.at(pattern);
                std::copy(
                    std::begin(transactions),
                    std::end(transactions),
                    std::back_inserter(output));
            } catch (...) {
            }
        });
    dedup(output);
    existing.swap(incoming);

    return output;
}

Wallet::~Wallet() = default;
}  // namespace opentxs::blockchain::database::common
