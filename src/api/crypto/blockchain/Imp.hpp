// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::Bip44Type
// IWYU pragma: no_forward_declare opentxs::UnitType
// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::HDProtocol
// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::SubaccountType
// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::Subchain
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"
// IWYU pragma: no_include "opentxs/core/Data.hpp"
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
#include "internal/blockchain/block/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Bip44Type.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/network/otdht/State.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
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
class Contacts;
}  // namespace session

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
namespace otdht
{
class Data;
class State;
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
class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::crypto::imp
{
struct Blockchain::Imp {
    using IDLock = UnallocatedMap<identifier::Generic, std::mutex>;

    auto Account(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain) const noexcept(false)
        -> const opentxs::blockchain::crypto::Account&;
    auto AccountList(const identifier::Nym& nymID) const noexcept
        -> UnallocatedSet<identifier::Generic>;
    auto AccountList(const opentxs::blockchain::Type chain) const noexcept
        -> UnallocatedSet<identifier::Generic>;
    auto AccountList() const noexcept -> UnallocatedSet<identifier::Generic>;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const UnallocatedCString& threadItemID) const noexcept
        -> UnallocatedCString;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::bitcoin::block::Transaction& transaction)
        const noexcept -> UnallocatedCString;
    auto address_prefix(
        const Style style,
        const opentxs::blockchain::Type chain) const noexcept(false)
        -> ByteArray;
    auto AssignContact(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const identifier::Generic& contactID) const noexcept -> bool;
    auto AssignLabel(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const UnallocatedCString& label) const noexcept -> bool;
    virtual auto AssignTransactionMemo(
        const TxidHex& id,
        const UnallocatedCString& label) const noexcept -> bool
    {
        return false;
    }
    auto BalanceOracleEndpoint() const noexcept -> std::string_view
    {
        return balance_oracle_endpoint_;
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
    auto HDSubaccount(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID) const noexcept(false)
        -> const opentxs::blockchain::crypto::HD&;
    using SyncState = UnallocatedVector<opentxs::network::otdht::State>;
    virtual auto IndexItem(const ReadView bytes) const noexcept
        -> opentxs::blockchain::block::ElementHash;
    virtual auto KeyEndpoint() const noexcept -> std::string_view;
    virtual auto KeyGenerated(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& account,
        const identifier::Generic& subaccount,
        const opentxs::blockchain::crypto::SubaccountType type,
        const opentxs::blockchain::crypto::Subchain subchain) const noexcept
        -> void;
    virtual auto LoadTransactionBitcoin(const TxidHex& txid) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::bitcoin::block::Transaction>;
    virtual auto LoadTransactionBitcoin(const Txid& txid) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::bitcoin::block::Transaction>;
    auto LookupAccount(const identifier::Generic& id) const noexcept
        -> AccountData;
    virtual auto LookupContacts(const Data& pubkeyHash) const noexcept
        -> ContactList;
    auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const opentxs::blockchain::Type derivationChain,
        const opentxs::blockchain::Type targetChain,
        const PasswordPrompt& reason) const noexcept -> identifier::Generic;
    auto NewNym(const identifier::Nym& id) const noexcept -> void;
    auto NewPaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const opentxs::blockchain::Type chain,
        const PasswordPrompt& reason) const noexcept -> identifier::Generic;
    auto Owner(const identifier::Generic& accountID) const noexcept
        -> const identifier::Nym&
    {
        return accounts_.Owner(accountID);
    }
    auto Owner(const Key& key) const noexcept -> const identifier::Nym&;
    auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID) const noexcept(false)
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
    virtual auto ProcessTransactions(
        const opentxs::blockchain::Type chain,
        Set<std::shared_ptr<opentxs::blockchain::bitcoin::block::Transaction>>&&
            transactions,
        const PasswordPrompt& reason) const noexcept -> bool;
    auto PubkeyHash(const opentxs::blockchain::Type chain, const Data& pubkey)
        const noexcept(false) -> ByteArray;
    auto RecipientContact(const Key& key) const noexcept -> identifier::Generic;
    auto Release(const Key key) const noexcept -> bool;
    virtual auto ReportScan(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& owner,
        const opentxs::blockchain::crypto::SubaccountType type,
        const identifier::Generic& account,
        const Subchain subchain,
        const opentxs::blockchain::block::Position& progress) const noexcept
        -> void;
    auto SenderContact(const Key& key) const noexcept -> identifier::Generic;
    virtual auto Start(std::shared_ptr<const api::Session> api) noexcept -> void
    {
    }
    auto SubaccountList(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain) const noexcept
        -> UnallocatedSet<identifier::Generic>
    {
        return accounts_.List(nymID, chain);
    }
    virtual auto Unconfirm(
        const Key key,
        const opentxs::blockchain::block::Txid& tx,
        const Time time) const noexcept -> bool;
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
    const CString balance_oracle_endpoint_;
    const api::crypto::Blockchain& parent_;
    mutable std::mutex lock_;
    mutable IDLock nym_lock_;
    mutable blockchain::AccountCache accounts_;
    mutable blockchain::Wallets wallets_;

    auto bip44_type(const UnitType type) const noexcept -> Bip44Type;
    auto decode_bech23(const UnallocatedCString& encoded) const noexcept
        -> std::optional<DecodedAddress>;
    auto decode_legacy(const UnallocatedCString& encoded) const noexcept
        -> std::optional<DecodedAddress>;
    auto get_node(const identifier::Generic& accountID) const noexcept(false)
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
        const PasswordPrompt& reason) const noexcept -> identifier::Generic;
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
        const identifier::Generic& id,
        const identifier::Nym& owner,
        opentxs::blockchain::Type chain,
        opentxs::blockchain::crypto::SubaccountType type) const noexcept -> void
    {
    }
};
}  // namespace opentxs::api::crypto::imp
