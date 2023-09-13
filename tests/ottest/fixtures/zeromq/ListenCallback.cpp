// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/ListenCallback.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <future>

#include "internal/network/zeromq/ListenCallback.hpp"

namespace ot = opentxs;
namespace zmq = opentxs::network::zeromq;

namespace ottest
{
ListenCallback::ListenCallback() noexcept
    : promise_()
    , test_message_("zeromq test message")
    , future_(promise_.get_future())
    , callback_(zmq::ListenCallback::Factory([&](auto&& input) -> void {
        promise_.set_value(ot::UnallocatedCString{input.get()[0].Bytes()});
    }))
{
}
}  // namespace ottest
