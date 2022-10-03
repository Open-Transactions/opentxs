// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>

#include "internal/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class FrameIterator::Imp final : public internal::FrameIterator
{
public:
    Message* parent_;
    std::atomic<std::size_t> position_;

    auto operator<(const zeromq::FrameIterator& rhs) const noexcept(false)
        -> bool;
    auto operator==(const zeromq::FrameIterator& rhs) const noexcept -> bool;
    auto hash() const noexcept -> std::size_t final;

    Imp() noexcept;
    Imp(Message* parent, std::size_t position = 0) noexcept;
    Imp(const Imp&) noexcept;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&) -> Imp& = delete;

    ~Imp() final;
};
}  // namespace opentxs::network::zeromq
