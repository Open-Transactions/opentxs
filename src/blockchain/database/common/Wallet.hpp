// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>
#include <optional>

#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
namespace bitcoin
{
namespace block
{
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace database
{
namespace common
{
class Bulk;
}  // namespace common
}  // namespace database
}  // namespace blockchain

namespace identifier
{
class Generic;
}  // namespace identifier

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

class ByteArray;
class Contact;
class Data;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::common
{
class Wallet
{
public:
    using PatternID = opentxs::blockchain::PatternID;
    using Txid = opentxs::blockchain::block::Txid;
    using pTxid = opentxs::blockchain::block::pTxid;

    auto AssociateTransaction(
        const Txid& txid,
        const UnallocatedVector<PatternID>& patterns) const noexcept -> bool;
    auto ForgetTransaction(const ReadView txid) const noexcept -> bool;
    auto LoadTransaction(const ReadView txid) const noexcept
        -> std::unique_ptr<bitcoin::block::Transaction>;
    auto LoadTransaction(const ReadView txid, proto::BlockchainTransaction& out)
        const noexcept -> std::unique_ptr<bitcoin::block::Transaction>;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> UnallocatedSet<identifier::Generic>;
    auto LookupTransactions(const PatternID pattern) const noexcept
        -> UnallocatedVector<pTxid>;
    auto StoreTransaction(const bitcoin::block::Transaction& tx) const noexcept
        -> bool;
    auto StoreTransaction(
        const bitcoin::block::Transaction& tx,
        proto::BlockchainTransaction& out) const noexcept -> bool;
    auto UpdateContact(const Contact& contact) const noexcept
        -> UnallocatedVector<pTxid>;
    auto UpdateMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> UnallocatedVector<pTxid>;

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
        UnallocatedMap<pTxid, UnallocatedSet<PatternID>>;
    using PatternToTransaction =
        UnallocatedMap<PatternID, UnallocatedSet<pTxid>>;

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
        -> UnallocatedVector<pTxid>;
};
}  // namespace opentxs::blockchain::database::common
