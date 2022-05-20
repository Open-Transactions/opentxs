// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <thread>
#include <thread>
#include <tuple>

#include "opentxs/api/Periodic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Flag;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::imp
{
class Periodic : virtual public api::Periodic
{
public:
    auto Cancel(const int task) const -> bool final;
    auto Reschedule(const int task, const std::chrono::seconds& interval) const
        -> bool final;
    auto Schedule(
        const std::chrono::seconds& interval,
        const PeriodicTask& task,
        const std::chrono::seconds& last) const -> int final;

    Periodic() = delete;
    Periodic(const Periodic&) = delete;
    Periodic(Periodic&&) = delete;
    auto operator=(const Periodic&) -> Periodic& = delete;
    auto operator=(Periodic&&) -> Periodic& = delete;

    ~Periodic() override;

protected:
    Flag& running_;

    void Shutdown();

    Periodic(Flag& running);

private:
    /** Last performed, Interval, Task */
    using TaskItem = std::tuple<Time, std::chrono::seconds, PeriodicTask>;
    using TaskList = UnallocatedMap<int, TaskItem>;

    mutable std::atomic<int> next_id_;
    mutable std::mutex periodic_lock_;
    mutable TaskList periodic_task_list_;
    std::thread periodic_;

    void thread();
};
}  // namespace opentxs::api::imp
