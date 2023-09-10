// Copyright (c) 2023 MatterFi, Inc. - All Rights Reserved
// You may use, distribute, and modify this code under the terms of the
// MatterFi Semi-Open License accompanying this file.

#include "matterfi/PaymentCode.hpp"  // IWYU pragma: associated

namespace matterfi
{
auto paymentcode_extra_notifications(
    const opentxs::Log&,
    const opentxs::blockchain::crypto::PaymentCode&,
    boost::container::flat_set<opentxs::PaymentCode>&) noexcept -> void
{
}

auto paymentcode_preemptive_notifications(
    const opentxs::Log&,
    const opentxs::api::Session&,
    const opentxs::identifier::Nym&,
    opentxs::blockchain::Type,
    boost::container::flat_set<opentxs::PaymentCode>&,
    opentxs::alloc::Strategy) noexcept -> void
{
}
}  // namespace matterfi
