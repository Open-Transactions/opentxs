// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"

#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <memory>
#include <mutex>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>

#include "api/crypto/blockchain/Blockchain.hpp"
#include "api/crypto/blockchain/Imp.hpp"
#include "blockchain/database/common/Database.hpp"
#include "internal/api/crypto/blockchain/BalanceOracle.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

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

namespace session
{
class Activity;
class Contacts;
}  // namespace session

class Legacy;
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Transaction;
}  // namespace internal

class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace p2p
{
class Data;
}  // namespace p2p

namespace zeromq
{
class Context;
class Message;
}  // namespace zeromq
}  // namespace network

class Contact;
class Data;
class Identifier;
class Options;
class PasswordPrompt;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::crypto::imp
{
struct BlockchainImp final : public Blockchain::Imp {
    using Txid = opentxs::blockchain::block::Txid;
    using pTxid = opentxs::blockchain::block::pTxid;
    using TxidHex = Blockchain::TxidHex;
    using PatternID = Blockchain::PatternID;
    using ContactList = Blockchain::ContactList;

    auto ActivityDescription(
        const identifier::Nym& nym,
        const Identifier& thread,
        const UnallocatedCString& threadItemID) const noexcept
        -> UnallocatedCString final;
    auto ActivityDescription(
        const identifier::Nym& nym,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::bitcoin::Transaction& transaction)
        const noexcept -> UnallocatedCString final;
    auto AssignTransactionMemo(
        const TxidHex& id,
        const UnallocatedCString& label) const noexcept -> bool final;
    auto IndexItem(const ReadView bytes) const noexcept -> PatternID final;
    auto KeyEndpoint() const noexcept -> std::string_view final;
    auto KeyGenerated(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& account,
        const Identifier& subaccount,
        const opentxs::blockchain::crypto::SubaccountType type,
        const opentxs::blockchain::crypto::Subchain subchain) const noexcept
        -> void final;
    auto LoadTransactionBitcoin(const TxidHex& txid) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Transaction> final;
    auto LoadTransactionBitcoin(const Txid& txid) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Transaction> final;
    auto LookupContacts(const Data& pubkeyHash) const noexcept
        -> ContactList final;
    auto ProcessContact(const Contact& contact) const noexcept -> bool final;
    auto ProcessMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> bool final;
    auto ProcessTransaction(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::bitcoin::Transaction& in,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto ReportScan(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& owner,
        const opentxs::blockchain::crypto::SubaccountType type,
        const Identifier& account,
        const Blockchain::Subchain subchain,
        const opentxs::blockchain::block::Position& progress) const noexcept
        -> void final;
    auto Unconfirm(
        const Blockchain::Key key,
        const opentxs::blockchain::block::Txid& tx,
        const Time time) const noexcept -> bool final;
    auto UpdateBalance(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept
        -> void final;
    auto UpdateBalance(
        const identifier::Nym& owner,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept
        -> void final;
    auto UpdateElement(UnallocatedVector<ReadView>& pubkeyHashes) const noexcept
        -> void final;

    BlockchainImp(
        const api::Session& api,
        const api::session::Activity& activity,
        const api::session::Contacts& contacts,
        const api::Legacy& legacy,
        const UnallocatedCString& dataFolder,
        const Options& args,
        api::crypto::Blockchain& parent) noexcept;

    ~BlockchainImp() final = default;

private:
    const api::session::Activity& activity_;
    const CString key_generated_endpoint_;
    OTZMQPublishSocket transaction_updates_;
    OTZMQPublishSocket key_updates_;
    OTZMQPublishSocket scan_updates_;
    OTZMQPublishSocket new_blockchain_accounts_;
    blockchain::BalanceOracle balances_;

    auto broadcast_update_signal(const Txid& txid) const noexcept -> void
    {
        broadcast_update_signal(UnallocatedVector<pTxid>{txid});
    }
    auto broadcast_update_signal(
        const UnallocatedVector<pTxid>& transactions) const noexcept -> void;
    auto broadcast_update_signal(
        const opentxs::blockchain::block::bitcoin::Transaction& tx)
        const noexcept -> void;
    auto load_transaction(const Lock& lock, const Txid& id) const noexcept
        -> std::unique_ptr<opentxs::blockchain::block::bitcoin::Transaction>;
    auto load_transaction(const Lock& lock, const TxidHex& id) const noexcept
        -> std::unique_ptr<opentxs::blockchain::block::bitcoin::Transaction>;
    auto notify_new_account(
        const Identifier& id,
        const identifier::Nym& owner,
        opentxs::blockchain::Type chain,
        opentxs::blockchain::crypto::SubaccountType type) const noexcept
        -> void final;
    auto reconcile_activity_threads(const Lock& lock, const Txid& txid)
        const noexcept -> bool;
    auto reconcile_activity_threads(
        const Lock& lock,
        const opentxs::blockchain::block::bitcoin::Transaction& tx)
        const noexcept -> bool;
};
}  // namespace opentxs::api::crypto::imp
