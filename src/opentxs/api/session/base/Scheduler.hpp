// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>

#include "internal/util/Lockable.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Periodic.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace base
{
class Scheduler;  // IWYU pragma: keep
}  // namespace base
}  // namespace session
}  // namespace api

class Flag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::session::base::Scheduler : virtual public api::Periodic,
                                               public Lockable
{
public:
    const api::Context& parent_;
    Flag& running_;

    auto Cancel(TaskID task) const noexcept -> bool final
    {
        return parent_.Cancel(task);
    }
    auto Reschedule(TaskID task, std::chrono::seconds interval) const noexcept
        -> bool final
    {
        return parent_.Reschedule(task, interval);
    }
    auto Schedule(std::chrono::seconds interval, SimpleCallback task)
        const noexcept -> TaskID final
    {
        return parent_.Schedule(interval, task);
    }
    auto Schedule(
        std::chrono::seconds interval,
        SimpleCallback task,
        std::chrono::seconds last) const noexcept -> TaskID final
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
    Scheduler(const api::Context& parent, Flag& running);
};
