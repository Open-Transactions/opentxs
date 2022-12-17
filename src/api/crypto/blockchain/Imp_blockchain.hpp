// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Data.hpp"

#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>

#include "api/crypto/blockchain/Blockchain.hpp"
#include "api/crypto/blockchain/Imp.hpp"
#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Router.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace session
{
class Activity;
class Client;
class Contacts;
}  // namespace session

class Legacy;
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Transaction;
class TransactionHash;
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace network
{
namespace otdht
{
class Data;
}  // namespace otdht

namespace zeromq
{
class Context;
class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainTransaction;
}  // namespace proto

class Contact;
class Data;
class Options;
class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::crypto::imp
{
struct BlockchainImp final : public Blockchain::Imp {
    using Txid = opentxs::blockchain::block::TransactionHash;
    using TxidHex = Blockchain::TxidHex;
    using ContactList = Blockchain::ContactList;

    auto ActivityDescription(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const std::string_view threadItemID,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> UnallocatedCString final;
    auto ActivityDescription(
        const identifier::Nym& nym,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::Transaction& transaction)
        const noexcept -> UnallocatedCString final;
    auto AssignTransactionMemo(
        const TxidHex& id,
        const std::string_view label,
        alloc::Default monotonic) const noexcept -> bool final;
    auto IndexItem(const ReadView bytes) const noexcept
        -> opentxs::blockchain::block::ElementHash final;
    auto KeyEndpoint() const noexcept -> std::string_view final;
    auto KeyGenerated(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& account,
        const identifier::Generic& subaccount,
        const opentxs::blockchain::crypto::SubaccountType type,
        const opentxs::blockchain::crypto::Subchain subchain) const noexcept
        -> void final;
    auto LoadTransaction(
        const TxidHex& txid,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept
        -> opentxs::blockchain::block::Transaction final;
    auto LoadTransaction(
        const Txid& txid,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept
        -> opentxs::blockchain::block::Transaction final;
    auto LookupContacts(const Data& pubkeyHash) const noexcept
        -> ContactList final;
    auto ProcessContact(const Contact& contact, alloc::Default monotonic)
        const noexcept -> bool final;
    auto ProcessMergedContact(
        const Contact& parent,
        const Contact& child,
        alloc::Default monotonic) const noexcept -> bool final;
    auto ProcessTransactions(
        const opentxs::blockchain::Type chain,
        Set<opentxs::blockchain::block::Transaction>&& transactions,
        const PasswordPrompt& reason,
        alloc::Default monotonic) const noexcept -> bool final;

    auto ReportScan(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& owner,
        const opentxs::blockchain::crypto::SubaccountType type,
        const identifier::Generic& account,
        const Blockchain::Subchain subchain,
        const opentxs::blockchain::block::Position& progress) const noexcept
        -> void final;
    auto Start(std::shared_ptr<const api::Session> api) noexcept -> void final;
    auto Unconfirm(
        const Blockchain::Key key,
        const Txid& tx,
        const Time time,
        alloc::Default monotonic) const noexcept -> bool final;
    auto UpdateElement(
        std::span<const ReadView> pubkeyHashes,
        alloc::Default monotonic) const noexcept -> void final;

    BlockchainImp(
        const api::session::Client& api,
        const api::session::Activity& activity,
        const api::session::Contacts& contacts,
        const api::Legacy& legacy,
        const std::string_view dataFolder,
        const Options& args,
        api::crypto::Blockchain& parent) noexcept;

    ~BlockchainImp() final = default;

private:
    const api::session::Client& client_;
    const api::session::Activity& activity_;
    const CString key_generated_endpoint_;
    OTZMQPublishSocket transaction_updates_;
    OTZMQPublishSocket key_updates_;
    OTZMQPublishSocket scan_updates_;
    OTZMQPublishSocket new_blockchain_accounts_;

    auto broadcast_update_signal(const Txid& txid, alloc::Default monotonic)
        const noexcept -> void;
    auto broadcast_update_signal(
        std::span<const Txid> transactions,
        alloc::Default monotonic) const noexcept -> void;
    auto broadcast_update_signal(
        const proto::BlockchainTransaction& proto,
        const opentxs::blockchain::block::Transaction& tx) const noexcept
        -> void;
    auto load_transaction(
        const Lock& lock,
        const Txid& id,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept
        -> opentxs::blockchain::block::Transaction;
    auto load_transaction(
        const Lock& lock,
        const Txid& id,
        proto::BlockchainTransaction& out,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept
        -> opentxs::blockchain::block::Transaction;
    auto load_transaction(
        const Lock& lock,
        const TxidHex& id,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept
        -> opentxs::blockchain::block::Transaction;
    auto load_transaction(
        const Lock& lock,
        const TxidHex& id,
        proto::BlockchainTransaction& out,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept
        -> opentxs::blockchain::block::Transaction;
    auto notify_new_account(
        const identifier::Generic& id,
        const identifier::Nym& owner,
        opentxs::blockchain::Type chain,
        opentxs::blockchain::crypto::SubaccountType type) const noexcept
        -> void final;
    auto reconcile_activity_threads(
        const Lock& lock,
        const Txid& txid,
        alloc::Default monotonic) const noexcept -> bool;
    auto reconcile_activity_threads(
        const Lock& lock,
        const proto::BlockchainTransaction& proto,
        const opentxs::blockchain::block::Transaction& tx) const noexcept
        -> bool;
};
}  // namespace opentxs::api::crypto::imp
