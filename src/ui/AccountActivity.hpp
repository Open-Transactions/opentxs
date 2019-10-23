// Copyright (c) 2010-2019 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/ui/AccountActivity.hpp"

#include "internal/ui/UI.hpp"
#include "List.hpp"

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
class AccountActivity final : public AccountActivityList
{
public:
#if OT_QT
    // Data method needs this. And it needs to match the parent type for the Qt
    using qt_super =
        AccountActivityList::qt_super;  // class that appears below this one.
#endif

    const Identifier& AccountID() const noexcept final
    {
        return account_id_.get();
    }
    int BalancePolarity() const noexcept final
    {
        return polarity(balance_.load());
    }
    Amount Balance() const noexcept final { return balance_.load(); }
#if OT_QT
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
        noexcept final;
#endif
    std::string DisplayBalance() const noexcept final;
    ~AccountActivity();

private:
    friend api::client::implementation::UI;

    using EventRow =
        std::pair<AccountActivitySortKey, const proto::PaymentEvent*>;
    using RowKey = std::pair<proto::PaymentEventType, EventRow>;

    const ListenerDefinitions listeners_;
    mutable std::atomic<Amount> balance_{0};
    const OTIdentifier account_id_;
    std::shared_ptr<const UnitDefinition> contract_{nullptr};

    static EventRow extract_event(
        const proto::PaymentEventType event,
        const proto::PaymentWorkflow& workflow) noexcept;
    static std::vector<RowKey> extract_rows(
        const proto::PaymentWorkflow& workflow) noexcept;

    void construct_row(
        const AccountActivityRowID& id,
        const AccountActivitySortKey& index,
        const CustomData& custom) const noexcept final;

    void process_balance(const network::zeromq::Message& message) noexcept;
    void process_workflow(
        const Identifier& workflowID,
        std::set<AccountActivityRowID>& active) noexcept;
    void process_workflow(const network::zeromq::Message& message) noexcept;
    void startup() noexcept;

    AccountActivity(
        const api::client::internal::Manager& api,
        const network::zeromq::socket::Publish& publisher,
        const identifier::Nym& nymID,
        const Identifier& accountID
#if OT_QT
        ,
        const bool qt,
        const RowCallbacks insertCallback,
        const RowCallbacks removeCallback
#endif
        ) noexcept;
    AccountActivity() = delete;
    AccountActivity(const AccountActivity&) = delete;
    AccountActivity(AccountActivity&&) = delete;
    AccountActivity& operator=(const AccountActivity&) = delete;
    AccountActivity& operator=(AccountActivity&&) = delete;
};
}  // namespace opentxs::ui::implementation
