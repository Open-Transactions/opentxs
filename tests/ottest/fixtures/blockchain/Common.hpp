// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <chrono>
#include <string_view>

#include "ottest/Basic.hpp"

namespace ottest
{
using namespace std::literals;

using Position = ot::blockchain::block::Position;
using Pattern = ot::blockchain::protocol::bitcoin::base::block::script::Pattern;
using FilterType = ot::blockchain::cfilter::Type;

constexpr auto test_chain_{ot::blockchain::Type::UnitTest};
constexpr auto sync_server_main_endpoint_{"inproc://sync_server/main"sv};
constexpr auto sync_server_push_endpoint_{"inproc://sync_server/push"sv};
constexpr auto coinbase_fun_{"The Industrial Revolution and its consequences "
                             "have been a disaster for the human race."};
}  // namespace ottest
