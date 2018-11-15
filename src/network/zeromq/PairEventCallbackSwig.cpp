// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "PairEventCallbackSwig.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PairEventCallbackSwig.hpp"

#define OT_METHOD                                                              \
    "opentxs::network::zeromq::implementation::PairEventCallbackSwig::"

namespace opentxs::network::zeromq
{
OTZMQPairEventCallback PairEventCallback::Factory(
    opentxs::PairEventCallbackSwig* callback)
{
    return OTZMQPairEventCallback(
        new implementation::PairEventCallbackSwig(callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
PairEventCallbackSwig::PairEventCallbackSwig(
    opentxs::PairEventCallbackSwig* callback)
    : callback_(callback)
{
    if (nullptr == callback_) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid callback pointer.")
            .Flush();

        OT_FAIL;
    }
}

PairEventCallbackSwig* PairEventCallbackSwig::clone() const
{
    return new PairEventCallbackSwig(callback_);
}

void PairEventCallbackSwig::Process(zeromq::Message& message) const
{
    OT_ASSERT(nullptr != callback_)
    OT_ASSERT(1 == message.Body().size());

    const auto event = proto::TextToProto<proto::PairEvent>(message.Body_at(0));

    switch (event.type()) {
        case proto::PAIREVENT_RENAME: {
            callback_->ProcessRename(event.issuer());
        } break;
        case proto::PAIREVENT_STORESECRET: {
            callback_->ProcessStoreSecret(event.issuer());
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unknown event type.").Flush();
        }
    }
}

PairEventCallbackSwig::~PairEventCallbackSwig() {}
}  // namespace opentxs::network::zeromq::implementation
