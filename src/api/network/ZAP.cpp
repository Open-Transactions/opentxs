// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::network::ZAP

#include "api/network/ZAP.hpp"  // IWYU pragma: associated

#include <span>
#include <utility>

#include "2_Factory.hpp"
#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/network/ZAP.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Factory::ZAP(const network::zeromq::Context& context) -> api::network::ZAP*
{
    return new api::network::imp::ZAP(context);
}
}  // namespace opentxs

namespace opentxs::api::network::imp
{
ZAP::ZAP(const opentxs::network::zeromq::Context& context)
    : context_(context)
    , handle_([&] {
        using enum opentxs::network::zeromq::socket::Type;

        return context_.Internal().MakeBatch(
            {
                Router,
            },
            "api::network::imp::ZAP");
    }())
    , batch_([&]() -> auto& {
        using Callback = opentxs::network::zeromq::ListenCallback;
        auto& out = handle_.batch_;
        out.listen_callbacks_.emplace_back(
            Callback::Factory([this](auto&& in) { process(std::move(in)); }));

        return out;
    }())
    , callback_(batch_.listen_callbacks_.at(0))
    , socket_([&]() -> auto& {
        auto& out = batch_.sockets_.at(0);
        constexpr auto endpoint = "inproc://zeromq.zap.01";
        const auto rc = out.Bind(endpoint);

        OT_ASSERT(rc);

        return out;
    }())
    , thread_(context_.Internal().Start(
          batch_.id_,
          {
              {socket_.ID(),
               &socket_,
               [id = socket_.ID(), &cb = callback_](auto&& m) {
                   cb.Process(std::move(m));
               }},
          }))
{
    OT_ASSERT(nullptr != thread_);
}

auto ZAP::process(opentxs::network::zeromq::Message&& msg) const noexcept
    -> void
{
    auto payload = msg.Payload();
    auto envelope = std::move(msg).Envelope();

    if (payload.size() < 6_uz) {
        LogError()(OT_PRETTY_CLASS())("received invalid ZAP message");

        return;
    }

    if (payload[0].Bytes() != "1.0") {
        LogError()(OT_PRETTY_CLASS())("invalid ZAP version");

        return;
    }

    socket_.SendDeferred(
        [&] {
            auto out = reply_to_message(std::move(envelope), true);
            out.AddFrame(std::move(payload[0]));
            out.AddFrame(std::move(payload[1]));
            out.AddFrame("200");
            out.AddFrame();
            out.AddFrame();
            out.AddFrame();

            return out;
        }(),
        __FILE__,
        __LINE__);
}

auto ZAP::RegisterDomain(
    const std::string_view domain,
    const opentxs::network::zeromq::zap::ReceiveCallback& callback) const
    -> bool
{
    return false;  // TODO
}

auto ZAP::SetDefaultPolicy(
    const opentxs::network::zeromq::zap::Policy policy) const -> bool
{
    return false;  // TODO
}

ZAP::~ZAP() { handle_.Release(); }
}  // namespace opentxs::api::network::imp
