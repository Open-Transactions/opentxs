// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <functional>
#include <memory>

#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace system
{
class error_code;
}  // namespace system
}  // namespace boost

namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace network
{
namespace asio
{
class Context;
}  // namespace asio
}  // namespace network
}  // namespace api

class Timer;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
auto operator<(const Timer& lhs, const Timer& rhs) noexcept -> bool;
auto operator==(const Timer& lhs, const Timer& rhs) noexcept -> bool;
auto swap(Timer& lhs, Timer& rhs) noexcept -> void;

class Timer
{
public:
    class Imp;

    auto operator<(const Timer& rhs) const noexcept -> bool;
    auto operator==(const Timer& rhs) const noexcept -> bool;

    auto Cancel() noexcept -> std::size_t;
    auto swap(Timer& rhs) noexcept -> void;
    using Handler = std::function<void(const boost::system::error_code&)>;
    auto SetAbsolute(const Time& time) noexcept -> std::size_t;
    auto SetRelative(const std::chrono::microseconds& time) noexcept
        -> std::size_t;
    auto Wait(Handler&& handler) noexcept -> void;
    auto Wait() noexcept -> void;

    Timer() noexcept;
    Timer(Imp* imp) noexcept;
    Timer(Timer&& rhs) noexcept;
    auto operator=(Timer&& rhs) noexcept -> Timer&;

    ~Timer();

private:
    Imp* imp_;

    Timer(const Timer&) = delete;
    auto operator=(const Timer&) -> Timer& = delete;
};
}  // namespace opentxs

namespace opentxs::factory
{
auto Timer(std::shared_ptr<api::network::asio::Context> asio) noexcept
    -> opentxs::Timer;
}  // namespace opentxs::factory
