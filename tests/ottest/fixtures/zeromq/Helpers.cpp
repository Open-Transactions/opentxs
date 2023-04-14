// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/Helpers.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <chrono>
#include <future>
#include <mutex>
#include <ratio>
#include <stdexcept>
#include <utility>

#include "internal/util/Mutex.hpp"

namespace ottest
{
struct ZMQQueue::Imp {
    auto get(std::size_t index) noexcept(false) -> const Message&
    {
        auto future = data_.future(index);
        static constexpr auto timeout = std::chrono::minutes{5};
        using Status = std::future_status;

        if (const auto s = future.wait_for(timeout); s != Status::ready) {
            throw std::runtime_error{"timeout"};
        }

        return future.get();
    }
    auto receive(const Message& msg) noexcept -> void
    {
        data_.promise(++counter_).set_value(msg);
    }

    Imp() noexcept
        : lock_()
        , data_()
        , counter_(-1)
    {
    }

private:
    using Promise = std::promise<ot::network::zeromq::Message>;
    using Future = std::shared_future<ot::network::zeromq::Message>;

    struct Task {
        Promise promise_;
        Future future_;

        Task() noexcept
            : promise_()
            , future_(promise_.get_future())
        {
        }
        Task(Task&& rhs) noexcept
            : promise_(std::move(rhs.promise_))
            , future_(std::move(rhs.future_))
        {
        }
        Task(const Task&) = delete;
        auto operator=(const Task&) -> Task& = delete;
        auto operator=(Task&&) -> Task& = delete;
    };

    struct Promises {
        auto future(std::size_t index) noexcept -> Future
        {
            auto lock = ot::Lock{lock_};

            return map_[index].future_;
        }
        auto promise(std::size_t index) noexcept -> Promise&
        {
            auto lock = ot::Lock{lock_};

            return map_[index].promise_;
        }

    private:
        mutable std::mutex lock_{};
        ot::UnallocatedMap<std::size_t, Task> map_{};
    };

    mutable std::mutex lock_;
    Promises data_;
    std::ptrdiff_t counter_;
};

ZMQQueue::ZMQQueue() noexcept
    : imp_(std::make_unique<Imp>())
{
}

auto ZMQQueue::get(std::size_t index) noexcept(false) -> const Message&
{
    return imp_->get(index);
}

auto ZMQQueue::receive(const Message& msg) noexcept -> void
{
    return imp_->receive(msg);
}

ZMQQueue::~ZMQQueue() = default;
}  // namespace ottest
