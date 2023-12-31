// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::PaymentWorkflowState

#include "opentxs/otx/client/Types.hpp"

#include <opentxs/protobuf/PaymentWorkflowEnums.pb.h>

namespace opentxs
{
auto translate(const otx::client::PaymentWorkflowState in) noexcept
    -> protobuf::PaymentWorkflowState;
auto translate(const otx::client::PaymentWorkflowType in) noexcept
    -> protobuf::PaymentWorkflowType;
auto translate(const protobuf::PaymentWorkflowState in) noexcept
    -> otx::client::PaymentWorkflowState;
auto translate(const protobuf::PaymentWorkflowType in) noexcept
    -> otx::client::PaymentWorkflowType;
}  // namespace opentxs
