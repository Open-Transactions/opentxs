// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <chrono>

#include "core/StateMachine.hpp"

namespace ottest
{
namespace ot = opentxs;

using namespace std::literals::chrono_literals;

class OPENTXS_EXPORT StateMachine : public ::testing::Test,
                                    public ot::internal::StateMachine
{
public:
    std::atomic<int> step_;
    std::atomic<int> target_;
    std::atomic<int> counter_;

    auto callback() -> bool;

    StateMachine();
};
}  // namespace ottest


