// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <future>

#include "internal/network/zeromq/ListenCallback.hpp"

namespace ot = opentxs;
namespace zmq = opentxs::network::zeromq;

namespace ottest
{
class OPENTXS_EXPORT ListenCallback : public ::testing::Test
{
private:
    std::promise<ot::UnallocatedCString> promise_;

protected:
    const ot::UnallocatedCString test_message_;
    std::future<ot::UnallocatedCString> future_;
    ot::OTZMQListenCallback callback_;

    ListenCallback() noexcept;
};
}  // namespace ottest
