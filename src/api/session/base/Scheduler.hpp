// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <thread>

#include "internal/util/Lockable.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Periodic.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Storage;
}  // namespace session
}  // namespace api

class Flag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
class Scheduler : virtual public api::Periodic, public Lockable
{
public:
    const api::Context& parent_;
    Flag& running_;

    auto Cancel(const TaskID task) const -> bool final
    {
        return parent_.Cancel(task);
    }
    auto Reschedule(const TaskID task, const std::chrono::seconds& interval)
        const -> bool final
    {
        return parent_.Reschedule(task, interval);
    }
    auto Schedule(
        const std::chrono::seconds& interval,
        const PeriodicTask& task,
        const std::chrono::seconds& last) const -> TaskID final
    {
        return parent_.Schedule(interval, task, last);
    }
    Scheduler() = delete;
    Scheduler(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    auto operator=(const Scheduler&) -> Scheduler& = delete;
    auto operator=(Scheduler&&) -> Scheduler& = delete;

    ~Scheduler() override;

protected:
    void Start(const api::session::Storage* const storage);

    Scheduler(const api::Context& parent, Flag& running);

private:
    std::thread periodic_;

    virtual void storage_gc_hook() = 0;

    void thread();
};
}  // namespace opentxs::api::session
