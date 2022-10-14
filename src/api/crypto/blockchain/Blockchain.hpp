// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "api/client/blockchain/database/Database.hpp"
// IWYU pragma: no_include "internal/blockchain/node/Node.hpp"
// IWYU pragma: no_include "opentxs/api/session/Contacts.hpp"
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/Account.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/AddressStyle.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/HD.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/HDProtocol.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/PaymentCode.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/Manager.hpp"
// IWYU pragma: no_include "internal/network/zeromq/socket/Publish.hpp"

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <future>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <thread>
#include <tuple>
#include <utility>

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Activity;
class Client;
class Contacts;
class Factory;
}  // namespace session

class Legacy;
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

namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
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
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace proto
{
class HDPath;
}  // namespace proto

class ByteArray;
class Contact;
class Data;
class Options;
class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::imp
{
class Blockchain final : virtual public internal::Blockchain
{
public:
    struct Imp;

    auto Account(const identifier::Nym& nymID, const Chain chain) const
        noexcept(false) -> const opentxs::blockchain::crypto::Account& final;
    auto AccountList(const identifier::Nym& nymID) const noexcept
        -> UnallocatedSet<identifier::Generic> final;
    auto AccountList(const Chain chain) const noexcept
        -> UnallocatedSet<identifier::Generic> final;
    auto AccountList() const noexcept
        -> UnallocatedSet<identifier::Generic> final;
    auto ActivityDescription(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const UnallocatedCString& threadItemID) const noexcept
        -> UnallocatedCString final;
    auto ActivityDescription(
        const identifier::Nym& nym,
        const Chain chain,
        const opentxs::blockchain::bitcoin::block::Transaction& transaction)
        const noexcept -> UnallocatedCString final;
    auto AssignContact(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const identifier::Generic& contactID) const noexcept -> bool final;
    auto AssignLabel(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const UnallocatedCString& label) const noexcept -> bool final;
    auto AssignTransactionMemo(
        const UnallocatedCString& id,
        const UnallocatedCString& label) const noexcept -> bool final;
    auto BalanceOracleEndpoint() const noexcept -> std::string_view final;
    auto CalculateAddress(
        const Chain chain,
        const opentxs::blockchain::crypto::AddressStyle format,
        const Data& pubkey) const noexcept -> UnallocatedCString final;
    auto Confirm(const Key key, const opentxs::blockchain::block::Txid& tx)
        const noexcept -> bool final;
    auto Contacts() const noexcept -> const api::session::Contacts& final;
    auto DecodeAddress(const UnallocatedCString& encoded) const noexcept
        -> DecodedAddress final;
    auto EncodeAddress(const Style style, const Chain chain, const Data& data)
        const noexcept -> UnallocatedCString final;
    auto GetKey(const Key& id) const noexcept(false)
        -> const opentxs::blockchain::crypto::Element& final;
    auto HDSubaccount(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID) const noexcept(false)
        -> const opentxs::blockchain::crypto::HD& final;
    auto IndexItem(const ReadView bytes) const noexcept
        -> opentxs::blockchain::block::ElementHash final;
    auto KeyEndpoint() const noexcept -> std::string_view final;
    auto KeyGenerated(
        const Chain chain,
        const identifier::Nym& account,
        const identifier::Generic& subaccount,
        const opentxs::blockchain::crypto::SubaccountType type,
        const opentxs::blockchain::crypto::Subchain subchain) const noexcept
        -> void final;
    auto LoadTransactionBitcoin(const TxidHex& id) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::bitcoin::block::Transaction> final;
    auto LoadTransactionBitcoin(const Txid& id) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::bitcoin::block::Transaction> final;
    auto LookupAccount(const identifier::Generic& id) const noexcept
        -> AccountData final;
    auto LookupContacts(const UnallocatedCString& address) const noexcept
        -> ContactList final;
    auto LookupContacts(const Data& pubkeyHash) const noexcept
        -> ContactList final;
    auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept
        -> identifier::Generic final;
    auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const Chain derivationChain,
        const Chain targetChain,
        const PasswordPrompt& reason) const noexcept
        -> identifier::Generic final;
    auto NewNym(const identifier::Nym& id) const noexcept -> void final;
    auto NewPaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept
        -> identifier::Generic final;
    auto NewPaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const ReadView& view,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept
        -> identifier::Generic final;
    auto Owner(const identifier::Generic& accountID) const noexcept
        -> const identifier::Nym& final;
    auto Owner(const Key& key) const noexcept -> const identifier::Nym& final;
    auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode& final;
    auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode& final;
    auto PubkeyHash(const Chain chain, const Data& pubkey) const noexcept(false)
        -> ByteArray final;
    auto ProcessContact(const Contact& contact) const noexcept -> bool final;
    auto ProcessMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> bool final;
    auto ProcessTransactions(
        const Chain chain,
        Set<std::shared_ptr<opentxs::blockchain::bitcoin::block::Transaction>>&&
            transactions,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto RecipientContact(const Key& key) const noexcept
        -> identifier::Generic final;
    auto Release(const Key key) const noexcept -> bool final;
    auto ReportScan(
        const Chain chain,
        const identifier::Nym& owner,
        const opentxs::blockchain::crypto::SubaccountType type,
        const identifier::Generic& account,
        const Subchain subchain,
        const opentxs::blockchain::block::Position& progress) const noexcept
        -> void final;
    auto SenderContact(const Key& key) const noexcept
        -> identifier::Generic final;
    auto Start(std::shared_ptr<const api::Session> api) noexcept -> void final;
    auto SubaccountList(const identifier::Nym& nymID, const Chain chain)
        const noexcept -> UnallocatedSet<identifier::Generic> final;
    auto UpdateElement(UnallocatedVector<ReadView>& pubkeyHashes) const noexcept
        -> void final;
    auto Unconfirm(
        const Key key,
        const opentxs::blockchain::block::Txid& tx,
        const Time time) const noexcept -> bool final;
    auto Wallet(const Chain chain) const noexcept(false)
        -> const opentxs::blockchain::crypto::Wallet& final;

    auto Init() noexcept -> void final;

    Blockchain(
        const api::session::Client& api,
        const api::session::Activity& activity,
        const api::session::Contacts& contacts,
        const api::Legacy& legacy,
        const UnallocatedCString& dataFolder,
        const Options& args) noexcept;
    Blockchain() = delete;
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    auto operator=(const Blockchain&) -> Blockchain& = delete;
    auto operator=(Blockchain&&) -> Blockchain& = delete;

    ~Blockchain() final;

private:
    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::api::crypto::imp
