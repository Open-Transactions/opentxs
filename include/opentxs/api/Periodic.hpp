// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstddef>
#include <functional>

#include "opentxs/Export.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs
{
namespace api
{
class Periodic;  // IWYU pragma: keep
}  // namespace api

using namespace std::literals::chrono_literals;

using PeriodicTask = std::function<void()>;
}  // namespace opentxs

/**
 The Periodic API is used for scheduling and canceling recurring tasks.
 */
class OPENTXS_EXPORT opentxs::api::Periodic
{
public:
    using TaskID = std::ptrdiff_t;

    /// Cancels a periodic task.
    virtual auto Cancel(TaskID task) const noexcept -> bool = 0;
    /// Reschedules a periodic task.
    virtual auto Reschedule(TaskID task, std::chrono::seconds interval)
        const noexcept -> bool = 0;
    /** Adds a task to the periodic task list with the specified interval. By
     * default, schedules for immediate execution.
     *
     * \returns: task identifier which may be used to manage the task
     */
    virtual auto Schedule(
        std::chrono::seconds interval,
        opentxs::SimpleCallback task) const noexcept -> TaskID = 0;
    virtual auto Schedule(
        std::chrono::seconds interval,
        opentxs::SimpleCallback task,
        std::chrono::seconds last) const noexcept -> TaskID = 0;

    Periodic(const Periodic&) = delete;
    Periodic(Periodic&&) = delete;
    auto operator=(const Periodic&) -> Periodic& = delete;
    auto operator=(Periodic&&) -> Periodic& = delete;

    OPENTXS_NO_EXPORT virtual ~Periodic() = default;

protected:
    Periodic() = default;
};
