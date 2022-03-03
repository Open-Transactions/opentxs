// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "interface/ui/base/Widget.hpp"  // IWYU pragma: associated

#include <iosfwd>
#include <type_traits>

#include "internal/api/session/UI.hpp"
#include "opentxs/Types.hpp"
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
    auto counter = std::ptrdiff_t{-1};

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
    const Identifier& id,
    const SimpleCallback& cb) noexcept
    : api_(api)
    , widget_id_(id)
    , ui_(api_.UI())
    , callbacks_()
    , listeners_()
{
    if (cb) { SetCallback(cb); }
}

auto Widget::ClearCallbacks() const noexcept -> void
{
    ui_.Internal().ClearUICallbacks(widget_id_);
}

auto Widget::SetCallback(SimpleCallback cb) const noexcept -> void
{
    ui_.Internal().RegisterUICallback(WidgetID(), cb);
}

auto Widget::setup_listeners(const ListenerDefinitions& definitions) noexcept
    -> void
{
    for (const auto& [endpoint, functor] : definitions) {
        const auto* copy{functor};
        auto& nextCallback =
            callbacks_.emplace_back(network::zeromq::ListenCallback::Factory(
                [=](const Message& message) -> void {
                    (*copy)(this, message);
                }));
        auto& socket = listeners_.emplace_back(
            api_.Network().ZeroMQ().SubscribeSocket(nextCallback.get()));
        const auto listening = socket->Start(endpoint);

        OT_ASSERT(listening)
    }
}

auto Widget::UpdateNotify() const noexcept -> void
{
    ui_.Internal().ActivateUICallback(WidgetID());
}

Widget::~Widget() { ClearCallbacks(); }
}  // namespace opentxs::ui::implementation
