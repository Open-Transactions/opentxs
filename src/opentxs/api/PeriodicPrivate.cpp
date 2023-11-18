// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/PeriodicPrivate.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <compare>
#include <exception>
#include <functional>
#include <memory>
#include <ratio>
#include <utility>

#include "BoostAsio.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::api
{
PeriodicPrivate::PeriodicPrivate(const network::Asio& asio)
    : asio_(asio)
    , data_()
{
}

auto PeriodicPrivate::Cancel(TaskID task) const noexcept -> bool
{
    return 1_uz == data_.lock()->erase(task);
}

auto PeriodicPrivate::first_interval(
    std::chrono::seconds interval,
    std::chrono::seconds last) noexcept -> std::chrono::microseconds
{
    if (last <= interval) {

        return interval - last;
    } else {

        return interval;
    }
}

auto PeriodicPrivate::make_callback(TaskID id) const noexcept -> Timer::Handler
{
    return [this, id](auto& ec) {
        if (ec) {
            if (unexpected_asio_error(ec)) {
                LogError()()("received asio error (")(ec.value())(") :")(ec)
                    .Flush();
            }
        } else {
            this->run(id);
        }
    };
}

auto PeriodicPrivate::next_id() noexcept -> TaskID
{
    static auto counter = std::atomic_int{-1};

    return ++counter;
}

auto PeriodicPrivate::Reschedule(TaskID id, std::chrono::seconds interval)
    const noexcept -> bool
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

auto PeriodicPrivate::run(TaskID id) const noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto i = data.find(id);

    if (data.end() == i) { return; }

    auto& [timer, task, period] = i->second;

    try {
        std::invoke(task);
    } catch (std::exception& e) {
        LogError()()(e.what()).Flush();
    }

    timer.SetRelative(period);
    timer.Wait(make_callback(id));
}

auto PeriodicPrivate::Schedule(
    std::chrono::seconds interval,
    SimpleCallback task) const noexcept -> TaskID
{
    return Schedule(interval, task, 0s);
}

auto PeriodicPrivate::Schedule(
    std::chrono::seconds interval,
    SimpleCallback job,
    std::chrono::seconds last) const noexcept -> TaskID
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto [i, added] =
        data.try_emplace(next_id(), asio_.Internal().GetTimer(), job, interval);

    assert_true(added);

    const auto& id = i->first;
    auto& [timer, task, period] = i->second;
    timer.SetRelative(first_interval(interval, last));
    timer.Wait(make_callback(id));

    return id;
}

auto PeriodicPrivate::Shutdown() noexcept -> void { data_.lock()->clear(); }

PeriodicPrivate::~PeriodicPrivate() { Shutdown(); }
}  // namespace opentxs::api
