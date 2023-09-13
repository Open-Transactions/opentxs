// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/RequestSocket.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;
namespace zmq = ot::network::zeromq;

namespace ottest
{
RequestSocket::RequestSocket()
    : context_(OTTestEnvironment::GetOT().ZMQ())
{
}
}  // namespace ottest
