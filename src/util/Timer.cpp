// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/Timer.hpp"  // IWYU pragma: associated

#include <boost/date_time/posix_time/posix_time_config.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/system/error_code.hpp>
#include <atomic>
#include <exception>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include "BoostAsio.hpp"
#include "api/network/asio/Context.hpp"
#include "opentxs/util/Log.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::factory
{
auto Timer(std::shared_ptr<api::network::asio::Context> asio) noexcept
    -> opentxs::Timer
{
    class DeadlineTimer final : public opentxs::Timer::Imp
    {
    public:
        auto Cancel() noexcept -> std::size_t final
        {
            try {

                return timer_.cancel();
            } catch (const std::exception& e) {
                LogError()()(e.what()).Flush();

                return {};
            }
        }
        auto IsActive() const noexcept -> bool final { return *is_active_; }
        auto SetAbsolute(const Time& time) noexcept -> std::size_t final
        {
            using namespace std::chrono;

            return SetRelative(
                duration_cast<microseconds>(time - Clock::now()));
        }
        auto SetRelative(const std::chrono::microseconds& time) noexcept
            -> std::size_t final
        {
            try {
                const auto boostTime =
                    boost::posix_time::microseconds{time.count()};

                return timer_.expires_from_now(boostTime);
            } catch (const std::exception& e) {
                LogError()()(e.what()).Flush();

                return {};
            }
        }
        auto Wait(Timer::Handler&& handler) noexcept -> void final
        {
            try {
                *is_active_ = true;

                return timer_.async_wait(
                    [active = is_active_,
                     cb = std::move(handler)](const auto& ec) {
                        *active = false;

                        if (cb) { std::invoke(cb, ec); }
                    });
            } catch (const std::exception& e) {
                LogError()()(e.what()).Flush();
            }
        }
        auto Wait() noexcept -> void final
        {
            try {
                const auto post = ScopeGuard{
                    [this] { *is_active_ = true; },
                    [this] { *is_active_ = false; }};

                return timer_.wait();
            } catch (const std::exception& e) {
                LogError()()(e.what()).Flush();
            }
        }

        DeadlineTimer(
            std::shared_ptr<api::network::asio::Context> asio) noexcept
            : asio_(asio)
            , timer_(asio_->get())
            , is_active_(std::make_shared<std::atomic_bool>(false))
        {
            assert_false(nullptr == asio_);
            assert_false(nullptr == is_active_);
        }

        ~DeadlineTimer() final = default;

    private:
        std::shared_ptr<api::network::asio::Context> asio_;
        boost::asio::deadline_timer timer_;
        std::shared_ptr<std::atomic_bool> is_active_;
    };

    return std::make_unique<DeadlineTimer>(asio).release();
}
}  // namespace opentxs::factory

namespace opentxs
{
auto operator<(const Timer& lhs, const Timer& rhs) noexcept -> bool
{
    return lhs.operator<(rhs);
}

auto operator==(const Timer& lhs, const Timer& rhs) noexcept -> bool
{
    return lhs.operator==(rhs);
}

auto swap(Timer& lhs, Timer& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs

namespace opentxs
{
auto Timer::Imp::IsActive() const noexcept -> bool { return false; }

auto Timer::Imp::Cancel() noexcept -> std::size_t { return {}; }

auto Timer::Imp::SetAbsolute(const Time&) noexcept -> std::size_t { return {}; }

auto Timer::Imp::SetRelative(const std::chrono::microseconds&) noexcept
    -> std::size_t
{
    return {};
}

auto Timer::Imp::Wait(Handler&& handler) noexcept -> void
{
    static const auto error = boost::system::error_code{};

    if (handler) { handler(error); }
}

auto Timer::Imp::Wait() noexcept -> void {}
}  // namespace opentxs

namespace opentxs
{
Timer::Timer(Imp* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp_);
}

Timer::Timer() noexcept
    : Timer(std::make_unique<Imp>().release())
{
}

Timer::Timer(Timer&& rhs) noexcept
    : Timer()
{
    swap(rhs);
}

auto Timer::Cancel() noexcept -> std::size_t { return imp_->Cancel(); }

auto Timer::IsActive() const noexcept -> bool { return imp_->IsActive(); }

auto Timer::operator<(const Timer& rhs) const noexcept -> bool
{
    return imp_ < rhs.imp_;
}

auto Timer::operator=(Timer&& rhs) noexcept -> Timer&
{
    swap(rhs);
    return *this;
}

auto Timer::operator==(const Timer& rhs) const noexcept -> bool
{
    return imp_ == rhs.imp_;
}

auto Timer::SetAbsolute(const Time& time) noexcept -> std::size_t
{
    return imp_->SetAbsolute(time);
}

auto Timer::SetRelative(const std::chrono::microseconds& time) noexcept
    -> std::size_t

{
    return imp_->SetRelative(time);
}

auto Timer::swap(Timer& rhs) noexcept -> void { std::swap(imp_, rhs.imp_); }

auto Timer::Wait(Handler&& handler) noexcept -> void
{
    imp_->Wait(std::move(handler));
}

auto Timer::Wait() noexcept -> void { imp_->Wait(); }

Timer::~Timer()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs
