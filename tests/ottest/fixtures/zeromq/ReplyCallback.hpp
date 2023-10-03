// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <span>
#include <utility>

#include "internal/network/zeromq/ReplyCallback.hpp"

namespace ot = opentxs;

namespace ottest
{
class OPENTXS_EXPORT ReplyCallback : public ::testing::Test
{
public:
    const ot::UnallocatedCString test_message_{"zeromq test message"};
};
}  // namespace ottest
