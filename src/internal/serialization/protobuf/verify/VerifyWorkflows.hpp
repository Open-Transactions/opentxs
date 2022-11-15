// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::EventTransportMethod

#pragma once

#include <PaymentWorkflowEnums.pb.h>
#include <cstdint>
#include <tuple>
#include <utility>

#include "internal/serialization/protobuf/Basic.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::proto
{
using PaymentWorkflowVersion = std::pair<std::uint32_t, PaymentWorkflowType>;
using WorkflowEventMap =
    UnallocatedMap<PaymentWorkflowVersion, UnallocatedSet<PaymentEventType>>;
using PaymentTypeVersion = std::pair<std::uint32_t, PaymentWorkflowType>;
using WorkflowStateMap =
    UnallocatedMap<PaymentTypeVersion, UnallocatedSet<PaymentWorkflowState>>;
using PaymentEventVersion = std::pair<std::uint32_t, PaymentEventType>;
using EventTransportMap =
    UnallocatedMap<PaymentEventVersion, UnallocatedSet<EventTransportMethod>>;

auto PaymentEventAllowedTransportMethod() noexcept -> const EventTransportMap&;
auto PaymentWorkflowAllowedEventTypes() noexcept -> const WorkflowEventMap&;
auto PaymentWorkflowAllowedInstrumentRevision() noexcept -> const VersionMap&;
auto PaymentWorkflowAllowedPaymentEvent() noexcept -> const VersionMap&;
auto PaymentWorkflowAllowedState() noexcept -> const WorkflowStateMap&;
}  // namespace opentxs::proto
