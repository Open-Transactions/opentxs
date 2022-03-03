// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"
// IWYU pragma: no_include "opentxs/identity/wot/claim/ClaimType.hpp"
// IWYU pragma: no_include "opentxs/otx/client/PaymentWorkflowState.hpp"
// IWYU pragma: no_include "opentxs/otx/client/PaymentWorkflowType.hpp"

#include "opentxs/otx/client/Types.hpp"

#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"

namespace opentxs
{
auto translate(const otx::client::PaymentWorkflowState in) noexcept
    -> proto::PaymentWorkflowState;
auto translate(const otx::client::PaymentWorkflowType in) noexcept
    -> proto::PaymentWorkflowType;
auto translate(const proto::PaymentWorkflowState in) noexcept
    -> otx::client::PaymentWorkflowState;
auto translate(const proto::PaymentWorkflowType in) noexcept
    -> otx::client::PaymentWorkflowType;
}  // namespace opentxs
