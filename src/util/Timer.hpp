// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/util/Timer.hpp"

#include <chrono>
#include <cstddef>

#include "opentxs/util/Time.hpp"

namespace boost
{
namespace asio
{
class io_context;
}  // namespace asio

namespace system
{
class error_code;
}  // namespace system
}  // namespace boost

namespace opentxs
{
class Timer::Imp
{
public:
    virtual auto IsActive() const noexcept -> bool;

    virtual auto Cancel() noexcept -> std::size_t;
    virtual auto SetAbsolute(const Time&) noexcept -> std::size_t;
    virtual auto SetRelative(const std::chrono::microseconds&) noexcept
        -> std::size_t;
    virtual auto Wait(Handler&&) noexcept -> void;
    virtual auto Wait() noexcept -> void;

    virtual ~Imp() = default;
};
}  // namespace opentxs
