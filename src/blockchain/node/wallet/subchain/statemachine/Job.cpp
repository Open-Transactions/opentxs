// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"  // IWYU pragma: associated

#include <chrono>
#include <functional>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "util/JobCounter.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
Job::Job(const ThreadPool pool, SubchainStateData& parent) noexcept
    : parent_(parent)
    , shutdown_(false)
    , lock_()
    , thread_pool_(pool)
    , running_(0)
    , cv_()
{
}

auto Job::finish(Lock& lock) noexcept -> void
{
    --running_;

    OT_ASSERT(0 <= running_);

    lock.unlock();
    cv_.notify_all();
}

auto Job::IsRunning() const noexcept -> bool
{
    auto lock = Lock{lock_};

    return is_running(lock);
}

auto Job::is_running(Lock& lock) const noexcept -> bool
{
    static constexpr auto zero = 0s;
    cv_.wait_for(lock, zero);

    return (0 < running_);
}

auto Job::queue_work(
    SimpleCallback cb,
    const char* msg,
    bool lockIsHeld) noexcept -> bool
{
    OT_ASSERT(cb);

    ++parent_.job_counter_;

    {
        auto lock = Lock{lock_, std::defer_lock};

        if (false == lockIsHeld) { lock.lock(); }

        ++running_;
    }

    const auto& log = LogTrace();
    const auto queued = parent_.api_.Network().Asio().Internal().Post(
        thread_pool_,
        [this, &log, job = std::move(cb), kind = UnallocatedCString{msg}] {
            auto post = ScopeGuard{[&] {
                log(OT_PRETTY_CLASS())(parent_.name_)(" ")(kind)(" job for ")(
                    parent_.db_key_->str())(" complete. Sending ")(
                    this->type())(" notification.")
                    .Flush();
                parent_.task_finished_(parent_.db_key_, this->type());
                --parent_.job_counter_;
            }};

            job();
        });

    if (queued) {
        log(OT_PRETTY_CLASS())(parent_.name_)(" ")(msg)(" job queued").Flush();
    } else {
        log(OT_PRETTY_CLASS())(parent_.name_)(" failed to queue ")(msg)(" job")
            .Flush();
        --parent_.job_counter_;
    }

    return queued;
}

auto Job::Shutdown() noexcept -> void { shutdown_ = true; }

auto Job::wait(Lock& lock) const noexcept -> void
{
    const auto start = Clock::now();
    static constexpr auto limit = 10s;

    while (true) {
        if (cv_.wait_for(lock, limit, [this] { return 0 == running_; })) {
            break;
        }

        const auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            Clock::now() - start);
        LogError()(OT_PRETTY_CLASS())(parent_.name_)(" waiting on ")(type())(
            " job for ")(duration.count())(" seconds")
            .Flush();
    }
}
}  // namespace opentxs::blockchain::node::wallet
