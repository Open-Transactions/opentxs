// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Proto.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto ZMQFrame(std::size_t size) noexcept -> network::zeromq::Frame;
auto ZMQFrame(const void* data, const std::size_t size) noexcept
    -> network::zeromq::Frame;
auto ZMQFrame(const ProtobufType& data) noexcept -> network::zeromq::Frame;
}  // namespace opentxs::factory
