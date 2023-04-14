// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/api/session/Types.hpp"  // IWYU pragma: associated

#include <PaymentWorkflowEnums.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <functional>

#include "opentxs/otx/client/PaymentWorkflowState.hpp"  // IWYU pragma: keep
#include "opentxs/otx/client/PaymentWorkflowType.hpp"   // IWYU pragma: keep
#include "opentxs/otx/client/Types.hpp"

namespace opentxs::api::session
{
using PaymentWorkflowStateMap = frozen::unordered_map<
    otx::client::PaymentWorkflowState,
    proto::PaymentWorkflowState,
    11>;
using PaymentWorkflowStateReverseMap = frozen::unordered_map<
    proto::PaymentWorkflowState,
    otx::client::PaymentWorkflowState,
    11>;
using PaymentWorkflowTypeMap = frozen::unordered_map<
    otx::client::PaymentWorkflowType,
    proto::PaymentWorkflowType,
    10>;
using PaymentWorkflowTypeReverseMap = frozen::unordered_map<
    proto::PaymentWorkflowType,
    otx::client::PaymentWorkflowType,
    10>;

auto paymentworkflowstate_map() noexcept -> const PaymentWorkflowStateMap&;
auto paymentworkflowtype_map() noexcept -> const PaymentWorkflowTypeMap&;
}  // namespace opentxs::api::session

namespace opentxs::api::session
{
auto paymentworkflowstate_map() noexcept -> const PaymentWorkflowStateMap&
{
    using enum otx::client::PaymentWorkflowState;
    using enum proto::PaymentWorkflowState;
    static constexpr auto map = PaymentWorkflowStateMap{
        {Error, PAYMENTWORKFLOWSTATE_ERROR},
        {Unsent, PAYMENTWORKFLOWSTATE_UNSENT},
        {Conveyed, PAYMENTWORKFLOWSTATE_CONVEYED},
        {Cancelled, PAYMENTWORKFLOWSTATE_CANCELLED},
        {Accepted, PAYMENTWORKFLOWSTATE_ACCEPTED},
        {Completed, PAYMENTWORKFLOWSTATE_COMPLETED},
        {Expired, PAYMENTWORKFLOWSTATE_EXPIRED},
        {Initiated, PAYMENTWORKFLOWSTATE_INITIATED},
        {Aborted, PAYMENTWORKFLOWSTATE_ABORTED},
        {Acknowledged, PAYMENTWORKFLOWSTATE_ACKNOWLEDGED},
        {Rejected, PAYMENTWORKFLOWSTATE_REJECTED},
    };

    return map;
}

auto paymentworkflowtype_map() noexcept -> const PaymentWorkflowTypeMap&
{
    using enum otx::client::PaymentWorkflowType;
    using enum proto::PaymentWorkflowType;
    static constexpr auto map = PaymentWorkflowTypeMap{
        {Error, PAYMENTWORKFLOWTYPE_ERROR},
        {OutgoingCheque, PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE},
        {IncomingCheque, PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE},
        {OutgoingInvoice, PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE},
        {IncomingInvoice, PAYMENTWORKFLOWTYPE_INCOMINGINVOICE},
        {OutgoingTransfer, PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER},
        {IncomingTransfer, PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER},
        {InternalTransfer, PAYMENTWORKFLOWTYPE_INTERNALTRANSFER},
        {OutgoingCash, PAYMENTWORKFLOWTYPE_OUTGOINGCASH},
        {IncomingCash, PAYMENTWORKFLOWTYPE_INCOMINGCASH},
    };

    return map;
}
}  // namespace opentxs::api::session

namespace opentxs
{
auto translate(const otx::client::PaymentWorkflowState in) noexcept
    -> proto::PaymentWorkflowState
{
    try {
        return api::session::paymentworkflowstate_map().at(in);
    } catch (...) {
        return proto::PAYMENTWORKFLOWSTATE_ERROR;
    }
}

auto translate(const otx::client::PaymentWorkflowType in) noexcept
    -> proto::PaymentWorkflowType
{
    try {
        return api::session::paymentworkflowtype_map().at(in);
    } catch (...) {
        return proto::PAYMENTWORKFLOWTYPE_ERROR;
    }
}

auto translate(const proto::PaymentWorkflowState in) noexcept
    -> otx::client::PaymentWorkflowState
{
    static const auto map =
        frozen::invert_unordered_map(api::session::paymentworkflowstate_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::client::PaymentWorkflowState::Error;
    }
}

auto translate(const proto::PaymentWorkflowType in) noexcept
    -> otx::client::PaymentWorkflowType
{
    static const auto map =
        frozen::invert_unordered_map(api::session::paymentworkflowtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::client::PaymentWorkflowType::Error;
    }
}
}  // namespace opentxs
