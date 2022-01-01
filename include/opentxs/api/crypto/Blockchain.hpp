// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: no_include "opentxs/blockchain/crypto/HDProtocol.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <set>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Time.hpp"

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
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
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

namespace contact
{
class Contact;
}  // namespace contact

class Identifier;
class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs

namespace opentxs::api::crypto
{
class OPENTXS_EXPORT Blockchain
{
public:
    using Chain = opentxs::blockchain::Type;
    using Key = opentxs::blockchain::crypto::Key;
    using Style = opentxs::blockchain::crypto::AddressStyle;
    using Subchain = opentxs::blockchain::crypto::Subchain;
    using DecodedAddress =
        std::tuple<OTData, Style, std::pmr::set<Chain>, bool>;
    using ContactList = std::pmr::set<OTIdentifier>;
    using Txid = opentxs::blockchain::block::Txid;
    using TxidHex = std::string;
    using PatternID = opentxs::blockchain::PatternID;
    using AccountData = std::pair<Chain, OTNymID>;

    // Throws std::out_of_range for invalid chains
    static auto Bip44(Chain chain) noexcept(false) -> Bip44Type;
    static auto Bip44Path(
        Chain chain,
        const std::string& seed,
        AllocateOutput destination) noexcept(false) -> bool;

    /// Throws std::runtime_error if chain is invalid
    virtual auto Account(const identifier::Nym& nymID, const Chain chain) const
        noexcept(false) -> const opentxs::blockchain::crypto::Account& = 0;
    virtual auto AccountList(const identifier::Nym& nymID) const noexcept
        -> std::pmr::set<OTIdentifier> = 0;
    virtual auto AccountList(const Chain chain) const noexcept
        -> std::pmr::set<OTIdentifier> = 0;
    virtual auto AccountList() const noexcept
        -> std::pmr::set<OTIdentifier> = 0;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const Identifier& thread,
        const std::string& threadItemID) const noexcept -> std::string = 0;
    virtual auto ActivityDescription(
        const identifier::Nym& nym,
        const Chain chain,
        const opentxs::blockchain::block::bitcoin::Transaction& transaction)
        const noexcept -> std::string = 0;
    virtual auto AssignContact(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const Identifier& label) const noexcept -> bool = 0;
    virtual auto AssignLabel(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const std::string& label) const noexcept -> bool = 0;
    virtual auto AssignTransactionMemo(
        const TxidHex& id,
        const std::string& label) const noexcept -> bool = 0;
    virtual auto CalculateAddress(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::crypto::AddressStyle format,
        const Data& pubkey) const noexcept -> std::string = 0;
    virtual auto Confirm(
        const Key key,
        const opentxs::blockchain::block::Txid& tx) const noexcept -> bool = 0;
    virtual auto DecodeAddress(const std::string& encoded) const noexcept
        -> DecodedAddress = 0;
    virtual auto EncodeAddress(
        const Style style,
        const Chain chain,
        const Data& data) const noexcept -> std::string = 0;
    /// Throws std::out_of_range if the specified key does not exist
    virtual auto GetKey(const Key& id) const noexcept(false)
        -> const opentxs::blockchain::crypto::Element& = 0;
    /// Throws std::out_of_range if the specified account does not exist
    virtual auto HDSubaccount(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept(false)
        -> const opentxs::blockchain::crypto::HD& = 0;
    virtual auto IndexItem(const ReadView bytes) const noexcept
        -> PatternID = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const crypto::internal::Blockchain& = 0;
    virtual auto LoadTransactionBitcoin(const Txid& id) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Transaction> = 0;
    virtual auto LoadTransactionBitcoin(const TxidHex& id) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Transaction> = 0;
    virtual auto LookupAccount(const Identifier& id) const noexcept
        -> AccountData = 0;
    virtual auto LookupContacts(const std::string& address) const noexcept
        -> ContactList = 0;
    virtual auto LookupContacts(const Data& pubkeyHash) const noexcept
        -> ContactList = 0;
    virtual auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier = 0;
    virtual auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const Chain derivationChain,
        const Chain targetChain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier = 0;
    virtual auto NewPaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const ReadView& view,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier = 0;
    virtual auto Owner(const Identifier& accountID) const noexcept
        -> const identifier::Nym& = 0;
    virtual auto Owner(const Key& key) const noexcept
        -> const identifier::Nym& = 0;
    /// Throws std::out_of_range if the specified account does not exist
    virtual auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode& = 0;
    virtual auto RecipientContact(const Key& key) const noexcept
        -> OTIdentifier = 0;
    virtual auto Release(const Key key) const noexcept -> bool = 0;
    virtual auto SenderContact(const Key& key) const noexcept
        -> OTIdentifier = 0;
    virtual auto SubaccountList(const identifier::Nym& nymID, const Chain chain)
        const noexcept -> std::pmr::set<OTIdentifier> = 0;
    virtual auto Unconfirm(
        const Key key,
        const opentxs::blockchain::block::Txid& tx,
        const Time time = Clock::now()) const noexcept -> bool = 0;
    /// Throws std::runtime_error if chain is invalid
    virtual auto Wallet(const Chain chain) const noexcept(false)
        -> const opentxs::blockchain::crypto::Wallet& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> crypto::internal::Blockchain& = 0;

    OPENTXS_NO_EXPORT virtual ~Blockchain() = default;

protected:
    Blockchain() noexcept = default;

private:
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    auto operator=(const Blockchain&) -> Blockchain& = delete;
    auto operator=(Blockchain&&) -> Blockchain& = delete;
};
}  // namespace opentxs::api::crypto
