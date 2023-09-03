// Copyright (c) 2023 MatterFi, Inc. - All Rights Reserved
// You may use, distribute, and modify this code under the terms of the
// MatterFi Semi-Open License accompanying this file.

#include "matterfi/PaymentCode.hpp"  // IWYU pragma: associated

namespace matterfi
{
auto paymentcode_extra_notifications(
    const opentxs::Log&,
    const opentxs::blockchain::crypto::PaymentCode& account,
    opentxs::Set<opentxs::PaymentCode>& out) noexcept -> void
{
}
}  // namespace matterfi
