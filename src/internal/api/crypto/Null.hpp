// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Nym.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Contacts;
class Factory;
}  // namespace session
}  // namespace api
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::blank
{
class Blockchain final : public crypto::internal::Blockchain
{
public:
    auto Account(const identifier::Nym&, const Chain) const noexcept(false)
        -> const opentxs::blockchain::crypto::Account& final
    {
        throw std::runtime_error{""};
    }
    auto AccountList(const identifier::Nym&) const noexcept
        -> UnallocatedSet<OTIdentifier> final
    {
        return {};
    }
    auto AccountList(const Chain) const noexcept
        -> UnallocatedSet<OTIdentifier> final
    {
        return {};
    }
    auto AccountList() const noexcept -> UnallocatedSet<OTIdentifier> final
    {
        return {};
    }
    auto ActivityDescription(
        const identifier::Nym&,
        const Identifier& thread,
        const UnallocatedCString&) const noexcept -> UnallocatedCString final
    {
        return {};
    }
    auto ActivityDescription(
        const identifier::Nym&,
        const Chain,
        const opentxs::blockchain::block::bitcoin::Transaction&) const noexcept
        -> UnallocatedCString final
    {
        return {};
    }
    auto AssignContact(
        const identifier::Nym&,
        const Identifier&,
        const Subchain,
        const Bip32Index,
        const Identifier&) const noexcept -> bool final
    {
        return {};
    }
    auto AssignLabel(
        const identifier::Nym&,
        const Identifier&,
        const Subchain,
        const Bip32Index,
        const UnallocatedCString&) const noexcept -> bool final
    {
        return {};
    }
    auto AssignTransactionMemo(const TxidHex&, const UnallocatedCString&)
        const noexcept -> bool final
    {
        return {};
    }
    auto CalculateAddress(
        const opentxs::blockchain::Type,
        const opentxs::blockchain::crypto::AddressStyle,
        const Data&) const noexcept -> UnallocatedCString final
    {
        return {};
    }
    auto Confirm(const Key, const opentxs::blockchain::block::Txid&)
        const noexcept -> bool final
    {
        return {};
    }
    auto Contacts() const noexcept -> const api::session::Contacts& final
    {
        OT_FAIL;  // TODO return a blank object
    }
    auto DecodeAddress(const UnallocatedCString&) const noexcept
        -> DecodedAddress final
    {
        static const auto data = OTData{id_.get()};

        return {data, {}, {}, {}};
    }
    auto EncodeAddress(const Style, const Chain, const Data&) const noexcept
        -> UnallocatedCString final
    {
        return {};
    }
    auto GetKey(const Key&) const noexcept(false)
        -> const opentxs::blockchain::crypto::Element& final
    {
        throw std::out_of_range{""};
    }
    auto HDSubaccount(const identifier::Nym&, const Identifier&) const
        noexcept(false) -> const opentxs::blockchain::crypto::HD& final
    {
        throw std::out_of_range{""};
    }
    auto IndexItem(const ReadView) const noexcept -> PatternID final
    {
        return {};
    }
    auto KeyEndpoint() const noexcept -> std::string_view final
    {
        static const auto null = CString{};

        return null;
    }
    auto KeyGenerated(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& account,
        const Identifier& subaccount,
        const opentxs::blockchain::crypto::SubaccountType type,
        const opentxs::blockchain::crypto::Subchain subchain) const noexcept
        -> void final
    {
    }
    auto LoadTransactionBitcoin(const Txid&) const noexcept -> std::unique_ptr<
        const opentxs::blockchain::block::bitcoin::Transaction> final
    {
        return {};
    }
    auto LoadTransactionBitcoin(const TxidHex&) const noexcept
        -> std::unique_ptr<
            const opentxs::blockchain::block::bitcoin::Transaction> final
    {
        return {};
    }
    auto LookupAccount(const Identifier&) const noexcept -> AccountData final
    {
        return {{}, {id_}};
    }
    auto LookupContacts(const UnallocatedCString&) const noexcept
        -> ContactList final
    {
        return {};
    }
    auto LookupContacts(const Data&) const noexcept -> ContactList final
    {
        return {};
    }
    auto NewHDSubaccount(
        const identifier::Nym&,
        const opentxs::blockchain::crypto::HDProtocol,
        const Chain,
        const PasswordPrompt&) const noexcept -> OTIdentifier final
    {
        return {id_.get()};
    }
    auto NewHDSubaccount(
        const identifier::Nym&,
        const opentxs::blockchain::crypto::HDProtocol,
        const Chain,
        const Chain,
        const PasswordPrompt&) const noexcept -> OTIdentifier final
    {
        return {id_.get()};
    }
    auto NewNym(const identifier::Nym& id) const noexcept -> void final {}
    auto NewPaymentCodeSubaccount(
        const identifier::Nym&,
        const opentxs::PaymentCode&,
        const opentxs::PaymentCode&,
        const proto::HDPath&,
        const Chain,
        const PasswordPrompt&) const noexcept -> OTIdentifier final
    {
        return {id_.get()};
    }
    auto NewPaymentCodeSubaccount(
        const identifier::Nym&,
        const opentxs::PaymentCode&,
        const opentxs::PaymentCode&,
        const ReadView&,
        const Chain,
        const PasswordPrompt&) const noexcept -> OTIdentifier final
    {
        return {id_.get()};
    }
    auto Owner(const Identifier&) const noexcept -> const identifier::Nym& final
    {
        return id_;
    }
    auto Owner(const Key&) const noexcept -> const identifier::Nym& final
    {
        return id_;
    }
    auto PaymentCodeSubaccount(
        const identifier::Nym&,
        const opentxs::PaymentCode&,
        const opentxs::PaymentCode&,
        const proto::HDPath&,
        const Chain,
        const PasswordPrompt&) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode& final
    {
        throw std::out_of_range{""};
    }
    auto PaymentCodeSubaccount(const identifier::Nym&, const Identifier&) const
        noexcept(false) -> const opentxs::blockchain::crypto::PaymentCode& final
    {
        throw std::out_of_range{""};
    }
    auto ProcessContact(const Contact&) const noexcept -> bool final
    {
        return {};
    }
    auto ProcessMergedContact(const Contact&, const Contact&) const noexcept
        -> bool final
    {
        return {};
    }
    auto ProcessTransaction(
        const Chain,
        const opentxs::blockchain::block::bitcoin::Transaction&,
        const PasswordPrompt&) const noexcept -> bool final
    {
        return {};
    }
    auto PubkeyHash(const opentxs::blockchain::Type, const Data&) const
        noexcept(false) -> OTData final
    {
        throw std::runtime_error{""};
    }
    auto ReportScan(
        const Chain,
        const identifier::Nym&,
        const opentxs::blockchain::crypto::SubaccountType,
        const Identifier&,
        const opentxs::blockchain::crypto::Subchain,
        const opentxs::blockchain::block::Position&) const noexcept
        -> void final
    {
    }
    auto RecipientContact(const Key&) const noexcept -> OTIdentifier final
    {
        return {id_.get()};
    }
    auto Release(const Key) const noexcept -> bool final { return {}; }
    auto SenderContact(const Key&) const noexcept -> OTIdentifier final
    {
        return {id_.get()};
    }
    auto SubaccountList(const identifier::Nym&, const Chain) const noexcept
        -> UnallocatedSet<OTIdentifier> final
    {
        return {};
    }
    auto Unconfirm(
        const Key,
        const opentxs::blockchain::block::Txid&,
        const Time time) const noexcept -> bool final
    {
        return {};
    }
    auto UpdateBalance(
        const opentxs::blockchain::Type,
        const opentxs::blockchain::Balance) const noexcept -> void final
    {
    }
    auto UpdateBalance(
        const identifier::Nym&,
        const opentxs::blockchain::Type,
        const opentxs::blockchain::Balance) const noexcept -> void final
    {
    }
    auto UpdateElement(UnallocatedVector<ReadView>&) const noexcept
        -> void final
    {
    }
    auto Wallet(const Chain) const noexcept(false)
        -> const opentxs::blockchain::crypto::Wallet& final
    {
        throw std::runtime_error{""};
    }

    auto Init() noexcept -> void final {}

    Blockchain(const session::Factory& factory) noexcept;

    ~Blockchain() final = default;

private:
    const OTNymID id_;
};
}  // namespace opentxs::api::crypto::blank
