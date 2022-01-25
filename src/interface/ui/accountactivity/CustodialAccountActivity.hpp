// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <atomic>
#include <utility>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "interface/ui/accountactivity/AccountActivity.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/interface/ui/AccountActivity.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
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

class Identifier;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
class CustodialAccountActivity final : public AccountActivity
{
public:
    auto ContractID() const noexcept -> UnallocatedCString final;
    auto DisplayUnit() const noexcept -> UnallocatedCString final;
    auto Name() const noexcept -> UnallocatedCString final;
    auto NotaryID() const noexcept -> UnallocatedCString final;
    auto NotaryName() const noexcept -> UnallocatedCString final;
    auto Unit() const noexcept -> UnitType final;

    CustodialAccountActivity(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const SimpleCallback& cb) noexcept;

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
        const Identifier& workflowID,
        UnallocatedSet<AccountActivityRowID>& active) noexcept -> void;
    auto process_workflow(const Message& message) noexcept -> void;
    auto process_unit(const Message& message) noexcept -> void;
    auto startup() noexcept -> void final;

    CustodialAccountActivity() = delete;
    CustodialAccountActivity(const CustodialAccountActivity&) = delete;
    CustodialAccountActivity(CustodialAccountActivity&&) = delete;
    auto operator=(const CustodialAccountActivity&)
        -> CustodialAccountActivity& = delete;
    auto operator=(CustodialAccountActivity&&)
        -> CustodialAccountActivity& = delete;
};
}  // namespace opentxs::ui::implementation
