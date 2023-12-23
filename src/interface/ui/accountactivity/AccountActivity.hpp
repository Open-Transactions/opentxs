// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <PaymentWorkflowEnums.pb.h>
#include <mutex>
#include <span>
#include <string_view>
#include <utility>

#include "core/Worker.hpp"
#include "interface/qt/SendMonitor.hpp"
#include "interface/ui/base/List.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/interface/ui/AccountActivity.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Account.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Polarity.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace ui
{
class AmountValidator;
class DestinationValidator;
class DisplayScaleQt;
}  // namespace ui

class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
template <typename T>
struct make_blank;

template <>
struct make_blank<ui::implementation::AccountActivityRowID> {
    static auto value(const api::Session& api)
        -> ui::implementation::AccountActivityRowID
    {
        return {identifier::Generic{}, proto::PAYMENTEVENTTYPE_ERROR};
    }
};
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using AccountActivityList = List<
    AccountActivityExternalInterface,
    AccountActivityInternalInterface,
    AccountActivityRowID,
    AccountActivityRowInterface,
    AccountActivityRowInternal,
    AccountActivityRowBlank,
    AccountActivitySortKey,
    AccountActivityPrimaryID>;

/** Show the list of Workflows applicable to this account

    Each row is a BalanceItem which is associated with a Workflow state.

    Some Workflows will only have one entry in the AccountActivity based on
    their type, but others may have multiple entries corresponding to different
    states.
 */
class AccountActivity : public AccountActivityList,
                        protected Worker<AccountActivity>
{
public:
    auto AccountID() const noexcept -> UnallocatedCString final
    {
        return account_id_.asBase58(api_.Crypto());
    }
    auto Balance() const noexcept -> const Amount final
    {
        const auto lock = sLock{shared_lock_};

        return balance_;
    }
    auto BalancePolarity() const noexcept -> int final
    {
        const auto lock = sLock{shared_lock_};

        return polarity(balance_);
    }
    auto ClearCallbacks() const noexcept -> void final;
    auto Contract() const noexcept -> const contract::Unit& final
    {
        return contract_.get();
    }
    auto DepositAddress() const noexcept -> UnallocatedCString override
    {
        return DepositAddress(blockchain::Type::UnknownBlockchain);
    }
    auto DepositAddress(const blockchain::Type) const noexcept
        -> UnallocatedCString override
    {
        return {};
    }
    auto DepositChains() const noexcept
        -> UnallocatedVector<blockchain::Type> override
    {
        return {};
    }
    auto DisplayBalance() const noexcept -> UnallocatedCString final
    {
        const auto lock = sLock{shared_lock_};

        return display_balance(balance_);
    }
    auto Notary() const noexcept -> const contract::Server& final
    {
        return notary_.get();
    }
    auto Notify([[maybe_unused]] std::span<const PaymentCode> contacts)
        const noexcept -> bool override
    {
        return false;
    }
    auto Notify(
        [[maybe_unused]] std::span<const PaymentCode> contacts,
        [[maybe_unused]] SendMonitor::Callback cb) const noexcept
        -> int override
    {
        return false;
    }
    using ui::AccountActivity::Send;
    auto Send(
        [[maybe_unused]] const identifier::Generic& contact,
        [[maybe_unused]] const Amount& amount,
        [[maybe_unused]] const std::string_view memo,
        [[maybe_unused]] std::span<const PaymentCode> notify) const noexcept
        -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const identifier::Generic& contact,
        [[maybe_unused]] const UnallocatedCString& amount,
        [[maybe_unused]] const std::string_view memo,
        [[maybe_unused]] Scale scale,
        [[maybe_unused]] std::span<const PaymentCode> notify) const noexcept
        -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const identifier::Generic& contact,
        [[maybe_unused]] const UnallocatedCString& amount,
        [[maybe_unused]] const std::string_view memo,
        [[maybe_unused]] Scale scale,
        [[maybe_unused]] SendMonitor::Callback cb,
        [[maybe_unused]] std::span<const PaymentCode> notify) const noexcept
        -> int override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const UnallocatedCString& address,
        [[maybe_unused]] const Amount& amount,
        [[maybe_unused]] const std::string_view memo,
        [[maybe_unused]] std::span<const PaymentCode> notify) const noexcept
        -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const UnallocatedCString& address,
        [[maybe_unused]] const UnallocatedCString& amount,
        [[maybe_unused]] const std::string_view memo,
        [[maybe_unused]] Scale scale,
        [[maybe_unused]] std::span<const PaymentCode> notify) const noexcept
        -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const UnallocatedCString& address,
        [[maybe_unused]] const UnallocatedCString& amount,
        [[maybe_unused]] const std::string_view memo,
        [[maybe_unused]] Scale scale,
        [[maybe_unused]] SendMonitor::Callback cb,
        [[maybe_unused]] std::span<const PaymentCode> notify) const noexcept
        -> int override
    {
        return false;
    }
    auto SendMonitor() const noexcept -> implementation::SendMonitor& final;
    auto SyncPercentage() const noexcept -> double override { return 100; }
    auto SyncProgress() const noexcept -> std::pair<int, int> override
    {
        return {1, 1};
    }
    auto Type() const noexcept -> AccountType final { return type_; }
    auto ValidateAddress([[maybe_unused]] const UnallocatedCString& text)
        const noexcept -> bool override
    {
        return false;
    }
    auto ValidateAmount([[maybe_unused]] const UnallocatedCString& text)
        const noexcept -> UnallocatedCString override
    {
        return {};
    }

    auto AmountValidator() noexcept -> ui::AmountValidator& final;
    auto DestinationValidator() noexcept -> ui::DestinationValidator& final;
    auto DisplayScaleQt() noexcept -> ui::DisplayScaleQt& final;
    auto SendMonitor() noexcept -> implementation::SendMonitor& final;
    auto SetCallbacks(Callbacks&& cb) noexcept -> void final;

    AccountActivity() = delete;
    AccountActivity(const AccountActivity&) = delete;
    AccountActivity(AccountActivity&&) = delete;
    auto operator=(const AccountActivity&) -> AccountActivity& = delete;
    auto operator=(AccountActivity&&) -> AccountActivity& = delete;

    ~AccountActivity() override;

protected:
    struct CallbackHolder {
        mutable std::mutex lock_{};
        Callbacks cb_{};
    };

    mutable CallbackHolder callbacks_;
    mutable Amount balance_;
    const identifier::Account account_id_;
    const AccountType type_;
    OTUnitDefinition contract_;
    OTServerContract notary_;

    virtual auto display_balance(opentxs::Amount value) const noexcept
        -> UnallocatedCString = 0;
    auto notify_balance(opentxs::Amount balance) const noexcept -> void;

    // NOTE only call in final class constructor bodies
    auto init(Endpoints endpoints) noexcept -> void;

    AccountActivity(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        const AccountType type,
        const SimpleCallback& cb) noexcept;

private:
    friend Worker<AccountActivity>;

    struct QT;

    QT* qt_;

    virtual auto startup() noexcept -> void = 0;

    auto construct_row(
        const AccountActivityRowID& id,
        const AccountActivitySortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto init_qt() noexcept -> void;
    virtual auto pipeline(const Message& in) noexcept -> void = 0;
    auto shutdown_qt() noexcept -> void;
};
}  // namespace opentxs::ui::implementation
