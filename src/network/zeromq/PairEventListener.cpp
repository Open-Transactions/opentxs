// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Socket.hpp"
#include "opentxs/network/zeromq/PairEventCallback.hpp"

#include "PairEventListener.hpp"

#include "opentxs/core/Log.hpp"

#define PAIR_EVENT_ENDPOINT_PATH "pairevent"
#define PAIR_EVENT_ENDPOINT_VERSION 1

#define OT_METHOD                                                              \
    "opentxs::network::zeromq::implementation::PairEventListener::"

namespace opentxs::network::zeromq::implementation
{
PairEventListener::PairEventListener(
    const zeromq::Context& context,
    const zeromq::PairEventCallback& callback,
    const int instance)
    : ot_super(context, callback)
    , instance_(instance)
{
    const auto endpoint = context_.BuildEndpoint(
        PAIR_EVENT_ENDPOINT_PATH, instance, PAIR_EVENT_ENDPOINT_VERSION);
    const bool started = Start(endpoint);

    OT_ASSERT(started)

    LogVerbose(OT_METHOD)(__FUNCTION__)(": listening on ")(endpoint).Flush();
}

PairEventListener* PairEventListener::clone() const
{
    return new PairEventListener(
        context_,
        dynamic_cast<const zeromq::PairEventCallback&>(callback_),
        instance_);
}
}  // namespace opentxs::network::zeromq::implementation
