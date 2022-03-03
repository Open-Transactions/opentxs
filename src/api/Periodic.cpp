// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"      // IWYU pragma: associated
#include "1_Internal.hpp"    // IWYU pragma: associated
#include "api/Periodic.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <tuple>
#include <utility>

#include "internal/util/Flag.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::api::imp
{
Periodic::Periodic(Flag& running)
    : running_(running)
    , next_id_(0)
    , periodic_lock_()
    , periodic_task_list_()
    , periodic_(&Periodic::thread, this)
{
}

auto Periodic::Cancel(const int task) const -> bool
{
    Lock lock(periodic_lock_);
    const auto output = periodic_task_list_.erase(task);

    return 1 == output;
}

auto Periodic::Reschedule(const int task, const std::chrono::seconds& interval)
    const -> bool
{
    Lock lock(periodic_lock_);
    auto it = periodic_task_list_.find(task);

    if (periodic_task_list_.end() == it) { return false; }

    std::get<1>(it->second) = interval;

    return false;
}

auto Periodic::Schedule(
    const std::chrono::seconds& interval,
    const PeriodicTask& task,
    const std::chrono::seconds& last) const -> int
{
    const auto id = ++next_id_;
    Lock lock(periodic_lock_);
    periodic_task_list_.emplace(
        id, TaskItem{Clock::from_time_t(last.count()), interval, task});

    return id;
}

void Periodic::Shutdown()
{
    if (periodic_.joinable()) { periodic_.join(); }
}

void Periodic::thread()
{
    while (running_) {
        const auto now = Clock::now();
        Lock lock(periodic_lock_);

        for (auto& [key, value] : periodic_task_list_) {
            auto& [last, interval, task] = value;

            if ((now - last) > interval) {
                last = now;
                std::thread taskThread{task};
                taskThread.detach();
            }
        }

        lock.unlock();
        Sleep(100ms);
    }
}

Periodic::~Periodic() { Shutdown(); }
}  // namespace opentxs::api::imp
