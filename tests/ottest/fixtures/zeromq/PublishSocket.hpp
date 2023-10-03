// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
class OPENTXS_EXPORT PublishSocket : public ::testing::Test
{
public:
    const zmq::Context& context_;

    PublishSocket();
};
}  // namespace ottest
