// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/Basic.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <QCoreApplication>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <thread>

namespace ottest
{
using namespace std::literals::chrono_literals;

class QtApplication
{
private:
    const opentxs::Time started_;
    std::atomic<bool> running_;
    std::promise<QObject*> promise_;
    std::thread thread_;

public:
    std::shared_future<QObject*> future_;

    auto start() noexcept -> void
    {
        if (auto running = running_.exchange(true); false == running) {
            thread_ = std::thread{[this] {
                // NOLINTBEGIN(modernize-avoid-c-arrays)
                char test[]{"test"};
                char* argv[]{&test[0], nullptr};
                int argc{1};
                // NOLINTEND(modernize-avoid-c-arrays)
                auto qt = QCoreApplication(argc, argv);
                promise_.set_value(&qt);
                qt.exec();
            }};
        }
    }

    auto stop() noexcept -> void
    {
        if (auto running = running_.exchange(false); running) {
            future_.get();
            // FIXME find correct way to shut down QCoreApplication without
            // getting random segfaults
            static constexpr auto delay = 4s;
            static constexpr auto zero = 0us;
            const auto elapsed = opentxs::Clock::now() - started_;
            const auto wait =
                std::chrono::duration_cast<std::chrono::microseconds>(
                    delay - elapsed);
            opentxs::sleep(std::max(wait, zero));
            QCoreApplication::exit(0);

            if (thread_.joinable()) { thread_.join(); }

            promise_ = {};
            future_ = promise_.get_future();
        }
    }

    QtApplication() noexcept
        : started_(opentxs::Clock::now())
        , running_(false)
        , promise_()
        , thread_()
        , future_(promise_.get_future())
    {
    }

    ~QtApplication() { stop(); }
};

QtApplication qt_{};

auto GetQT() noexcept -> QObject*
{
    qt_.start();

    return qt_.future_.get();
}

auto StartQT(bool) noexcept -> void { qt_.start(); }

auto StopQT() noexcept -> void { qt_.stop(); }
}  // namespace ottest
