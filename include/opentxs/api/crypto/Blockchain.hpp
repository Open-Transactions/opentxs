// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
namespace internal
{
class Blockchain;
}  // namespace internal
}  // namespace crypto
}  // namespace api

namespace blockchain
{
namespace block
{
class Transaction;
}  // namespace block

namespace block
{
class TransactionHash;
}  // namespace block

namespace crypto
{
class Account;
class Element;
class HD;
class PaymentCode;
class Wallet;
}  // namespace crypto

namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

namespace crypto
{
namespace asymmetric
{
namespace key
{
class EllipticCurve;
}  // namespace key
}  // namespace asymmetric
}  // namespace crypto

namespace identifier
{
class Account;
class Generic;
class Nym;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

class Contact;
class PasswordPrompt;
class PaymentCode;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto
{
/**
 The api::crypto::Blockchain API is used for accessing blockchain-related crypto
 functionality.
 */
class OPENTXS_EXPORT Blockchain
{
public:
    using Chain = opentxs::blockchain::Type;
    using Key = opentxs::blockchain::crypto::Key;
    using Style = opentxs::blockchain::crypto::AddressStyle;
    using Subchain = opentxs::blockchain::crypto::Subchain;
    using DecodedAddress = std::tuple<ByteArray, Style, Set<Chain>, bool>;
    using ContactList = UnallocatedSet<identifier::Generic>;
    using Txid = opentxs::blockchain::block::TransactionHash;
    using TxidHex = UnallocatedCString;
    using AccountData = std::pair<Chain, identifier::Nym>;

    // Throws std::out_of_range for invalid chains
    static auto Bip44(Chain chain) noexcept(false)
        -> opentxs::blockchain::crypto::Bip44Type;
    static auto Bip44Path(
        Chain chain,
        const opentxs::crypto::SeedID& seed,
        Writer&& destination) noexcept(false) -> bool;

    /// Throws std::runtime_error if chain is invalid
    virtual auto Account(const identifier::Nym& nymID, const Chain chain) const
        noexcept(false) -> const opentxs::blockchain::crypto::Account& = 0;
    virtual auto AccountList(const identifier::Nym& nymID) const noexcept
        -> UnallocatedSet<identifier::Account> = 0;
    virtual auto AccountList(const Chain chain) const noexcept
        -> UnallocatedSet<identifier::Account> = 0;
    virtual auto AccountList() const noexcept
        -> UnallocatedSet<identifier::Account> = 0;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const identifier::Generic& thread,
        const UnallocatedCString& threadItemID) const noexcept
        -> UnallocatedCString = 0;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const Chain chain,
        const opentxs::blockchain::block::Transaction& transaction)
        const noexcept -> UnallocatedCString = 0;
    virtual auto AssignContact(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        const Subchain subchain,
        const opentxs::crypto::Bip32Index index,
        const identifier::Generic& contact) const noexcept -> bool = 0;
    virtual auto AssignLabel(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        const Subchain subchain,
        const opentxs::crypto::Bip32Index index,
        const UnallocatedCString& label) const noexcept -> bool = 0;
    virtual auto AssignTransactionMemo(
        const TxidHex& id,
        const UnallocatedCString& label) const noexcept -> bool = 0;
    virtual auto Confirm(const Key key, const Txid& tx) const noexcept
        -> bool = 0;
    virtual auto DecodeAddress(std::string_view encoded) const noexcept
        -> DecodedAddress = 0;
    virtual auto EncodeAddress(
        const Style style,
        const Chain chain,
        const Data& data) const noexcept -> UnallocatedCString = 0;
    virtual auto EncodeAddress(
        const Style style,
        const Chain chain,
        const opentxs::crypto::asymmetric::key::EllipticCurve& pubkey)
        const noexcept -> UnallocatedCString = 0;
    /// Throws std::out_of_range if the specified key does not exist
    virtual auto GetKey(const Key& id) const noexcept(false)
        -> const opentxs::blockchain::crypto::Element& = 0;
    /// Throws std::out_of_range if the specified account does not exist
    virtual auto HDSubaccount(
        const identifier::Nym& nymID,
        const identifier::Account& accountID) const noexcept(false)
        -> const opentxs::blockchain::crypto::HD& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const crypto::internal::Blockchain& = 0;
    virtual auto LoadOrCreateSubaccount(
        const identifier::Nym& nym,
        const opentxs::PaymentCode& remote,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept
        -> const opentxs::blockchain::crypto::PaymentCode& = 0;
    virtual auto LoadOrCreateSubaccount(
        const identity::Nym& nym,
        const opentxs::PaymentCode& remote,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept
        -> const opentxs::blockchain::crypto::PaymentCode& = 0;
    virtual auto LoadTransaction(const Txid& id) const noexcept
        -> opentxs::blockchain::block::Transaction = 0;
    virtual auto LoadTransaction(const TxidHex& id) const noexcept
        -> opentxs::blockchain::block::Transaction = 0;
    virtual auto LookupAccount(const identifier::Account& id) const noexcept
        -> AccountData = 0;
    virtual auto LookupContacts(
        const UnallocatedCString& address) const noexcept -> ContactList = 0;
    virtual auto LookupContacts(const Data& pubkeyHash) const noexcept
        -> ContactList = 0;
    virtual auto NewEthereumSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept -> identifier::Account = 0;
    virtual auto NewEthereumSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const Chain derivationChain,
        const Chain targetChain,
        const PasswordPrompt& reason) const noexcept -> identifier::Account = 0;
    virtual auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept -> identifier::Account = 0;
    virtual auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const Chain derivationChain,
        const Chain targetChain,
        const PasswordPrompt& reason) const noexcept -> identifier::Account = 0;
    virtual auto Owner(const identifier::Account& accountID) const noexcept
        -> const identifier::Nym& = 0;
    virtual auto Owner(const Key& key) const noexcept
        -> const identifier::Nym& = 0;
    /// Throws std::out_of_range if the specified account does not exist
    virtual auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const identifier::Account& accountID) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode& = 0;
    virtual auto RecipientContact(const Key& key) const noexcept
        -> identifier::Generic = 0;
    virtual auto Release(const Key key) const noexcept -> bool = 0;
    virtual auto SenderContact(const Key& key) const noexcept
        -> identifier::Generic = 0;
    virtual auto SubaccountList(const identifier::Nym& nymID, const Chain chain)
        const noexcept -> UnallocatedSet<identifier::Account> = 0;
    virtual auto Unconfirm(
        const Key key,
        const Txid& tx,
        const Time time = Clock::now()) const noexcept -> bool = 0;
    /// Throws std::runtime_error if chain is invalid
    virtual auto Wallet(const Chain chain) const noexcept(false)
        -> const opentxs::blockchain::crypto::Wallet& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> crypto::internal::Blockchain& = 0;

    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    auto operator=(const Blockchain&) -> Blockchain& = delete;
    auto operator=(Blockchain&&) -> Blockchain& = delete;

    OPENTXS_NO_EXPORT virtual ~Blockchain() = default;

protected:
    Blockchain() noexcept = default;
};
}  // namespace opentxs::api::crypto
