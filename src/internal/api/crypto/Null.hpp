// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/api/crypto/Blockchain.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Contacts;
class Factory;
}  // namespace session

class Crypto;
}  // namespace api

namespace blockchain
{
namespace block
{
class TransactionHash;
}  // namespace block
}  // namespace blockchain
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
        -> UnallocatedSet<identifier::Account> final
    {
        return {};
    }
    auto AccountList(const Chain) const noexcept
        -> UnallocatedSet<identifier::Account> final
    {
        return {};
    }
    auto AccountList() const noexcept
        -> UnallocatedSet<identifier::Account> final
    {
        return {};
    }
    auto ActivityDescription(
        const identifier::Nym&,
        const identifier::Generic& thread,
        const UnallocatedCString&) const noexcept -> UnallocatedCString final
    {
        return {};
    }
    auto API() const noexcept -> const api::Crypto& final { OT_FAIL; }
    auto ActivityDescription(
        const identifier::Nym&,
        const Chain,
        const opentxs::blockchain::block::Transaction&) const noexcept
        -> UnallocatedCString final
    {
        return {};
    }
    auto AssignContact(
        const identifier::Nym&,
        const identifier::Account&,
        const Subchain,
        const Bip32Index,
        const identifier::Generic&) const noexcept -> bool final
    {
        return {};
    }
    auto AssignLabel(
        const identifier::Nym&,
        const identifier::Account&,
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
    auto BalanceOracleEndpoint() const noexcept -> std::string_view final
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
    auto Confirm(const Key, const opentxs::blockchain::block::TransactionHash&)
        const noexcept -> bool final
    {
        return {};
    }
    auto Contacts() const noexcept -> const api::session::Contacts& final
    {
        OT_FAIL;  // TODO return a blank object
    }
    auto DecodeAddress(std::string_view) const noexcept -> DecodedAddress final
    {
        static const auto data = ByteArray{id_};

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
    auto GetNotificationStatus(const identifier::Nym&, alloc::Strategy alloc)
        const noexcept -> opentxs::blockchain::crypto::NotificationStatus final
    {
        return opentxs::blockchain::crypto::NotificationStatus{alloc.result_};
    }
    auto HDSubaccount(const identifier::Nym&, const identifier::Account&) const
        noexcept(false) -> const opentxs::blockchain::crypto::HD& final
    {
        throw std::out_of_range{""};
    }
    auto IndexItem(const ReadView) const noexcept
        -> opentxs::blockchain::block::ElementHash final
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
        const identifier::Account& subaccount,
        const opentxs::blockchain::crypto::SubaccountType type,
        const opentxs::blockchain::crypto::Subchain subchain) const noexcept
        -> void final
    {
    }
    auto LoadTransaction(const Txid&) const noexcept
        -> opentxs::blockchain::block::Transaction final
    {
        return {};
    }
    auto LoadTransaction(const TxidHex&) const noexcept
        -> opentxs::blockchain::block::Transaction final
    {
        return {};
    }
    auto LookupAccount(const identifier::Account&) const noexcept
        -> AccountData final
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
        const PasswordPrompt&) const noexcept -> identifier::Account final
    {
        return {account_};
    }
    auto NewHDSubaccount(
        const identifier::Nym&,
        const opentxs::blockchain::crypto::HDProtocol,
        const Chain,
        const Chain,
        const PasswordPrompt&) const noexcept -> identifier::Account final
    {
        return {account_};
    }
    auto NewNym(const identifier::Nym& id) const noexcept -> void final {}
    auto NewPaymentCodeSubaccount(
        const identifier::Nym&,
        const opentxs::PaymentCode&,
        const opentxs::PaymentCode&,
        const proto::HDPath&,
        const Chain,
        const PasswordPrompt&) const noexcept -> identifier::Account final
    {
        return {account_};
    }
    auto NewPaymentCodeSubaccount(
        const identifier::Nym&,
        const opentxs::PaymentCode&,
        const opentxs::PaymentCode&,
        const ReadView&,
        const Chain,
        const PasswordPrompt&) const noexcept -> identifier::Account final
    {
        return {account_};
    }
    auto Owner(const identifier::Account&) const noexcept
        -> const identifier::Nym& final
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
    auto PaymentCodeSubaccount(
        const identifier::Nym&,
        const identifier::Account&) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode& final
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
    auto ProcessTransactions(
        const Chain chain,
        Set<opentxs::blockchain::block::Transaction>&& transactions,
        const PasswordPrompt& reason) const noexcept -> bool final
    {
        return {};
    }
    auto PubkeyHash(const opentxs::blockchain::Type, const Data&) const
        noexcept(false) -> ByteArray final
    {
        throw std::runtime_error{""};
    }
    auto ReportScan(
        const Chain,
        const identifier::Nym&,
        const opentxs::blockchain::crypto::SubaccountType,
        const identifier::Account&,
        const opentxs::blockchain::crypto::Subchain,
        const opentxs::blockchain::block::Position&) const noexcept
        -> void final
    {
    }
    auto RecipientContact(const Key&) const noexcept
        -> identifier::Generic final
    {
        return {id_};
    }
    [[nodiscard]] auto RegisterAccount(
        const opentxs::blockchain::Type,
        const identifier::Nym&,
        const identifier::Account&) const noexcept -> bool final
    {
        return {};
    }
    [[nodiscard]] auto RegisterSubaccount(
        const opentxs::blockchain::crypto::SubaccountType,
        const opentxs::blockchain::Type,
        const identifier::Nym&,
        const identifier::Account&,
        const identifier::Account&) const noexcept -> bool final
    {
        return {};
    }
    auto Release(const Key) const noexcept -> bool final { return {}; }
    auto SenderContact(const Key&) const noexcept -> identifier::Generic final
    {
        return {id_};
    }
    auto Start(std::shared_ptr<const api::Session> api) noexcept -> void final
    {
    }
    auto SubaccountList(const identifier::Nym&, const Chain) const noexcept
        -> UnallocatedSet<identifier::Account> final
    {
        return {};
    }
    auto Unconfirm(
        const Key,
        const opentxs::blockchain::block::TransactionHash&,
        const Time time) const noexcept -> bool final
    {
        return {};
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
    const identifier::Nym id_;
    const identifier::Account account_;
};
}  // namespace opentxs::api::crypto::blank
