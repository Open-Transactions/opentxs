// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "api/session/ui/UpdateManager.hpp"  // IWYU pragma: associated

#include <functional>
#include <mutex>
#include <string_view>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::session::ui
{
struct UpdateManager::Imp {
    auto ActivateUICallback(const Identifier& id) const noexcept -> void
    {
        pipeline_.Push([&] {
            auto out = opentxs::network::zeromq::Message{};
            out.StartBody();
            out.AddFrame(id);

            return out;
        }());
    }
    auto ClearUICallbacks(const Identifier& id) const noexcept -> void
    {
        if (id.empty()) {
            LogError()(OT_PRETTY_CLASS())("Invalid widget id").Flush();

            return;
        } else {
            LogTrace()(OT_PRETTY_CLASS())("Clearing callback for widget ")(id)
                .Flush();
        }

        auto lock = Lock{lock_};
        map_.erase(id);
    }
    auto RegisterUICallback(const Identifier& id, const SimpleCallback& cb)
        const noexcept -> void
    {
        if (id.empty()) {
            LogError()(OT_PRETTY_CLASS())("Invalid widget id").Flush();

            return;
        } else {
            LogTrace()(OT_PRETTY_CLASS())("Registering callback for widget ")(
                id)
                .Flush();
        }

        if (cb) {
            auto lock = Lock{lock_};
            map_[id].emplace_back(cb);
        } else {
            LogError()(OT_PRETTY_CLASS())("Invalid callback").Flush();
        }
    }

    Imp(const api::session::Client& api) noexcept
        : api_(api)
        , lock_()
        , map_()
        , publisher_(api.Network().ZeroMQ().PublishSocket())
        , pipeline_(api.Network().ZeroMQ().Internal().Pipeline(
              [this](auto&& in) { pipeline(std::move(in)); }))
    {
        publisher_->Start(api_.Endpoints().WidgetUpdate().data());
        LogTrace()(OT_PRETTY_CLASS())("using ZMQ batch ")(pipeline_.BatchID())
            .Flush();
    }

private:
    const api::session::Client& api_;
    mutable std::mutex lock_;
    mutable UnallocatedMap<OTIdentifier, UnallocatedVector<SimpleCallback>>
        map_;
    OTZMQPublishSocket publisher_;
    opentxs::network::zeromq::Pipeline pipeline_;

    auto pipeline(zmq::Message&& in) noexcept -> void
    {
        const auto body = in.Body();

        OT_ASSERT(0u < body.size());

        auto& idFrame = body.at(0);

        OT_ASSERT(0u < idFrame.size());

        const auto id = api_.Factory().Identifier(idFrame);
        auto lock = Lock{lock_};
        auto it = map_.find(id);

        if (map_.end() == it) { return; }

        const auto& callbacks = it->second;

        for (const auto& cb : callbacks) {
            if (cb) { cb(); }
        }

        const auto& socket = publisher_.get();
        socket.Send([&] {
            auto work = opentxs::network::zeromq::tagged_message(
                WorkType::UIModelUpdated);
            work.AddFrame(std::move(idFrame));

            return work;
        }());
    }
};

UpdateManager::UpdateManager(const api::session::Client& api) noexcept
    : imp_(std::make_unique<Imp>(api))
{
    // WARNING: do not access api_.Wallet() during construction
}

auto UpdateManager::ActivateUICallback(const Identifier& widget) const noexcept
    -> void
{
    imp_->ActivateUICallback(widget);
}

auto UpdateManager::ClearUICallbacks(const Identifier& widget) const noexcept
    -> void
{
    imp_->ClearUICallbacks(widget);
}

auto UpdateManager::RegisterUICallback(
    const Identifier& widget,
    const SimpleCallback& cb) const noexcept -> void
{
    imp_->RegisterUICallback(widget, cb);
}

UpdateManager::~UpdateManager() = default;
}  // namespace opentxs::api::session::ui
