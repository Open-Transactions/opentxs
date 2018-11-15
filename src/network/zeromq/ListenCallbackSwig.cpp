// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "ListenCallbackSwig.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallbackSwig.hpp"

#define OT_METHOD                                                              \
    "opentxs::network::zeromq::implementation::ListenCallbackSwig::"

namespace opentxs::network::zeromq
{
OTZMQListenCallback ListenCallback::Factory(
    opentxs::ListenCallbackSwig* callback)
{
    return OTZMQListenCallback(
        new implementation::ListenCallbackSwig(callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
ListenCallbackSwig::ListenCallbackSwig(opentxs::ListenCallbackSwig* callback)
    : callback_(callback)
{
    if (nullptr == callback_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid callback pointer.")
            .Flush();

        OT_FAIL;
    }
}

ListenCallbackSwig* ListenCallbackSwig::clone() const
{
    return new ListenCallbackSwig(callback_);
}

void ListenCallbackSwig::Process(zeromq::Message& message) const
{
    OT_ASSERT(nullptr != callback_)

    callback_->Process(message);
}

ListenCallbackSwig::~ListenCallbackSwig() {}
}  // namespace opentxs::network::zeromq::implementation
