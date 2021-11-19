// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Proto.hpp"
#include "internal/network/zeromq/Types.hpp"

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

namespace opentxs::network::zeromq::internal
{
class Message
{
public:
    virtual auto AddFrame(const ProtobufType& input) noexcept
        -> zeromq::Frame& = 0;
    virtual auto ExtractFront() noexcept -> zeromq::Frame = 0;
    virtual auto Prepend(SocketID id) noexcept -> zeromq::Frame& = 0;

    virtual ~Message() = default;
};
}  // namespace opentxs::network::zeromq::internal