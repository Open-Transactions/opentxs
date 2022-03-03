// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs::api
{
class OPENTXS_EXPORT Periodic
{
public:
    virtual auto Cancel(const int task) const -> bool = 0;
    virtual auto Reschedule(
        const int task,
        const std::chrono::seconds& interval) const -> bool = 0;
    /** Adds a task to the periodic task list with the specified interval. By
     * default, schedules for immediate execution.
     *
     * \returns: task identifier which may be used to manage the task
     */
    virtual auto Schedule(
        const std::chrono::seconds& interval,
        const opentxs::PeriodicTask& task,
        const std::chrono::seconds& last = 0s) const -> int = 0;

    OPENTXS_NO_EXPORT virtual ~Periodic() = default;

protected:
    Periodic() = default;

private:
    Periodic(const Periodic&) = delete;
    Periodic(Periodic&&) = delete;
    auto operator=(const Periodic&) -> Periodic& = delete;
    auto operator=(Periodic&&) -> Periodic& = delete;
};
}  // namespace opentxs::api
