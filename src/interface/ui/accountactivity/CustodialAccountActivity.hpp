// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::PaymentEventType

#pragma once

#include <PaymentWorkflowEnums.pb.h>
#include <utility>

#include "interface/ui/accountactivity/AccountActivity.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace proto
{
class PaymentEvent;
class PaymentWorkflow;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class CustodialAccountActivity final : public AccountActivity
{
public:
    auto API() const noexcept -> const api::Session& final { return api_; }
    auto ContractID() const noexcept -> UnallocatedCString final;
    auto DisplayUnit() const noexcept -> UnallocatedCString final;
    auto Name() const noexcept -> UnallocatedCString final;
    auto NotaryID() const noexcept -> UnallocatedCString final;
    auto NotaryName() const noexcept -> UnallocatedCString final;
    auto Unit() const noexcept -> UnitType final;

    CustodialAccountActivity(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        const SimpleCallback& cb) noexcept;
    CustodialAccountActivity() = delete;
    CustodialAccountActivity(const CustodialAccountActivity&) = delete;
    CustodialAccountActivity(CustodialAccountActivity&&) = delete;
    auto operator=(const CustodialAccountActivity&)
        -> CustodialAccountActivity& = delete;
    auto operator=(CustodialAccountActivity&&)
        -> CustodialAccountActivity& = delete;

    ~CustodialAccountActivity() final;

private:
    using EventRow =
        std::pair<AccountActivitySortKey, const proto::PaymentEvent*>;
    using RowKey = std::pair<proto::PaymentEventType, EventRow>;

    enum class Work : OTZMQWorkType {
        notary = value(WorkType::NotaryUpdated),
        unit = value(WorkType::UnitDefinitionUpdated),
        contact = value(WorkType::ContactUpdated),
        account = value(WorkType::AccountUpdated),
        workflow = value(WorkType::WorkflowAccountUpdate),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        shutdown = value(WorkType::Shutdown),
    };

    UnallocatedCString alias_;

    static auto extract_event(
        const proto::PaymentEventType event,
        const proto::PaymentWorkflow& workflow) noexcept -> EventRow;
    static auto extract_rows(const proto::PaymentWorkflow& workflow) noexcept
        -> UnallocatedVector<RowKey>;

    auto display_balance(opentxs::Amount value) const noexcept
        -> UnallocatedCString final;

    auto pipeline(const Message& in) noexcept -> void final;
    auto process_balance(const Message& message) noexcept -> void;
    auto process_contact(const Message& message) noexcept -> void;
    auto process_notary(const Message& message) noexcept -> void;
    auto process_workflow(
        const identifier::Generic& workflowID,
        UnallocatedSet<AccountActivityRowID>& active) noexcept -> void;
    auto process_workflow(const Message& message) noexcept -> void;
    auto process_unit(const Message& message) noexcept -> void;
    auto startup() noexcept -> void final;
};
}  // namespace opentxs::ui::implementation
