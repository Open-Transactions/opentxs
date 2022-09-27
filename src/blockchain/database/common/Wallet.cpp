// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/database/common/Wallet.hpp"  // IWYU pragma: associated

#include <BlockchainTransaction.pb.h>
#include <algorithm>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "blockchain/database/common/Bulk.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/storage/file/Index.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
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
    const Txid& txid,
    const UnallocatedVector<PatternID>& in) const noexcept -> bool
{
    LogTrace()(OT_PRETTY_CLASS())("Transaction ")(txid.asHex())(
        " is associated with patterns:")
        .Flush();
    // TODO transaction data never changes so indexing should only happen
    // once.
    auto incoming = UnallocatedSet<PatternID>{};
    std::for_each(std::begin(in), std::end(in), [&](auto& pattern) {
        incoming.emplace(pattern);
        LogTrace()("    * ")(pattern).Flush();
    });
    Lock lock(lock_);
    auto& existing = transaction_to_patterns_[txid];
    auto newElements = UnallocatedVector<PatternID>{};
    auto removedElements = UnallocatedVector<PatternID>{};
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

    return true;
}

auto Wallet::ForgetTransaction(const ReadView txid) const noexcept -> bool
{
    return lmdb_.Delete(transaction_table_, txid);
}

auto Wallet::LoadTransaction(const ReadView txid) const noexcept
    -> std::unique_ptr<bitcoin::block::Transaction>
{
    auto proto = proto::BlockchainTransaction{};

    return LoadTransaction(txid, proto);
}

auto Wallet::LoadTransaction(
    const ReadView txid,
    proto::BlockchainTransaction& proto) const noexcept
    -> std::unique_ptr<bitcoin::block::Transaction>
{
    try {
        proto = [&] {
            const auto indices = [&] {
                auto out = Vector<storage::file::Index>{};
                auto cb = [&out](const auto in) {
                    auto& index = out.emplace_back();
                    index.Deserialize(in);
                };
                lmdb_.Load(transaction_table_, txid, cb);

                if (out.empty() || out.front().empty()) {
                    throw std::out_of_range("Transaction not found");
                }

                return out;
            }();
            const auto views = bulk_.Read(indices);

            OT_ASSERT(false == views.empty());

            const auto& bytes = views.front();

            if (false == valid(bytes)) {
                ForgetTransaction(txid);

                throw std::out_of_range(
                    "failed to load serialized transaction");
            }

            return proto::Factory<proto::BlockchainTransaction>(bytes);
        }();

        return factory::BitcoinTransaction(api_, proto);
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

auto Wallet::LookupTransactions(const PatternID pattern) const noexcept
    -> UnallocatedVector<pTxid>
{
    auto output = UnallocatedVector<pTxid>{};

    try {
        const auto& data = pattern_to_transactions_.at(pattern);
        std::transform(
            std::begin(data), std::end(data), std::back_inserter(output), [
            ](const auto& txid) -> auto{ return txid; });

    } catch (...) {
    }

    return output;
}

auto Wallet::StoreTransaction(
    const bitcoin::block::Transaction& in) const noexcept -> bool
{
    auto out = proto::BlockchainTransaction{};

    return StoreTransaction(in, out);
}

auto Wallet::StoreTransaction(
    const bitcoin::block::Transaction& in,
    proto::BlockchainTransaction& proto) const noexcept -> bool
{
    try {
        proto = [&] {
            auto out = in.Internal().Serialize();

            if (false == out.has_value()) {
                throw std::runtime_error{"Failed to serialize transaction"};
            }

            return out.value();
        }();
        const auto& hash = proto.txid();
        const auto bytes = proto.ByteSizeLong();
        auto tx = lmdb_.TransactionRW();
        auto write = bulk_.Write(tx, {bytes});
        auto& [index, view] = write.at(0);

        if ((false == view.valid(bytes)) || (index.empty())) {
            throw std::runtime_error{
                "Failed to get write position for transaction"};
        }

        if (false == proto::write(proto, preallocated(bytes, view.data()))) {
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
    -> UnallocatedVector<pTxid>
{
    auto newAddresses = UnallocatedVector<ByteArray>{};
    auto removedAddresses = UnallocatedVector<ByteArray>{};
    auto output = UnallocatedVector<pTxid>{};
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
            const auto pattern = blockchain_.IndexItem(element.Bytes());

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
            const auto pattern = blockchain_.IndexItem(element.Bytes());

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
    -> UnallocatedVector<pTxid>
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
    const opentxs::Contact& child) const noexcept -> UnallocatedVector<pTxid>
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
            const auto pattern = blockchain_.IndexItem(element.Bytes());

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
