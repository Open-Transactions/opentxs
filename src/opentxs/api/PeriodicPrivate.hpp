// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <chrono>
#include <functional>
#include <tuple>

#include "internal/util/Timer.hpp"
#include "opentxs/api/Periodic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
class Asio;
}  // namespace network

class PeriodicPrivate;  // IWYU pragma: keep
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::PeriodicPrivate final : public Periodic
{
public:
    auto Cancel(TaskID task) const noexcept -> bool final;
    auto Reschedule(TaskID task, std::chrono::seconds interval) const noexcept
        -> bool final;
    auto Schedule(std::chrono::seconds interval, SimpleCallback task)
        const noexcept -> TaskID final;
    auto Schedule(
        std::chrono::seconds interval,
        SimpleCallback task,
        std::chrono::seconds last) const noexcept -> TaskID final;

    auto Shutdown() noexcept -> void;

    PeriodicPrivate(const network::Asio& asio);
    PeriodicPrivate() = delete;
    PeriodicPrivate(const PeriodicPrivate&) = delete;
    PeriodicPrivate(PeriodicPrivate&&) = delete;
    auto operator=(const PeriodicPrivate&) -> PeriodicPrivate& = delete;
    auto operator=(PeriodicPrivate&&) -> PeriodicPrivate& = delete;

    ~PeriodicPrivate() final;

private:
    using Params = std::tuple<Timer, SimpleCallback, std::chrono::microseconds>;
    using TaskMap = Map<TaskID, Params>;
    using Data = libguarded::plain_guarded<TaskMap>;

    const network::Asio& asio_;
    mutable Data data_;

    static auto first_interval(
        std::chrono::seconds interval,
        std::chrono::seconds last) noexcept -> std::chrono::microseconds;
    static auto next_id() noexcept -> TaskID;

    auto make_callback(TaskID id) const noexcept -> Timer::Handler;
    auto run(TaskID id) const noexcept -> void;
};
