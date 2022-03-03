// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"
// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <string_view>

#include "api/crypto/blockchain/AccountCache.hpp"
#include "api/crypto/blockchain/Blockchain.hpp"
#include "api/crypto/blockchain/Wallets.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Bip44Type.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
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
class Contacts;
}  // namespace session

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

namespace crypto
{
namespace internal
{
struct Subaccount;
struct Wallet;
}  // namespace internal

class Account;
class Element;
class HD;
class PaymentCode;
class Subaccount;
class Wallet;
}  // namespace crypto

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
namespace p2p
{
class Data;
class State;
}  // namespace p2p

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

class Contact;
class PasswordPrompt;
class PaymentCode;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::crypto::imp
{
struct Blockchain::Imp {
    using IDLock = UnallocatedMap<OTIdentifier, std::mutex>;

    auto Account(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain) const noexcept(false)
        -> const opentxs::blockchain::crypto::Account&;
    auto AccountList(const identifier::Nym& nymID) const noexcept
        -> UnallocatedSet<OTIdentifier>;
    auto AccountList(const opentxs::blockchain::Type chain) const noexcept
        -> UnallocatedSet<OTIdentifier>;
    auto AccountList() const noexcept -> UnallocatedSet<OTIdentifier>;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const Identifier& thread,
        const UnallocatedCString& threadItemID) const noexcept
        -> UnallocatedCString;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::bitcoin::Transaction& transaction)
        const noexcept -> UnallocatedCString;
    auto address_prefix(
        const Style style,
        const opentxs::blockchain::Type chain) const noexcept(false) -> OTData;
    auto AssignContact(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const Identifier& contactID) const noexcept -> bool;
    auto AssignLabel(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const UnallocatedCString& label) const noexcept -> bool;
    virtual auto AssignTransactionMemo(
        const TxidHex& id,
        const UnallocatedCString& label) const noexcept -> bool
    {
        return false;
    }
    auto CalculateAddress(
        const opentxs::blockchain::Type chain,
        const Style format,
        const Data& pubkey) const noexcept -> UnallocatedCString;
    auto Confirm(const Key key, const opentxs::blockchain::block::Txid& tx)
        const noexcept -> bool;
    auto Contacts() const noexcept -> const api::session::Contacts&
    {
        return contacts_;
    }
    auto DecodeAddress(const UnallocatedCString& encoded) const noexcept
        -> DecodedAddress;
    auto EncodeAddress(
        const Style style,
        const opentxs::blockchain::Type chain,
        const Data& data) const noexcept -> UnallocatedCString;
    auto GetKey(const Key& id) const noexcept(false)
        -> const opentxs::blockchain::crypto::Element&;
    auto HDSubaccount(const identifier::Nym& nymID, const Identifier& accountID)
        const noexcept(false) -> const opentxs::blockchain::crypto::HD&;
    using SyncState = UnallocatedVector<opentxs::network::p2p::State>;
    virtual auto IndexItem(const ReadView bytes) const noexcept -> PatternID;
    virtual auto KeyEndpoint() const noexcept -> std::string_view;
    virtual auto KeyGenerated(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& account,
        const Identifier& subaccount,
        const opentxs::blockchain::crypto::SubaccountType type,
        const opentxs::blockchain::crypto::Subchain subchain) const noexcept
        -> void;
    virtual auto LoadTransactionBitcoin(const TxidHex& txid) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Transaction>;
    virtual auto LoadTransactionBitcoin(const Txid& txid) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Transaction>;
    auto LookupAccount(const Identifier& id) const noexcept -> AccountData;
    virtual auto LookupContacts(const Data& pubkeyHash) const noexcept
        -> ContactList;
    auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const opentxs::blockchain::Type derivationChain,
        const opentxs::blockchain::Type targetChain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier;
    auto NewNym(const identifier::Nym& id) const noexcept -> void;
    auto NewPaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const opentxs::blockchain::Type chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier;
    auto Owner(const Identifier& accountID) const noexcept
        -> const identifier::Nym&
    {
        return accounts_.Owner(accountID);
    }
    auto Owner(const Key& key) const noexcept -> const identifier::Nym&;
    auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode&;
    auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const opentxs::blockchain::Type chain,
        const PasswordPrompt& reason) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode&;
    virtual auto ProcessContact(const Contact& contact) const noexcept -> bool;
    virtual auto ProcessMergedContact(
        const Contact& parent,
        const Contact& child) const noexcept -> bool;
    virtual auto ProcessTransaction(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::bitcoin::Transaction& in,
        const PasswordPrompt& reason) const noexcept -> bool;
    auto PubkeyHash(const opentxs::blockchain::Type chain, const Data& pubkey)
        const noexcept(false) -> OTData;
    auto RecipientContact(const Key& key) const noexcept -> OTIdentifier;
    auto Release(const Key key) const noexcept -> bool;
    virtual auto ReportScan(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& owner,
        const opentxs::blockchain::crypto::SubaccountType type,
        const Identifier& account,
        const Subchain subchain,
        const opentxs::blockchain::block::Position& progress) const noexcept
        -> void;
    auto SenderContact(const Key& key) const noexcept -> OTIdentifier;
    auto SubaccountList(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain) const noexcept
        -> UnallocatedSet<OTIdentifier>
    {
        return accounts_.List(nymID, chain);
    }
    virtual auto Unconfirm(
        const Key key,
        const opentxs::blockchain::block::Txid& tx,
        const Time time) const noexcept -> bool;
    virtual auto UpdateBalance(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept -> void;
    virtual auto UpdateBalance(
        const identifier::Nym& owner,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept -> void;
    virtual auto UpdateElement(
        UnallocatedVector<ReadView>& pubkeyHashes) const noexcept -> void;
    auto Wallet(const opentxs::blockchain::Type chain) const noexcept(false)
        -> const opentxs::blockchain::crypto::Wallet&;

    virtual auto Init() noexcept -> void;

    Imp(const api::Session& api,
        const api::session::Contacts& contacts,
        api::crypto::Blockchain& parent) noexcept;

    virtual ~Imp() = default;

protected:
    const api::Session& api_;
    const api::session::Contacts& contacts_;
    const DecodedAddress blank_;
    mutable std::mutex lock_;
    mutable IDLock nym_lock_;
    mutable blockchain::AccountCache accounts_;
    mutable blockchain::Wallets wallets_;

    auto bip44_type(const UnitType type) const noexcept -> Bip44Type;
    auto decode_bech23(const UnallocatedCString& encoded) const noexcept
        -> std::optional<DecodedAddress>;
    auto decode_legacy(const UnallocatedCString& encoded) const noexcept
        -> std::optional<DecodedAddress>;
    auto get_node(const Identifier& accountID) const noexcept(false)
        -> opentxs::blockchain::crypto::Subaccount&;
    auto init_path(
        const UnallocatedCString& root,
        const UnitType chain,
        const Bip32Index account,
        const opentxs::blockchain::crypto::HDProtocol standard,
        proto::HDPath& path) const noexcept -> void;
    auto new_payment_code(
        const Lock& lock,
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const opentxs::blockchain::Type chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier;
    auto p2pkh(const opentxs::blockchain::Type chain, const Data& pubkeyHash)
        const noexcept -> UnallocatedCString;
    auto p2sh(const opentxs::blockchain::Type chain, const Data& scriptHash)
        const noexcept -> UnallocatedCString;
    auto p2wpkh(const opentxs::blockchain::Type chain, const Data& pubkeyHash)
        const noexcept -> UnallocatedCString;
    auto nym_mutex(const identifier::Nym& nym) const noexcept -> std::mutex&;
    auto validate_nym(const identifier::Nym& nymID) const noexcept -> bool;

private:
    virtual auto notify_new_account(
        const Identifier& id,
        const identifier::Nym& owner,
        opentxs::blockchain::Type chain,
        opentxs::blockchain::crypto::SubaccountType type) const noexcept -> void
    {
    }
};
}  // namespace opentxs::api::crypto::imp
