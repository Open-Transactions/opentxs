// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Bytes.hpp"

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
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace database
{
namespace common
{
class Bulk;
}  // namespace common
}  // namespace database
}  // namespace blockchain

namespace contact
{
class Contact;
}  // namespace contact

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs

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
        const std::pmr::vector<PatternID>& patterns) const noexcept -> bool;
    auto LoadTransaction(const ReadView txid) const noexcept
        -> std::unique_ptr<block::bitcoin::Transaction>;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> std::pmr::set<OTIdentifier>;
    auto LookupTransactions(const PatternID pattern) const noexcept
        -> std::pmr::vector<pTxid>;
    auto StoreTransaction(const block::bitcoin::Transaction& tx) const noexcept
        -> bool;
    auto UpdateContact(const contact::Contact& contact) const noexcept
        -> std::pmr::vector<pTxid>;
    auto UpdateMergedContact(
        const contact::Contact& parent,
        const contact::Contact& child) const noexcept
        -> std::pmr::vector<pTxid>;

    Wallet(
        const api::Session& api,
        const api::crypto::Blockchain& blockchain,
        storage::lmdb::LMDB& lmdb,
        Bulk& bulk) noexcept(false);

    ~Wallet();

private:
    using ContactToElement = std::pmr::map<OTIdentifier, std::pmr::set<OTData>>;
    using ElementToContact = std::pmr::map<OTData, std::pmr::set<OTIdentifier>>;
    using TransactionToPattern = std::pmr::map<pTxid, std::pmr::set<PatternID>>;
    using PatternToTransaction = std::pmr::map<PatternID, std::pmr::set<pTxid>>;

    const api::Session& api_;
    const api::crypto::Blockchain& blockchain_;
    storage::lmdb::LMDB& lmdb_;
    Bulk& bulk_;
    const int transaction_table_;
    mutable std::mutex lock_;
    mutable ContactToElement contact_to_element_;
    mutable ElementToContact element_to_contact_;
    mutable TransactionToPattern transaction_to_patterns_;
    mutable PatternToTransaction pattern_to_transactions_;

    auto update_contact(
        const Lock& lock,
        const std::pmr::set<OTData>& existing,
        const std::pmr::set<OTData>& incoming,
        const Identifier& contactID) const noexcept -> std::pmr::vector<pTxid>;
};
}  // namespace opentxs::blockchain::database::common
