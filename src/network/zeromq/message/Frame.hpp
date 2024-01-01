// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <zmq.h>
#include <cstddef>

#include "internal/network/zeromq/message/Frame.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/protobuf/Types.internal.hpp"

namespace opentxs::network::zeromq
{
class Frame::Imp final : public internal::Frame
{
public:
    auto operator<(const zeromq::Frame& rhs) const noexcept -> bool;
    auto operator==(const zeromq::Frame& rhs) const noexcept -> bool;

    auto Bytes() const noexcept -> ReadView
    {
        return ReadView{static_cast<const char*>(data()), size()};
    }
    auto data() const noexcept -> const void*
    {
        return ::zmq_msg_data(&message_);
    }
    auto size() const noexcept -> std::size_t
    {
        return ::zmq_msg_size(&message_);
    }

    operator zmq_msg_t*() noexcept { return &message_; }

    auto data() noexcept -> std::byte* final
    {
        return static_cast<std::byte*>(::zmq_msg_data(&message_));
    }

    mutable zmq_msg_t message_;

    Imp(const void* data, std::size_t size) noexcept;
    Imp(std::size_t size) noexcept;
    Imp() noexcept;
    Imp(const protobuf::MessageType& input) noexcept;
    Imp(const Imp&) noexcept;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() final;
};
}  // namespace opentxs::network::zeromq
