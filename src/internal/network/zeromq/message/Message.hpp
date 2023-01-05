// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"

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

namespace opentxs::network::zeromq::internal
{
class Message
{
public:
    virtual auto Envelope() const noexcept
        -> std::span<const zeromq::Frame> = 0;

    virtual auto AddFrame(const ProtobufType& input) noexcept
        -> zeromq::Frame& = 0;
    virtual auto EnsureDelimiter() noexcept -> void = 0;
    virtual auto Envelope() noexcept -> std::span<zeromq::Frame> = 0;
    virtual auto ExtractFront() noexcept -> zeromq::Frame = 0;
    virtual auto Prepend(SocketID id) noexcept -> zeromq::Frame& = 0;

    virtual ~Message() = default;
};
}  // namespace opentxs::network::zeromq::internal
