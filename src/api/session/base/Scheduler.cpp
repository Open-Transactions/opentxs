// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "api/session/base/Scheduler.hpp"  // IWYU pragma: associated

#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Thread.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs::api::session
{
Scheduler::Scheduler(const api::Context& parent, Flag& running)
    : Lockable()
    , parent_(parent)
    , running_(running)
    , periodic_()
{
}

void Scheduler::Start(const api::session::Storage* const storage)
{
    OT_ASSERT(nullptr != storage);

    periodic_ = std::thread(&Scheduler::thread, this);
}

void Scheduler::thread()
{
    SetThisThreadsName("Scheduler");

    while (running_) {
        // Storage has its own interval checking.
        storage_gc_hook();
        Sleep(100ms);
    }
}

Scheduler::~Scheduler()
{
    if (periodic_.joinable()) { periodic_.join(); }
}
}  // namespace opentxs::api::session
