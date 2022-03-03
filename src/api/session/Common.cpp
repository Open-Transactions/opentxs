// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "internal/api/session/Types.hpp"  // IWYU pragma: associated

#include <robin_hood.h>

#include "opentxs/otx/client/PaymentWorkflowState.hpp"
#include "opentxs/otx/client/PaymentWorkflowType.hpp"
#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"
#include "util/Container.hpp"

namespace opentxs::api::session
{
using PaymentWorkflowStateMap = robin_hood::unordered_flat_map<
    otx::client::PaymentWorkflowState,
    proto::PaymentWorkflowState>;
using PaymentWorkflowStateReverseMap = robin_hood::unordered_flat_map<
    proto::PaymentWorkflowState,
    otx::client::PaymentWorkflowState>;
using PaymentWorkflowTypeMap = robin_hood::unordered_flat_map<
    otx::client::PaymentWorkflowType,
    proto::PaymentWorkflowType>;
using PaymentWorkflowTypeReverseMap = robin_hood::unordered_flat_map<
    proto::PaymentWorkflowType,
    otx::client::PaymentWorkflowType>;

auto paymentworkflowstate_map() noexcept -> const PaymentWorkflowStateMap&;
auto paymentworkflowtype_map() noexcept -> const PaymentWorkflowTypeMap&;
}  // namespace opentxs::api::session

namespace opentxs::api::session
{
using PaymentWorkflowState = otx::client::PaymentWorkflowState;
using PaymentWorkflowType = otx::client::PaymentWorkflowType;

auto paymentworkflowstate_map() noexcept -> const PaymentWorkflowStateMap&
{
    static const auto map = PaymentWorkflowStateMap{
        {PaymentWorkflowState::Error, proto::PAYMENTWORKFLOWSTATE_ERROR},
        {PaymentWorkflowState::Unsent, proto::PAYMENTWORKFLOWSTATE_UNSENT},
        {PaymentWorkflowState::Conveyed, proto::PAYMENTWORKFLOWSTATE_CONVEYED},
        {PaymentWorkflowState::Cancelled,
         proto::PAYMENTWORKFLOWSTATE_CANCELLED},
        {PaymentWorkflowState::Accepted, proto::PAYMENTWORKFLOWSTATE_ACCEPTED},
        {PaymentWorkflowState::Completed,
         proto::PAYMENTWORKFLOWSTATE_COMPLETED},
        {PaymentWorkflowState::Expired, proto::PAYMENTWORKFLOWSTATE_EXPIRED},
        {PaymentWorkflowState::Initiated,
         proto::PAYMENTWORKFLOWSTATE_INITIATED},
        {PaymentWorkflowState::Aborted, proto::PAYMENTWORKFLOWSTATE_ABORTED},
        {PaymentWorkflowState::Acknowledged,
         proto::PAYMENTWORKFLOWSTATE_ACKNOWLEDGED},
        {PaymentWorkflowState::Rejected, proto::PAYMENTWORKFLOWSTATE_REJECTED},
    };

    return map;
}

auto paymentworkflowtype_map() noexcept -> const PaymentWorkflowTypeMap&
{
    static const auto map = PaymentWorkflowTypeMap{
        {PaymentWorkflowType::Error, proto::PAYMENTWORKFLOWTYPE_ERROR},
        {PaymentWorkflowType::OutgoingCheque,
         proto::PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE},
        {PaymentWorkflowType::IncomingCheque,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE},
        {PaymentWorkflowType::OutgoingInvoice,
         proto::PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE},
        {PaymentWorkflowType::IncomingInvoice,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGINVOICE},
        {PaymentWorkflowType::OutgoingTransfer,
         proto::PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER},
        {PaymentWorkflowType::IncomingTransfer,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER},
        {PaymentWorkflowType::InternalTransfer,
         proto::PAYMENTWORKFLOWTYPE_INTERNALTRANSFER},
        {PaymentWorkflowType::OutgoingCash,
         proto::PAYMENTWORKFLOWTYPE_OUTGOINGCASH},
        {PaymentWorkflowType::IncomingCash,
         proto::PAYMENTWORKFLOWTYPE_INCOMINGCASH},
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
    static const auto map = reverse_arbitrary_map<
        otx::client::PaymentWorkflowState,
        proto::PaymentWorkflowState,
        api::session::PaymentWorkflowStateReverseMap>(
        api::session::paymentworkflowstate_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::client::PaymentWorkflowState::Error;
    }
}

auto translate(const proto::PaymentWorkflowType in) noexcept
    -> otx::client::PaymentWorkflowType
{
    static const auto map = reverse_arbitrary_map<
        otx::client::PaymentWorkflowType,
        proto::PaymentWorkflowType,
        api::session::PaymentWorkflowTypeReverseMap>(
        api::session::paymentworkflowtype_map());

    try {
        return map.at(in);
    } catch (...) {
        return otx::client::PaymentWorkflowType::Error;
    }
}
}  // namespace opentxs
