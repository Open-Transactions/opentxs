// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Data.hpp"

#pragma once

#include <cs_shared_guarded.h>
#include <memory>
#include <shared_mutex>

#include "internal/blockchain/crypto/Account.hpp"
#include "internal/blockchain/crypto/Subaccount.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Contacts;
}  // namespace session

class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
struct Notifications;
}  // namespace crypto
}  // namespace blockchain

namespace protobuf
{
class HDPath;
}  // namespace protobuf

class PasswordPrompt;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::crypto::implementation
{
class Account final : public internal::Account
{
public:
    using Accounts = UnallocatedSet<identifier::Account>;

    auto AccountID() const noexcept -> const identifier::Account& final
    {
        return account_id_;
    }
    auto FindNym(const identifier::Nym& id) const noexcept -> void final;
    auto GetDepositAddress(
        const blockchain::crypto::AddressStyle style,
        const PasswordPrompt& reason,
        const UnallocatedCString& memo) const noexcept
        -> UnallocatedCString final;
    auto GetDepositAddress(
        const blockchain::crypto::AddressStyle style,
        const identifier::Generic& contact,
        const PasswordPrompt& reason,
        const UnallocatedCString& memo) const noexcept
        -> UnallocatedCString final;
    auto Get(Notifications& out) const noexcept -> void final;
    auto GetNextChangeKey(const PasswordPrompt& reason) const noexcept(false)
        -> const crypto::Element& final;
    auto GetNextDepositKey(const PasswordPrompt& reason) const noexcept(false)
        -> const crypto::Element& final;
    auto GetSubaccounts() const noexcept
        -> UnallocatedVector<crypto::Subaccount> final;
    auto GetSubaccounts(SubaccountType type) const noexcept
        -> UnallocatedVector<crypto::Subaccount> final;
    auto Internal() const noexcept -> Account& final
    {
        return const_cast<Account&>(*this);
    }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return nym_id_;
    }
    auto Parent() const noexcept -> const crypto::Wallet& final
    {
        return parent_;
    }
    auto Subaccount(const identifier::Account& id) const noexcept(false)
        -> crypto::Subaccount& final;
    auto Target() const noexcept -> crypto::Target final { return chain_; }

    auto AddEthereum(
        const protobuf::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason) noexcept -> crypto::Subaccount& final;
    auto AddHD(
        const protobuf::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason) noexcept -> crypto::Subaccount& final;
    auto AddPaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const protobuf::HDPath& path,
        const PasswordPrompt& reason) noexcept -> crypto::Subaccount& final;
    auto Startup() noexcept -> void final {}
    auto Subaccount(const identifier::Account& id) noexcept(false)
        -> crypto::Subaccount& final;

    Account(
        const api::Session& api,
        const api::session::Contacts& contacts,
        const crypto::Wallet& parent,
        const identifier::Nym& nym,
        const Accounts& hd,
        const Accounts& ethereum,
        const Accounts& paymentCode) noexcept;
    Account() = delete;
    Account(const Account&) = delete;
    Account(Account&&) = delete;
    auto operator=(const Account&) -> Account& = delete;
    auto operator=(Account&&) -> Account& = delete;

    ~Account() final = default;

private:
    struct Data {
        using Pointer = std::shared_ptr<internal::Subaccount>;
        using Map = UnorderedMultimap<SubaccountType, Pointer>;
        using Index = UnorderedMap<identifier::Account, Pointer>;

        Map map_{};
        Index index_{};
    };

    using Subaccounts = libguarded::shared_guarded<Data, std::shared_mutex>;

    const api::Session& api_;
    const api::session::Contacts& contacts_;
    const crypto::Wallet& parent_;
    const opentxs::blockchain::Type chain_;
    const identifier::Nym nym_id_;
    const identifier::Account account_id_;
    mutable Subaccounts subaccounts_;
    OTZMQPushSocket find_nym_;

    auto find_next_element(
        Subchain subchain,
        const identifier::Generic& contact,
        const UnallocatedCString& label,
        const PasswordPrompt& reason) const noexcept(false)
        -> const crypto::Element&;
    auto get_subaccounts(Data& map) const noexcept
        -> UnallocatedVector<crypto::Subaccount>;
    auto get_subaccounts(SubaccountType type, Data& map) const noexcept
        -> UnallocatedVector<crypto::Subaccount>;

    auto init_ethereum(const Accounts& accounts, Data& data) noexcept -> void;
    auto init_ethereum(
        const identifier::Account& id,
        Data& data,
        bool checking) noexcept -> crypto::Subaccount&;
    auto init_ethereum(
        const protobuf::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason) noexcept -> crypto::Subaccount&;
    auto init_hd(const Accounts& HDAccounts, Data& data) noexcept -> void;
    auto init_hd(
        const identifier::Account& id,
        Data& data,
        bool checking) noexcept -> crypto::Subaccount&;
    auto init_hd(
        const protobuf::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason) noexcept -> crypto::Subaccount&;
    auto init_notification(Data& data) noexcept -> void;
    auto init_payment_code(const Accounts& HDAccounts, Data& data) noexcept
        -> void;
    auto init_payment_code(
        const identifier::Account& id,
        Data& data,
        bool checking) noexcept -> crypto::Subaccount&;
    auto init_payment_code(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const protobuf::HDPath& path,
        const PasswordPrompt& reason) noexcept -> crypto::Subaccount&;
};
}  // namespace opentxs::blockchain::crypto::implementation
