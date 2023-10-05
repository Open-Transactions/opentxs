// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/network/blockchain/StartupPublisher.hpp"  // IWYU pragma: associated

#include "internal/api/session/Endpoints.hpp"
#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::api::network::blockchain
{
StartupPublisher::StartupPublisher(
    const api::session::Endpoints& endpoints,
    const opentxs::network::zeromq::Context& zmq) noexcept
    : zmq_(zmq.Internal())
    , handle_(zmq_.Internal().MakeBatch(
          [] {
              using Type = opentxs::network::zeromq::socket::Type;
              auto out = Vector<Type>{
                  Type::Publish,
                  Type::Pull,
              };

              return out;
          }(),
          "api::network::blockchain::StartupPublisher"))
    , batch_([&]() -> auto& {
        auto& out = handle_.batch_;
        out.listen_callbacks_.emplace_back(Callback::Factory([this](auto&& in) {
            publish_.Send(std::move(in), __FILE__, __LINE__);
        }));

        return out;
    }())
    , cb_(batch_.listen_callbacks_.at(0))
    , publish_([&]() -> auto& {
        auto& out = batch_.sockets_.at(0);
        const auto rc =
            out.Bind(endpoints.Internal().BlockchainStartupPublish().data());

        OT_ASSERT(rc);

        return out;
    }())
    , pull_([&]() -> auto& {
        auto& out = batch_.sockets_.at(1);
        const auto rc =
            out.Bind(endpoints.Internal().BlockchainStartupPull().data());

        OT_ASSERT(rc);

        return out;
    }())
    , thread_(zmq_.Internal().Start(
          batch_.id_,
          {
              {pull_.ID(),
               &pull_,
               [&cb = cb_](auto&& m) { cb.Process(std::move(m)); }},
          }))
{
    OT_ASSERT(nullptr != thread_);

    LogTrace()(OT_PRETTY_CLASS())("using ZMQ batch ")(batch_.id_).Flush();
}

StartupPublisher::~StartupPublisher() { handle_.Release(); }
}  // namespace opentxs::api::network::blockchain
