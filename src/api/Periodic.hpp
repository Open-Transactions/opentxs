// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <thread>
#include <tuple>

#include "internal/util/Timer.hpp"
#include "opentxs/api/Periodic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
class Asio;
}  // namespace network
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::imp
{
class Periodic final : public api::Periodic
{
public:
    auto Cancel(const TaskID task) const -> bool final;
    auto Reschedule(const TaskID task, const std::chrono::seconds& interval)
        const -> bool final;
    auto Schedule(
        const std::chrono::seconds& interval,
        const PeriodicTask& task,
        const std::chrono::seconds& last) const -> TaskID final;

    auto Shutdown() -> void;

    Periodic(const network::Asio& asio);
    Periodic() = delete;
    Periodic(const Periodic&) = delete;
    Periodic(Periodic&&) = delete;
    auto operator=(const Periodic&) -> Periodic& = delete;
    auto operator=(Periodic&&) -> Periodic& = delete;

    ~Periodic() final;

private:
    using Params = std::tuple<Timer, PeriodicTask, std::chrono::microseconds>;
    using TaskMap = Map<TaskID, Params>;
    using Data = libguarded::plain_guarded<TaskMap>;

    const network::Asio& asio_;
    mutable Data data_;

    static auto first_interval(
        const std::chrono::seconds& interval,
        const std::chrono::seconds& last) noexcept -> std::chrono::microseconds;
    static auto next_id() noexcept -> TaskID;

    auto make_callback(TaskID id) const noexcept -> Timer::Handler;
    auto run(TaskID id) const noexcept -> void;
};
}  // namespace opentxs::api::imp
