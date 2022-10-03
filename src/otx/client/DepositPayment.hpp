// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "core/StateMachine.hpp"
#include "internal/otx/client/Client.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/otx/client/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class UnitDefinition;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::client::implementation
{
class PaymentTasks;

class DepositPayment final : public opentxs::internal::StateMachine
{
public:
    using TaskID = api::session::OTX::TaskID;

    DepositPayment(
        client::internal::StateMachine& parent,
        const TaskID taskID,
        const DepositPaymentTask& payment,
        PaymentTasks& paymenttasks);
    DepositPayment() = delete;

    ~DepositPayment() final;

private:
    client::internal::StateMachine& parent_;
    const TaskID task_id_;
    DepositPaymentTask payment_;
    Depositability state_;
    api::session::OTX::Result result_;
    PaymentTasks& payment_tasks_;

    auto deposit() -> bool;
    auto get_account_id(const identifier::UnitDefinition& unit)
        -> identifier::Generic;
};
}  // namespace opentxs::otx::client::implementation
