// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/common/Wallet.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainTransaction.pb.h>
#include <algorithm>
#include <iterator>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "blockchain/database/common/Bulk.hpp"
#include "blockchain/protocol/bitcoin/base/block/transaction/TransactionPrivate.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/storage/file/Index.hpp"
#include "internal/util/storage/file/Mapped.hpp"
#include "internal/util/storage/file/Types.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/Types.internal.tpp"
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
    LogTrace()()("Transaction ")(txid.asHex())(" is associated with patterns:")
        .Flush();
    // TODO transaction data never changes so indexing should only happen
    // once.
    auto incoming = UnallocatedSet<ElementHash>{};
    std::ranges::for_each(in, [&](auto& pattern) {
        incoming.emplace(pattern);
        LogTrace()("    * ")(pattern).Flush();
    });
    auto lock = Lock{lock_};
    auto& existing = transaction_to_patterns_[txid];
    auto newElements = UnallocatedVector<ElementHash>{};
    auto removedElements = UnallocatedVector<ElementHash>{};
    std::ranges::set_difference(
        incoming, existing, std::back_inserter(newElements));
    std::ranges::set_difference(
        existing, incoming, std::back_inserter(removedElements));

    if (0 < newElements.size()) { LogTrace()()("New patterns:").Flush(); }

    std::ranges::for_each(newElements, [&](const auto& element) {
        pattern_to_transactions_[element].insert(txid);
        LogTrace()("    * ")(element).Flush();
    });

    if (0 < removedElements.size()) {
        LogTrace()()("Obsolete patterns:").Flush();
    }

    std::ranges::for_each(removedElements, [&](const auto& element) {
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
    auto proto = protobuf::BlockchainTransaction{};

    return LoadTransaction(txid, proto, alloc, monotonic);
}

auto Wallet::LoadTransaction(
    const block::TransactionHash& txid,
    protobuf::BlockchainTransaction& proto,
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

            assert_false(files.empty());

            const auto bytes = files.front();

            if (false == valid(bytes)) {
                ForgetTransaction(txid);

                throw std::out_of_range(
                    "failed to load serialized transaction");
            }

            return protobuf::Factory<protobuf::BlockchainTransaction>(bytes);
        }();

        return factory::BitcoinTransaction(
            api_.Crypto().Blockchain(), api_.Factory(), proto, alloc);
    } catch (const std::exception& e) {
        LogTrace()()(e.what()).Flush();

        return {};
    }
}

auto Wallet::LookupContact(const Data& pubkeyHash) const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    auto lock = Lock{lock_};

    return element_to_contact_[pubkeyHash];
}

auto Wallet::LookupTransactions(const ElementHash pattern) const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    auto output = UnallocatedVector<block::TransactionHash>{};

    try {
        const auto& data = pattern_to_transactions_.at(pattern);
        std::ranges::transform(
            data, std::back_inserter(output), [](const auto& txid) -> auto {
                return txid;
            });

    } catch (...) {
    }

    return output;
}

auto Wallet::StoreTransaction(const block::Transaction& in) const noexcept
    -> bool
{
    auto out = protobuf::BlockchainTransaction{};

    return StoreTransaction(in, out);
}

auto Wallet::StoreTransaction(
    const block::Transaction& in,
    protobuf::BlockchainTransaction& proto) const noexcept -> bool
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
                return protobuf::write(proto, std::move(writer));
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
            LogTrace()()("saved ")(bytes)(" bytes at position ")(
                index.MemoryPosition())(" for transaction ")
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
        LogError()()(e.what()).Flush();

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
    std::ranges::set_difference(
        incoming, existing, std::back_inserter(newAddresses));
    std::ranges::set_difference(
        existing, incoming, std::back_inserter(removedAddresses));
    std::ranges::for_each(removedAddresses, [&](const auto& element) {
        element_to_contact_[element].erase(contactID);
        const auto pattern = blockchain_.Internal().IndexItem(element.Bytes());

        try {
            const auto& transactions = pattern_to_transactions_.at(pattern);
            std::ranges::copy(transactions, std::back_inserter(output));
        } catch (...) {
        }
    });
    std::ranges::for_each(newAddresses, [&](const auto& element) {
        element_to_contact_[element].insert(contactID);
        const auto pattern = blockchain_.Internal().IndexItem(element.Bytes());

        try {
            const auto& transactions = pattern_to_transactions_.at(pattern);
            std::ranges::copy(transactions, std::back_inserter(output));
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
        std::ranges::for_each(data, [&](auto& in) {
            auto& [bytes, style, type] = in;
            incoming.emplace(std::move(bytes));
        });
    }

    auto lock = Lock{lock_};
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
        std::ranges::for_each(data, [&](auto& in) {
            auto& [bytes, style, type] = in;
            deleted.emplace(std::move(bytes));
        });
    }

    {
        auto data = parent.BlockchainAddresses();
        std::ranges::for_each(data, [&](auto& in) {
            auto& [bytes, style, type] = in;
            incoming.emplace(std::move(bytes));
        });
    }

    auto lock = Lock{lock_};
    const auto& contactID = parent.ID();
    const auto& deletedID = child.ID();
    auto& existing = contact_to_element_[contactID];
    contact_to_element_.erase(deletedID);
    auto output = update_contact(lock, existing, incoming, contactID);
    std::ranges::for_each(deleted, [&](const auto& element) {
        element_to_contact_[element].erase(deletedID);
        const auto pattern = blockchain_.Internal().IndexItem(element.Bytes());

        try {
            const auto& transactions = pattern_to_transactions_.at(pattern);
            std::ranges::copy(transactions, std::back_inserter(output));
        } catch (...) {
        }
    });
    dedup(output);
    existing.swap(incoming);

    return output;
}

Wallet::~Wallet() = default;
}  // namespace opentxs::blockchain::database::common
