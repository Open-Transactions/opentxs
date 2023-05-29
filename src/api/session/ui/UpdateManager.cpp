// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/session/ui/UpdateManager.hpp"  // IWYU pragma: associated

#include <functional>
#include <mutex>
#include <span>
#include <type_traits>  // IWYU pragma: keep
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::session::ui
{
struct UpdateManager::Imp {
    auto ActivateUICallback(const identifier::Generic& id) const noexcept
        -> void
    {
        pipeline_.Push([&] {
            auto out = opentxs::network::zeromq::Message{};
            out.StartBody();
            out.AddFrame(id);

            return out;
        }());
    }
    auto ClearUICallbacks(const identifier::Generic& id) const noexcept -> void
    {
        if (id.empty()) {
            LogError()(OT_PRETTY_CLASS())("Invalid widget id").Flush();

            return;
        } else {
            LogTrace()(OT_PRETTY_CLASS())("Clearing callback for widget ")(
                id, api_.Crypto())
                .Flush();
        }

        auto lock = Lock{lock_};
        map_.erase(id);
    }
    auto RegisterUICallback(
        const identifier::Generic& id,
        const SimpleCallback& cb) const noexcept -> void
    {
        if (id.empty()) {
            LogError()(OT_PRETTY_CLASS())("Invalid widget id").Flush();

            return;
        } else {
            LogTrace()(OT_PRETTY_CLASS())("Registering callback for widget ")(
                id, api_.Crypto())
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
        , publisher_(api.Network().ZeroMQ().Internal().PublishSocket())
        , pipeline_(api.Network().ZeroMQ().Internal().Pipeline(
              [this](auto&& in) { pipeline(std::move(in)); },
              "UpdateManager"))
    {
        publisher_->Start(api_.Endpoints().WidgetUpdate().data());
        LogTrace()(OT_PRETTY_CLASS())("using ZMQ batch ")(pipeline_.BatchID())
            .Flush();
    }

private:
    const api::session::Client& api_;
    mutable std::mutex lock_;
    mutable UnallocatedMap<
        identifier::Generic,
        UnallocatedVector<SimpleCallback>>
        map_;
    OTZMQPublishSocket publisher_;
    opentxs::network::zeromq::Pipeline pipeline_;

    auto pipeline(zmq::Message&& in) noexcept -> void
    {
        const auto body = in.Payload();

        OT_ASSERT(0_uz < body.size());

        const auto& idFrame = body[0];

        OT_ASSERT(0_uz < idFrame.size());

        const auto id = api_.Factory().IdentifierFromHash(idFrame.Bytes());
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
                WorkType::UIModelUpdated, true);
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

auto UpdateManager::ActivateUICallback(
    const identifier::Generic& widget) const noexcept -> void
{
    imp_->ActivateUICallback(widget);
}

auto UpdateManager::ClearUICallbacks(
    const identifier::Generic& widget) const noexcept -> void
{
    imp_->ClearUICallbacks(widget);
}

auto UpdateManager::RegisterUICallback(
    const identifier::Generic& widget,
    const SimpleCallback& cb) const noexcept -> void
{
    imp_->RegisterUICallback(widget, cb);
}

UpdateManager::~UpdateManager() = default;
}  // namespace opentxs::api::session::ui
