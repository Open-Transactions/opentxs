// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/common/PeerRequestListener.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>

namespace ottest
{
PeerRequestListener::PeerRequestListener(
    const opentxs::api::Session& requestor,
    const opentxs::api::Session& responder,
    Callback cb,
    std::shared_ptr<Promise> promise) noexcept
    : future_(promise->get_future())
{
    using namespace opentxs::network::zeromq;
    using enum socket::Direction;
    requestor.Network().ZeroMQ().SpawnActor(
        requestor,
        "requestor",
        DefaultStartup(),
        DefaultShutdown(),
        [promise, &requestor](auto, auto type, auto&& message, auto alloc) {
            const auto body = message.Payload();
            promise->set_value(requestor.Factory().PeerReply(body[5], alloc));

            return opentxs::network::zeromq::actor::Replies{alloc.work_};
        },
        DefaultStateMachine(),
        {
            {requestor.Endpoints().Shutdown(), Connect},
            {requestor.Endpoints().PeerReply(), Connect},
        });
    responder.Network().ZeroMQ().SpawnActor(
        responder,
        "responder",
        DefaultStartup(),
        DefaultShutdown(),
        [cb, &responder](auto, auto type, auto&& message, auto alloc) {
            const auto body = message.Payload();
            auto request = responder.Factory().PeerRequest(body[5], alloc);
            std::invoke(cb, std::move(request));
            auto out = opentxs::network::zeromq::actor::Replies{alloc.work_};
            out.clear();
            auto& [_, messages] = out.emplace_back(
                opentxs::network::zeromq::actor::LoopbackIndex,
                opentxs::Vector<opentxs::network::zeromq::Message>{
                    alloc.work_});
            messages.emplace_back([&] {
                auto m = opentxs::network::zeromq::Message{};
                m.StartBody();
                m.AddFrame(opentxs::WorkType::Shutdown);

                return m;
            }());

            return out;
        },
        DefaultStateMachine(),
        {
            {responder.Endpoints().Shutdown(), Connect},
            {responder.Endpoints().PeerRequest(), Connect},
        });
}

PeerRequestListener::PeerRequestListener(
    const opentxs::api::Session& requestor,
    const opentxs::api::Session& responder,
    Callback cb) noexcept
    : PeerRequestListener(
          requestor,
          responder,
          std::move(cb),
          std::make_shared<Promise>())
{
}

PeerRequestListener::~PeerRequestListener() = default;
}  // namespace ottest
