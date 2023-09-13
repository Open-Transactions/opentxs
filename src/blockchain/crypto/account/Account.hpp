// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Data.hpp"

#pragma once

#include "blockchain/crypto/account/NodeGroup.hpp"
#include "blockchain/crypto/account/NodeIndex.hpp"
#include "internal/blockchain/crypto/Account.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Imported.hpp"
#include "opentxs/blockchain/crypto/Notification.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
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

namespace proto
{
class HDPath;
}  // namespace proto

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
    auto Chain() const noexcept -> opentxs::blockchain::Type final
    {
        return chain_;
    }
    [[nodiscard]] auto ClaimAccountID(
        const identifier::Account& id,
        bool existing,
        crypto::Subaccount* node) const noexcept -> bool final;
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
    auto GetHD() const noexcept -> const HDAccounts& final { return hd_; }
    auto GetImported() const noexcept -> const ImportedAccounts& final
    {
        return imported_;
    }
    auto GetNotification() const noexcept -> const NotificationAccounts& final
    {
        return notification_;
    }
    auto GetNextChangeKey(const PasswordPrompt& reason) const noexcept(false)
        -> const crypto::Element& final;
    auto GetNextDepositKey(const PasswordPrompt& reason) const noexcept(false)
        -> const crypto::Element& final;
    auto GetPaymentCode() const noexcept -> const PaymentCodeAccounts& final
    {
        return payment_code_;
    }
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
        -> const crypto::Subaccount& final;

    auto AddHDNode(
        const proto::HDPath& path,
        const crypto::HDProtocol standard,
        const PasswordPrompt& reason,
        identifier::Account& id) noexcept -> bool final;
    auto AddUpdatePaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const PasswordPrompt& reason,
        identifier::Account& out) noexcept -> bool final
    {
        return payment_code_.Construct(
            out, contacts_, local, remote, path, reason);
    }
    auto AddUpdatePaymentCode(
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const opentxs::blockchain::block::TransactionHash& txid,
        const PasswordPrompt& reason,
        identifier::Account& out) noexcept -> bool final
    {
        return payment_code_.Construct(
            out, contacts_, local, remote, path, txid, reason);
    }
    auto Startup() noexcept -> void final { init_notification(); }
    auto Subaccount(const identifier::Account& id) noexcept(false)
        -> crypto::Subaccount& final;

    Account(
        const api::Session& api,
        const api::session::Contacts& contacts,
        const crypto::Wallet& parent,
        const identifier::Nym& nym,
        const Accounts& hd,
        const Accounts& imported,
        const Accounts& paymentCode) noexcept;
    Account() = delete;
    Account(const Account&) = delete;
    Account(Account&&) = delete;
    auto operator=(const Account&) -> Account& = delete;
    auto operator=(Account&&) -> Account& = delete;

    ~Account() final = default;

private:
    using HDNodes = account::NodeGroup<HDAccounts, crypto::HD>;
    using ImportedNodes =
        account::NodeGroup<ImportedAccounts, crypto::Imported>;
    using NotificationNodes =
        account::NodeGroup<NotificationAccounts, crypto::Notification>;
    using PaymentCodeNodes =
        account::NodeGroup<PaymentCodeAccounts, crypto::PaymentCode>;

    const api::Session& api_;
    const api::session::Contacts& contacts_;
    const crypto::Wallet& parent_;
    const opentxs::blockchain::Type chain_;
    const identifier::Nym nym_id_;
    const identifier::Account account_id_;
    HDNodes hd_;
    ImportedNodes imported_;
    NotificationNodes notification_;
    PaymentCodeNodes payment_code_;
    mutable account::NodeIndex node_index_;
    OTZMQPushSocket find_nym_;

    auto init_hd(const Accounts& HDAccounts) noexcept -> void;
    auto init_notification() noexcept -> void;
    auto init_payment_code(const Accounts& HDAccounts) noexcept -> void;

    auto find_next_element(
        Subchain subchain,
        const identifier::Generic& contact,
        const UnallocatedCString& label,
        const PasswordPrompt& reason) const noexcept(false)
        -> const crypto::Element&;
};
}  // namespace opentxs::blockchain::crypto::implementation
