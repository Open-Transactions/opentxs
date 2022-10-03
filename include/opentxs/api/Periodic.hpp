// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstddef>
#include <functional>

#include "opentxs/Export.hpp"

namespace opentxs
{
using namespace std::literals::chrono_literals;

using PeriodicTask = std::function<void()>;
}  // namespace opentxs

namespace opentxs::api
{
/**
 The Periodic API is used for scheduling and canceling recurring tasks.
 */
class OPENTXS_EXPORT Periodic
{
public:
    using TaskID = std::ptrdiff_t;

    /// Cancels a periodic task.
    virtual auto Cancel(const TaskID task) const -> bool = 0;
    /// Reschedules a periodic task.
    virtual auto Reschedule(
        const TaskID task,
        const std::chrono::seconds& interval) const -> bool = 0;
    /** Adds a task to the periodic task list with the specified interval. By
     * default, schedules for immediate execution.
     *
     * \returns: task identifier which may be used to manage the task
     */
    virtual auto Schedule(
        const std::chrono::seconds& interval,
        const opentxs::PeriodicTask& task,
        const std::chrono::seconds& last = 0s) const -> TaskID = 0;

    Periodic(const Periodic&) = delete;
    Periodic(Periodic&&) = delete;
    auto operator=(const Periodic&) -> Periodic& = delete;
    auto operator=(Periodic&&) -> Periodic& = delete;

    OPENTXS_NO_EXPORT virtual ~Periodic() = default;

protected:
    Periodic() = default;
};
}  // namespace opentxs::api
