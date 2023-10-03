// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <zmq.h>
#include <string_view>

namespace ot = opentxs;
namespace zmq = opentxs::network::zeromq;

namespace ottest
{
using namespace std::literals;

class OPENTXS_EXPORT Frame : public ::testing::Test
{
protected:
    const ot::UnallocatedCString test_string_{"testString"};
    ot::network::zeromq::Message message_{};
};
}  // namespace ottest
