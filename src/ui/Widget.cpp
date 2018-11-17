// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"

#include "Widget.hpp"

#define OT_METHOD "opentxs::ui::implementation::Widget::"

namespace opentxs::ui::implementation
{
Widget::Widget(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const Identifier& id)
    : api_(api)
    , publisher_(publisher)
    , widget_id_(Identifier::Factory(id))
    , callbacks_()
    , listeners_()
{
}

Widget::Widget(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher)
    : Widget(api, publisher, Identifier::Random())
{
}

void Widget::setup_listeners(const ListenerDefinitions& definitions)
{
    for (const auto& [endpoint, functor] : definitions) {
        const auto* copy{functor};
        auto& nextCallback =
            callbacks_.emplace_back(network::zeromq::ListenCallback::Factory(
                [=](const network::zeromq::Message& message) -> void {
                    (*copy)(this, message);
                }));
        auto& socket = listeners_.emplace_back(
            api_.ZeroMQ().SubscribeSocket(nextCallback.get()));
        const auto listening = socket->Start(endpoint);

        OT_ASSERT(listening)
    }
}

void Widget::UpdateNotify() const
{
    publisher_.Publish(widget_id_->str());
    LogOutput(OT_METHOD)(__FUNCTION__)(": Widget ")(widget_id_)(" updated.")
        .Flush();
}

OTIdentifier Widget::WidgetID() const
{
    return Identifier::Factory(widget_id_);
}
}  // namespace opentxs::ui::implementation
