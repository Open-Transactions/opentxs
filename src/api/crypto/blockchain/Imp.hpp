// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Data.hpp"

#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string_view>

#include "api/crypto/blockchain/AccountCache.hpp"
#include "api/crypto/blockchain/Blockchain.hpp"
#include "api/crypto/blockchain/Wallets.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/otdht/State.hpp"
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
class Contacts;
}  // namespace session

class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Transaction;
}  // namespace block

namespace crypto
{
class Account;
class Element;
class HD;
class PaymentCode;
class Subaccount;
class Wallet;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

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

namespace opentxs::api::crypto::imp
{
struct Blockchain::Imp {
    using IDLock = UnallocatedMap<identifier::Generic, std::mutex>;

    auto Account(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain) const noexcept(false)
        -> const opentxs::blockchain::crypto::Account&;
    auto AccountList(const identifier::Nym& nymID) const noexcept
        -> UnallocatedSet<identifier::Account>;
    auto AccountList(const opentxs::blockchain::Type chain) const noexcept
        -> UnallocatedSet<identifier::Account>;
    auto AccountList() const noexcept -> UnallocatedSet<identifier::Account>;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const std::string_view threadItemID,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> UnallocatedCString;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::Transaction& transaction)
        const noexcept -> UnallocatedCString;
    auto address_prefix(
        const Style style,
        const opentxs::blockchain::Type chain) const noexcept(false)
        -> ByteArray;
    auto AssignContact(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const identifier::Generic& contactID) const noexcept -> bool;
    auto AssignLabel(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const std::string_view label) const noexcept -> bool;
    virtual auto AssignTransactionMemo(
        const TxidHex& id,
        const std::string_view label,
        alloc::Default monotonic) const noexcept -> bool;
    auto BalanceOracleEndpoint() const noexcept -> std::string_view
    {
        return balance_oracle_endpoint_;
    }
    auto CalculateAddress(
        const opentxs::blockchain::Type chain,
        const Style format,
        const Data& pubkey) const noexcept -> UnallocatedCString;
    auto Confirm(
        const Key key,
        const opentxs::blockchain::block::TransactionHash& tx) const noexcept
        -> bool;
    auto Contacts() const noexcept -> const api::session::Contacts&
    {
        return contacts_;
    }
    auto DecodeAddress(const std::string_view encoded) const noexcept
        -> DecodedAddress;
    auto EncodeAddress(
        const Style style,
        const opentxs::blockchain::Type chain,
        const Data& data) const noexcept -> UnallocatedCString;
    auto GetKey(const Key& id) const noexcept(false)
        -> const opentxs::blockchain::crypto::Element&;
    auto HDSubaccount(
        const identifier::Nym& nymID,
        const identifier::Account& accountID) const noexcept(false)
        -> const opentxs::blockchain::crypto::HD&;
    using SyncState = UnallocatedVector<opentxs::network::otdht::State>;
    virtual auto IndexItem(const ReadView bytes) const noexcept
        -> opentxs::blockchain::block::ElementHash;
    virtual auto KeyEndpoint() const noexcept -> std::string_view;
    virtual auto KeyGenerated(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& account,
        const identifier::Account& subaccount,
        const opentxs::blockchain::crypto::SubaccountType type,
        const opentxs::blockchain::crypto::Subchain subchain) const noexcept
        -> void;
    virtual auto LoadTransaction(
        const TxidHex& txid,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept
        -> opentxs::blockchain::block::Transaction;
    virtual auto LoadTransaction(
        const Txid& txid,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept
        -> opentxs::blockchain::block::Transaction;
    auto LookupAccount(const identifier::Account& id) const noexcept
        -> AccountData;
    virtual auto LookupContacts(const Data& pubkeyHash) const noexcept
        -> ContactList;
    auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const opentxs::blockchain::Type derivationChain,
        const opentxs::blockchain::Type targetChain,
        const PasswordPrompt& reason) const noexcept -> identifier::Account;
    auto NewNym(const identifier::Nym& id) const noexcept -> void;
    auto NewPaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const opentxs::blockchain::Type chain,
        const PasswordPrompt& reason) const noexcept -> identifier::Account;
    auto Owner(const identifier::Account& accountID) const noexcept
        -> const identifier::Nym&
    {
        return accounts_.Owner(accountID);
    }
    auto Owner(const Key& key) const noexcept -> const identifier::Nym&;
    auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const identifier::Account& accountID) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode&;
    auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath path,
        const opentxs::blockchain::Type chain,
        const PasswordPrompt& reason) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode&;
    virtual auto ProcessContact(
        const Contact& contact,
        alloc::Default monotonic) const noexcept -> bool;
    virtual auto ProcessMergedContact(
        const Contact& parent,
        const Contact& child,
        alloc::Default monotonic) const noexcept -> bool;
    virtual auto ProcessTransactions(
        const opentxs::blockchain::Type chain,
        Set<opentxs::blockchain::block::Transaction>&& transactions,
        const PasswordPrompt& reason,
        alloc::Default monotonic) const noexcept -> bool;
    auto PubkeyHash(const opentxs::blockchain::Type chain, const Data& pubkey)
        const noexcept(false) -> ByteArray;
    auto RecipientContact(const Key& key) const noexcept -> identifier::Generic;
    auto Release(const Key key) const noexcept -> bool;
    virtual auto ReportScan(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& owner,
        const opentxs::blockchain::crypto::SubaccountType type,
        const identifier::Account& account,
        const Subchain subchain,
        const opentxs::blockchain::block::Position& progress) const noexcept
        -> void;
    auto SenderContact(const Key& key) const noexcept -> identifier::Generic;
    virtual auto Start(std::shared_ptr<const api::Session> api) noexcept
        -> void;
    auto SubaccountList(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain) const noexcept
        -> UnallocatedSet<identifier::Account>
    {
        return accounts_.List(nymID, chain);
    }
    virtual auto Unconfirm(
        const Key key,
        const opentxs::blockchain::block::TransactionHash& tx,
        const Time time,
        alloc::Default monotonic) const noexcept -> bool;
    virtual auto UpdateElement(
        std::span<const ReadView> pubkeyHashes,
        alloc::Default monotonic) const noexcept -> void;
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
    auto decode_bech23(const std::string_view encoded) const noexcept
        -> std::optional<DecodedAddress>;
    auto decode_legacy(const std::string_view encoded) const noexcept
        -> std::optional<DecodedAddress>;
    auto get_node(const identifier::Account& accountID) const noexcept(false)
        -> opentxs::blockchain::crypto::Subaccount&;
    auto init_path(
        const std::string_view root,
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
        const PasswordPrompt& reason) const noexcept -> identifier::Account;
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
        const identifier::Account& id,
        const identifier::Nym& owner,
        opentxs::blockchain::Type chain,
        opentxs::blockchain::crypto::SubaccountType type) const noexcept -> void
    {
    }
};
}  // namespace opentxs::api::crypto::imp
