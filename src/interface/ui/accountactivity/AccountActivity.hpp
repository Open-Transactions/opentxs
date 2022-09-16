// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <PaymentWorkflowEnums.pb.h>
#include <atomic>
#include <functional>
#include <mutex>
#include <utility>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "core/Worker.hpp"
#include "interface/qt/SendMonitor.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/interface/ui/AccountActivity.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Polarity.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;

namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class PaymentEvent;
class PaymentWorkflow;
}  // namespace proto

namespace ui
{
class AmountValidator;
class DestinationValidator;
class DisplayScaleQt;
}  // namespace ui
// }  // namespace v1
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
        sLock lock(shared_lock_);

        return balance_;
    }
    auto BalancePolarity() const noexcept -> int final
    {
        sLock lock(shared_lock_);

        return polarity(balance_);
    }
    auto ClearCallbacks() const noexcept -> void final;
    auto Contract() const noexcept -> const contract::Unit& final
    {
        return contract_.get();
    }
    auto DepositAddress() const noexcept -> UnallocatedCString override
    {
        return DepositAddress(blockchain::Type::Unknown);
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
        sLock lock(shared_lock_);

        return display_balance(balance_);
    }
    auto Notary() const noexcept -> const contract::Server& final
    {
        return notary_.get();
    }
    using ui::AccountActivity::Send;
    auto Send(
        [[maybe_unused]] const identifier::Generic& contact,
        [[maybe_unused]] const Amount& amount,
        [[maybe_unused]] const UnallocatedCString& memo) const noexcept
        -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const identifier::Generic& contact,
        [[maybe_unused]] const UnallocatedCString& amount,
        [[maybe_unused]] const UnallocatedCString& memo,
        [[maybe_unused]] Scale scale) const noexcept -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const identifier::Generic& contact,
        [[maybe_unused]] const UnallocatedCString& amount,
        [[maybe_unused]] const UnallocatedCString& memo,
        [[maybe_unused]] Scale scale,
        [[maybe_unused]] SendMonitor::Callback cb) const noexcept
        -> int override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const UnallocatedCString& address,
        [[maybe_unused]] const Amount& amount,
        [[maybe_unused]] const UnallocatedCString& memo) const noexcept
        -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const UnallocatedCString& address,
        [[maybe_unused]] const UnallocatedCString& amount,
        [[maybe_unused]] const UnallocatedCString& memo,
        [[maybe_unused]] Scale scale) const noexcept -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const UnallocatedCString& address,
        [[maybe_unused]] const UnallocatedCString& amount,
        [[maybe_unused]] const UnallocatedCString& memo,
        [[maybe_unused]] Scale scale,
        [[maybe_unused]] SendMonitor::Callback cb) const noexcept
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
    const identifier::Generic account_id_;
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
        const identifier::Generic& accountID,
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
