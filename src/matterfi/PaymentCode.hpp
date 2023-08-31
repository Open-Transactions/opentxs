// Copyright (c) 2023 MatterFi, Inc. - All Rights Reserved
// You may use, distribute, and modify this code under the terms of the
// MatterFi Semi-Open License accompanying this file.

#pragma once

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class PaymentCode;
}  // namespace crypto
}  // namespace blockchain

class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace matterfi
{
auto paymentcode_extra_notifications(
    const opentxs::blockchain::crypto::PaymentCode& account,
    opentxs::Set<opentxs::PaymentCode>& out) noexcept -> void;
}  // namespace matterfi
