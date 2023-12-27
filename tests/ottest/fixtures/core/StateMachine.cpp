// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/core/StateMachine.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <atomic>
#include <chrono>

namespace ottest
{
auto StateMachine::callback() -> bool
{
    while (step_.load() <= counter_.load()) { ot::sleep(10us); }

    ++counter_;

    return counter_.load() < target_.load();
}

StateMachine::StateMachine()
    : ot::internal::StateMachine([this] { return callback(); })
    , step_(0)
    , target_(0)
    , counter_(0)
{
}

}  // namespace ottest
