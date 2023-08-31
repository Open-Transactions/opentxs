// Copyright (c) 2023 MatterFi, Inc. - All Rights Reserved
// You may use, distribute, and modify this code under the terms of the
// MatterFi Semi-Open License accompanying this file.

#include "matterfi/PaymentCode.hpp"  // IWYU pragma: associated

#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/core/PaymentCode.hpp"

// This functions in this file implement operations covered by one or more
// claims of US Provisional Patent Application 63/507,085 and 63/512,052
// assigned to MatterFi, Inc.
namespace matterfi
{
using namespace opentxs::literals;

auto paymentcode_extra_notifications(
    const opentxs::blockchain::crypto::PaymentCode& account,
    opentxs::Set<opentxs::PaymentCode>& out) noexcept -> void
{
    if (0_uz == account.IncomingNotificationCount()) {
        out.emplace(account.Local());
    }
}
}  // namespace matterfi
