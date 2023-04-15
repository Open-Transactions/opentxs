// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/Periodic.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>
#include <atomic>
#include <chrono>
#include <compare>
#include <exception>
#include <functional>
#include <memory>
#include <utility>

#include "internal/api/network/Asio.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::api::imp
{
Periodic::Periodic(const network::Asio& asio)
    : asio_(asio)
    , data_()
{
}

auto Periodic::Cancel(const TaskID task) const -> bool
{
    return 1_uz == data_.lock()->erase(task);
}

auto Periodic::first_interval(
    const std::chrono::seconds& interval,
    const std::chrono::seconds& last) noexcept -> std::chrono::microseconds
{
    if (last <= interval) {

        return interval - last;
    } else {

        return interval;
    }
}

auto Periodic::make_callback(TaskID id) const noexcept -> Timer::Handler
{
    return [this, id](auto& ec) {
        if (ec) {
            if (boost::system::errc::operation_canceled != ec.value()) {
                LogError()(OT_PRETTY_CLASS())(ec).Flush();
            }
        } else {
            this->run(id);
        }
    };
}

auto Periodic::next_id() noexcept -> TaskID
{
    static auto counter = std::atomic_int{-1};

    return ++counter;
}

auto Periodic::Reschedule(const TaskID id, const std::chrono::seconds& interval)
    const -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto i = data.find(id);

    if (data.end() == i) { return false; }

    auto& [timer, task, period] = i->second;

    timer.Cancel();
    period = interval;
    timer.SetRelative(period);
    timer.Wait(make_callback(id));

    return true;
}

auto Periodic::run(TaskID id) const noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto i = data.find(id);

    if (data.end() == i) { return; }

    auto& [timer, task, period] = i->second;

    try {
        std::invoke(task);
    } catch (std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    timer.SetRelative(period);
    timer.Wait(make_callback(id));
}

auto Periodic::Schedule(
    const std::chrono::seconds& interval,
    const PeriodicTask& job,
    const std::chrono::seconds& last) const -> TaskID
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto [i, added] =
        data.try_emplace(next_id(), asio_.Internal().GetTimer(), job, interval);

    OT_ASSERT(added);

    const auto& id = i->first;
    auto& [timer, task, period] = i->second;
    timer.SetRelative(first_interval(interval, last));
    timer.Wait(make_callback(id));

    return id;
}

auto Periodic::Shutdown() -> void { data_.lock()->clear(); }

Periodic::~Periodic() { Shutdown(); }
}  // namespace opentxs::api::imp
