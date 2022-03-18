// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "1_Internal.hpp"       // IWYU pragma: associated
#include "util/JobCounter.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
using OutstandingMap = UnallocatedMap<int, std::atomic_int>;

struct Outstanding::Imp {
    auto operator++() noexcept -> Imp&
    {
        {
            auto lock = Lock{lock_};
            const auto count = ++(position_->second);
            idle_ = finished(count);
            limited_ = limited(count);
        }

        finished_.notify_all();
        ready_.notify_all();

        return *this;
    }
    auto operator--() noexcept -> Imp&
    {
        {
            auto lock = Lock{lock_};
            const auto count = --(position_->second);
            idle_ = finished(count);
            limited_ = limited(count);
        }

        finished_.notify_all();
        ready_.notify_all();

        return *this;
    }
    auto wait_for_finished() noexcept -> void
    {
        auto lock = Lock{lock_};
        finished_.wait(lock, [this] { return finished(); });
    }
    auto wait_for_ready() noexcept -> void
    {
        auto lock = Lock{lock_};
        ready_.wait(lock, [this] { return false == limited(); });
    }

    Imp(JobCounter::Imp& parent,
        OutstandingMap::iterator position,
        int limit) noexcept
        : limit_([&] {
            const auto threads =
                static_cast<int>(std::thread::hardware_concurrency());

            if (0 >= limit) {

                return threads;
            } else {

                return std::min<int>(threads, limit);
            }
        }())
        , parent_(parent)
        , lock_()
        , idle_(true)
        , limited_(false)
        , finished_()
        , ready_()
        , position_(position)
    {
    }

    ~Imp();

private:
    const int limit_;
    JobCounter::Imp& parent_;
    std::mutex lock_;
    bool idle_;
    bool limited_;
    std::condition_variable finished_;
    std::condition_variable ready_;
    OutstandingMap::iterator position_;

    auto finished() const noexcept -> bool { return finished(value()); }
    auto finished(int count) const noexcept -> bool { return 0 == count; }
    auto limited() const noexcept -> bool { return limited(value()); }
    auto limited(int count) const noexcept -> bool { return count >= limit_; }
    auto value() const noexcept -> int { return position_->second; }

    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

struct JobCounter::Imp {
    auto Allocate(int limit) noexcept -> Outstanding
    {
        auto lock = Lock{lock_};
        auto [it, added] = map_.emplace(++counter_, 0);

        OT_ASSERT(added);

        return Outstanding{
            std::make_unique<Outstanding::Imp>(*this, it, limit).release()};
    }
    auto Deallocate(OutstandingMap::iterator position) noexcept -> void
    {
        auto lock = Lock{lock_};
        map_.erase(position);
    }

    Imp() noexcept
        : lock_()
        , counter_()
        , map_()
    {
    }

    ~Imp() = default;

private:
    mutable std::mutex lock_;
    int counter_;
    OutstandingMap map_;

    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

JobCounter::JobCounter() noexcept
    : imp_(std::make_unique<Imp>().release())
{
}

auto JobCounter::Allocate(int limit) noexcept -> Outstanding
{
    return imp_->Allocate(limit);
}

Outstanding::Outstanding(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

Outstanding::Outstanding(Outstanding&& rhs) noexcept
    : imp_(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

auto Outstanding::operator++() noexcept -> Outstanding&
{
    ++(*imp_);

    return *this;
}

auto Outstanding::operator--() noexcept -> Outstanding&
{
    --(*imp_);

    return *this;
}

auto Outstanding::wait_for_finished() noexcept -> void
{
    imp_->wait_for_finished();
}

auto Outstanding::wait_for_ready() noexcept -> void { imp_->wait_for_ready(); }

JobCounter::~JobCounter()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}

Outstanding::Imp::~Imp()
{
    wait_for_finished();
    parent_.Deallocate(position_);
}

Outstanding::~Outstanding()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs
