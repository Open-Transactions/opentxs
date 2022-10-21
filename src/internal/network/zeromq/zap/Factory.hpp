// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/zap/ZAP.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Reply;
class Request;
}  // namespace zap
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto ZAPRequest(
    const ReadView address,
    const ReadView domain,
    const network::zeromq::zap::Mechanism mechanism,
    const ReadView requestID,
    const ReadView identity,
    const ReadView version) noexcept -> network::zeromq::zap::Request;
auto ZAPReply(
    const network::zeromq::zap::Request& request,
    const network::zeromq::zap::Status code,
    const ReadView status) noexcept -> network::zeromq::zap::Reply;
auto ZAPReply(
    const network::zeromq::zap::Request& request,
    const network::zeromq::zap::Status code,
    const ReadView status,
    const ReadView userID,
    const ReadView metadata,
    const ReadView version) noexcept -> network::zeromq::zap::Reply;
}  // namespace opentxs::factory
