// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/base/Scheduler.hpp"  // IWYU pragma: associated

namespace opentxs::api::session::base
{
Scheduler::Scheduler(const api::Context& parent, Flag& running)
    : Lockable()
    , parent_(parent)
    , running_(running)
{
}

Scheduler::~Scheduler() = default;
}  // namespace opentxs::api::session::base
