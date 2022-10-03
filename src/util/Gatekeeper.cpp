// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "util/Gatekeeper.hpp"  // IWYU pragma: associated

#include <atomic>
#include <future>
#include <thread>

#include "internal/util/LogMacros.hpp"

namespace opentxs
{
struct Gatekeeper::Imp {
    std::atomic_int count_;
    std::promise<void> promise_;
    std::shared_future<void> future_;

    Imp() noexcept
        : count_(0)
        , promise_()
        , future_(promise_.get_future())
    {
    }
};

struct Ticket::Imp {
    std::atomic_int& count_;
    const bool invalid_;

    Imp(Gatekeeper::Imp& parent) noexcept
        : count_(parent.count_)
        , invalid_([&] {
            auto jobs = count_.load();

            if (0 > jobs) { return true; }

            while (false == count_.compare_exchange_strong(jobs, jobs + 1)) {
                if (0 > jobs) { return true; }
            }

            return false;
        }())
    {
    }
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp()
    {
        if (invalid_) { return; }

        --count_;
    }
};

Gatekeeper::Gatekeeper() noexcept
    : imp_(std::make_unique<Imp>())
{
    OT_ASSERT(imp_);
}

Ticket::Ticket(Gatekeeper::Imp& parent) noexcept
    : imp_(std::make_unique<Imp>(parent))
{
    OT_ASSERT(imp_);
}

Ticket::Ticket(Ticket&& rhs) noexcept
    : imp_(rhs.imp_.release())
{
    OT_ASSERT(imp_);
}

auto Gatekeeper::get() const noexcept -> Ticket { return Ticket{*imp_}; }

auto Gatekeeper::shutdown() noexcept -> void
{
    auto& count = imp_->count_;
    auto jobs = count.load();

    if (0 > jobs) {
        imp_->future_.get();

        return;
    }

    static constexpr auto shutdown{-1};

    while (false == count.compare_exchange_strong(jobs, shutdown)) {
        if (0 > jobs) {
            imp_->future_.get();

            return;
        }
    }

    OT_ASSERT(0 <= jobs);

    const auto target = shutdown - jobs;

    while (count.load() > target) { std::this_thread::yield(); }

    imp_->promise_.set_value();
}

Ticket::operator bool() const noexcept
{
    if (imp_) { return imp_->invalid_; }

    return true;
}

Gatekeeper::~Gatekeeper() { shutdown(); }

Ticket::~Ticket() = default;
}  // namespace opentxs
