// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/qt/SendMonitor.hpp"  // IWYU pragma: associated

#include <QString>  // IWYU pragma: keep
#include <atomic>
#include <chrono>
#include <compare>
#include <future>
#include <mutex>
#include <ratio>
#include <thread>
#include <tuple>
#include <utility>

#include "internal/util/Mutex.hpp"
#include "internal/util/Thread.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/ByteArray.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::ui::implementation
{
struct SendMonitor::Imp {
    using Key = int;

    auto shutdown() noexcept -> void
    {
        {
            auto lock = Lock{lock_};
            running_ = false;
        }

        future_.get();
    }
    auto watch(Future&& future, Callback&& cb) noexcept -> Key
    {
        auto lock = Lock{lock_};

        if (false == running_) { return -1; }

        const auto& [key, data, callback] =
            queue_.emplace_back(++counter_, std::move(future), std::move(cb));

        return key;
    }

    Imp() noexcept
        : lock_()
        , counter_(-1)
        , queue_()
        , running_(true)
        , promise_()
        , future_(promise_.get_future())
        , thread_(&Imp::thread, this)
    {
    }

    ~Imp()
    {
        running_ = false;

        if (thread_.joinable()) { thread_.join(); }
    }

private:
    using Data = std::tuple<Key, Future, Callback>;
    using Duration = std::chrono::milliseconds;

    mutable std::mutex lock_;
    Key counter_;
    UnallocatedList<Data> queue_;
    std::atomic_bool running_;
    std::promise<void> promise_;
    std::shared_future<void> future_;
    std::thread thread_;

    static auto process(Data& data) noexcept -> bool
    {
        auto& [key, future, cb] = data;
        static constexpr auto wait = 1us;
        using Status = std::future_status;

        if (Status::ready != future.wait_for(wait)) { return false; }

        const auto [rc, txid] = future.get();

        cb(key, static_cast<int>(rc), blockchain::HashToNumber(txid).c_str());

        return true;
    }
    static auto rate_limit(const Time& start, const Duration& minimum) noexcept
        -> void
    {
        const auto elapsed = Clock::now() - start;

        if (elapsed > minimum) { return; }

        Sleep(std::chrono::duration_cast<Duration>(minimum - elapsed));
    }

    auto run() noexcept -> void
    {
        for (auto i{queue_.begin()}; i != queue_.end();) {
            if (process(*i)) {
                i = queue_.erase(i);
            } else {
                ++i;
            }
        }
    }
    auto thread() noexcept -> void
    {
        SetThisThreadsName("SendMonitor");

        static constexpr auto limit = Duration{25};

        while (running_) {
            const auto start = Clock::now();
            auto post = ScopeGuard{[&] { rate_limit(start, limit); }};
            auto lock = Lock{lock_};

            if (0u == queue_.size()) { continue; }

            run();
        }

        // NOTE: no lock needed here since other threads will now refuse to
        // modify queue_
        while (0u < queue_.size()) {
            const auto start = Clock::now();
            auto post = ScopeGuard{[&] { rate_limit(start, limit); }};
            run();
        }

        promise_.set_value();
    }
};

SendMonitor::SendMonitor() noexcept
    : imp_(std::make_unique<Imp>())
{
}

auto SendMonitor::shutdown() noexcept -> void { imp_->shutdown(); }

auto SendMonitor::watch(Future&& future, Callback&& cb) noexcept -> int
{
    return imp_->watch(std::move(future), std::move(cb));
}

SendMonitor::~SendMonitor() { shutdown(); }
}  // namespace opentxs::ui::implementation
