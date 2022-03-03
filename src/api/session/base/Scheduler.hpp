// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <thread>
#include <tuple>

#include "internal/util/Lockable.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Periodic.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace network
{
class Dht;
}  // namespace network

namespace session
{
class Storage;
}  // namespace session
}  // namespace api

class Flag;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
class Scheduler : virtual public api::Periodic, public Lockable
{
public:
    const api::Context& parent_;
    std::int64_t nym_publish_interval_{0};
    std::int64_t nym_refresh_interval_{0};
    std::int64_t server_publish_interval_{0};
    std::int64_t server_refresh_interval_{0};
    std::int64_t unit_publish_interval_{0};
    std::int64_t unit_refresh_interval_{0};
    Flag& running_;

    auto Cancel(const int task) const -> bool final
    {
        return parent_.Cancel(task);
    }
    auto Reschedule(const int task, const std::chrono::seconds& interval) const
        -> bool final
    {
        return parent_.Reschedule(task, interval);
    }
    auto Schedule(
        const std::chrono::seconds& interval,
        const PeriodicTask& task,
        const std::chrono::seconds& last) const -> int final
    {
        return parent_.Schedule(interval, task, last);
    }

    ~Scheduler() override;

protected:
    void Start(
        const api::session::Storage* const storage,
        const api::network::Dht& dht);

    Scheduler(const api::Context& parent, Flag& running);

private:
    std::thread periodic_;

    virtual void storage_gc_hook() = 0;

    Scheduler() = delete;
    Scheduler(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    auto operator=(const Scheduler&) -> Scheduler& = delete;
    auto operator=(Scheduler&&) -> Scheduler& = delete;

    void thread();
};
}  // namespace opentxs::api::session
