// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "interface/ui/base/Widget.hpp"  // IWYU pragma: associated

#include <type_traits>

#include "internal/api/session/UI.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::ui::implementation
{
auto verify_empty(const CustomData& custom) noexcept -> bool
{
    auto counter = -1_z;

    for (const auto& ptr : custom) {
        ++counter;

        if (nullptr != ptr) {
            LogError()("opentxs::ui::implementation::")(__func__)(
                ": unused pointer at index ")(counter)
                .Flush();

            return false;
        }
    }

    return true;
}

Widget::Widget(
    const api::session::Client& api,
    const identifier::Generic& id,
    const SimpleCallback& cb) noexcept
    : widget_id_(id)
    , ui_(api.UI())
    , callbacks_()
    , listeners_()
    , need_clear_callbacks_(true)
{
    if (cb) { SetCallback(cb); }
}

auto Widget::ClearCallbacks() const noexcept -> void
{
    ui_.Internal().ClearUICallbacks(widget_id_);
    need_clear_callbacks_ = false;
}

auto Widget::SetCallback(SimpleCallback cb) const noexcept -> void
{
    ui_.Internal().RegisterUICallback(WidgetID(), cb);
}

auto Widget::setup_listeners(
    const api::session::Client& api,
    const ListenerDefinitions& definitions) noexcept -> void
{
    for (const auto& [endpoint, functor] : definitions) {
        const auto* copy{functor};
        auto& nextCallback =
            callbacks_.emplace_back(network::zeromq::ListenCallback::Factory(
                [=, this](const Message& message) -> void {
                    (*copy)(this, message);
                }));
        auto& socket =
            listeners_.emplace_back(api.Network().ZeroMQ().SubscribeSocket(
                nextCallback.get(), "Widget"));
        const auto listening = socket->Start(endpoint);

        OT_ASSERT(listening);
    }
}

auto Widget::UpdateNotify() const noexcept -> void
{
    ui_.Internal().ActivateUICallback(WidgetID());
}

Widget::~Widget()
{
    if (need_clear_callbacks_) { Widget::ClearCallbacks(); }
}
}  // namespace opentxs::ui::implementation
