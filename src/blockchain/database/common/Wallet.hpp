// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <mutex>

#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Session;
}  // namespace api

namespace blockchain
{

namespace database
{
namespace common
{
class Bulk;
}  // namespace common
}  // namespace database
}  // namespace blockchain

namespace proto
{
class BlockchainTransaction;
}  // namespace proto

namespace storage
{
namespace lmdb
{
class Database;
}  // namespace lmdb
}  // namespace storage

class Contact;
class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::common
{
class Wallet
{
public:
    auto AssociateTransaction(
        const block::TransactionHash& txid,
        const ElementHashes& patterns) const noexcept -> bool;
    auto ForgetTransaction(const block::TransactionHash& txid) const noexcept
        -> bool;
    auto LoadTransaction(
        const block::TransactionHash& txid,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> block::Transaction;
    auto LoadTransaction(
        const block::TransactionHash& txid,
        proto::BlockchainTransaction& out,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> block::Transaction;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> UnallocatedSet<identifier::Generic>;
    auto LookupTransactions(const ElementHash pattern) const noexcept
        -> UnallocatedVector<block::TransactionHash>;
    auto StoreTransaction(const block::Transaction& tx) const noexcept -> bool;
    auto StoreTransaction(
        const block::Transaction& tx,
        proto::BlockchainTransaction& out) const noexcept -> bool;
    auto UpdateContact(const Contact& contact) const noexcept
        -> UnallocatedVector<block::TransactionHash>;
    auto UpdateMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> UnallocatedVector<block::TransactionHash>;

    Wallet(
        const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        storage::lmdb::Database& lmdb,
        Bulk& bulk) noexcept(false);

    ~Wallet();

private:
    using ContactToElement =
        UnallocatedMap<identifier::Generic, UnallocatedSet<ByteArray>>;
    using ElementToContact =
        UnallocatedMap<ByteArray, UnallocatedSet<identifier::Generic>>;
    using TransactionToPattern =
        UnallocatedMap<block::TransactionHash, UnallocatedSet<ElementHash>>;
    using PatternToTransaction =
        UnallocatedMap<ElementHash, UnallocatedSet<block::TransactionHash>>;

    const api::Session& api_;
    const api::crypto::Blockchain& blockchain_;
    storage::lmdb::Database& lmdb_;
    Bulk& bulk_;
    const int transaction_table_;
    mutable std::mutex lock_;
    mutable ContactToElement contact_to_element_;
    mutable ElementToContact element_to_contact_;
    mutable TransactionToPattern transaction_to_patterns_;
    mutable PatternToTransaction pattern_to_transactions_;

    auto update_contact(
        const Lock& lock,
        const UnallocatedSet<ByteArray>& existing,
        const UnallocatedSet<ByteArray>& incoming,
        const identifier::Generic& contactID) const noexcept
        -> UnallocatedVector<block::TransactionHash>;
};
}  // namespace opentxs::blockchain::database::common
