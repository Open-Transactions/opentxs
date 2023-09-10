// Copyright (c) 2023 MatterFi, Inc. - All Rights Reserved
// You may use, distribute, and modify this code under the terms of the
// MatterFi Semi-Open License accompanying this file.

#pragma once

#include <boost/container/flat_set.hpp>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace crypto
{
class PaymentCode;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Log;
class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace matterfi
{
auto paymentcode_extra_notifications(
    const opentxs::Log& log,
    const opentxs::blockchain::crypto::PaymentCode& account,
    boost::container::flat_set<opentxs::PaymentCode>& out) noexcept -> void;
auto paymentcode_preemptive_notifications(
    const opentxs::Log& log,
    const opentxs::api::Session& api,
    const opentxs::identifier::Nym& sender,
    opentxs::blockchain::Type chain,
    boost::container::flat_set<opentxs::PaymentCode>& out,
    opentxs::alloc::Strategy alloc) noexcept -> void;
}  // namespace matterfi
