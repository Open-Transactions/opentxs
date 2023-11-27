// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>

#include "opentxs/api/network/ZAPPrivate.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <memory>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/network/zap/Types.internal.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::api::network
{
using namespace std::literals;

ZAPPrivate::ZAPPrivate(const opentxs::network::zeromq::Context& context)
    : ZAPPrivate(context, context.Internal().PreallocateBatch())
{
}

ZAPPrivate::ZAPPrivate(
    const opentxs::network::zeromq::Context& context,
    opentxs::network::zeromq::BatchID batch)
    : context_(context)
    , policy_(ZAPPolicy::Accept)
    , domains_(context_.Internal().Alloc(batch))
    , handle_([&] {
        using enum opentxs::network::zeromq::socket::Type;

        return context_.Internal().MakeBatch(
            batch,
            {
                Router,
                Router,
            },
            "ZAP dispatcher");
    }())
    , batch_([&]() -> auto& {
        using Callback = opentxs::network::zeromq::ListenCallback;
        auto& out = handle_.batch_;
        out.listen_callbacks_.emplace_back(Callback::Factory(
            [this](auto&& in) { process_upstream(std::move(in)); }));
        out.listen_callbacks_.emplace_back(Callback::Factory(
            [this](auto&& in) { process_downstream(std::move(in)); }));

        return out;
    }())
    , upstream_callback_(batch_.listen_callbacks_.at(0))
    , downstream_callback_(batch_.listen_callbacks_.at(1))
    , upstream_([&]() -> auto& {
        auto& out = batch_.sockets_.at(0);
        constexpr auto endpoint = "inproc://zeromq.zap.01";
        const auto rc = out.Bind(endpoint);

        assert_true(rc);

        return out;
    }())
    , downstream_(batch_.sockets_.at(1))
    , thread_(context_.Internal().Start(
          batch_.id_,
          {
              {upstream_.ID(),
               &upstream_,
               [id = upstream_.ID(), &cb = upstream_callback_](auto&& m) {
                   cb.Process(std::move(m));
               }},
              {downstream_.ID(),
               &downstream_,
               [id = downstream_.ID(), &cb = downstream_callback_](auto&& m) {
                   cb.Process(std::move(m));
               }},
          }))
{
    assert_false(nullptr == thread_);
}

auto ZAPPrivate::process_downstream(
    opentxs::network::zeromq::Message&& msg) noexcept -> void
{
    const auto domain = msg.ExtractFront();

    if (domains_.lock()->contains(domain.Bytes())) {
        upstream_.Send(std::move(msg));
    }
}

auto ZAPPrivate::process_upstream(
    opentxs::network::zeromq::Message&& msg) noexcept -> void
{
    auto payload = msg.Payload();
    auto envelope = std::move(msg).Envelope();

    if (payload.size() < 6_uz) {
        LogError()()("received invalid ZAP message");

        return;
    }

    if (payload[0].Bytes() != "1.0") {
        LogError()()("invalid ZAP version");

        return;
    }

    const auto& domain = payload[2];
    auto handle = domains_.lock();
    auto& domains = *handle;

    if (auto i = domains.find(domain.Bytes()); domains.end() != i) {
        auto& [endpoint, connected] = i->second;

        if (false == connected) {
            connected = downstream_.Connect(endpoint.c_str());
        }

        try {
            if (false == connected) {
                throw std::runtime_error{
                    "failed to connect to endpoint "s.append(endpoint)
                        .append(" for domain ")
                        .append(domain.Bytes())};
            }

            if (downstream_.SendDeferred([&] {
                    auto out = opentxs::network::zeromq::Message{};
                    auto copy = [&](const auto& frame) { out.AddFrame(frame); };
                    out.AddFrame(domain);
                    std::ranges::for_each(envelope.get(), copy);
                    out.StartBody();
                    std::ranges::for_each(payload, copy);

                    return out;
                }())) {
                // NOTE reply will be relayed to upstream_ when it is received

                return;
            } else {
                throw std::runtime_error{
                    "failed to transmit ZAP request to endpoint "s
                        .append(endpoint)
                        .append(" for domain ")
                        .append(domain.Bytes())};
            }
        } catch (const std::exception& e) {
            LogError()()(e.what()).Flush();
            using enum zap::Status;
            send_reply(std::move(envelope), payload, TemporaryError);
        }
    } else {
        send_reply(std::move(envelope), payload, [&] {
            using enum ZAPPolicy;
            using enum zap::Status;

            if (Accept == policy_.load()) {

                return Success;
            } else {

                return AuthFailure;
            }
        }());
    }
}

auto ZAPPrivate::RegisterDomain(
    std::string_view domain,
    std::string_view handler) const noexcept -> bool
{
    auto handle = domains_.lock();
    auto& map = *handle;

    if (auto i = map.find(domain); map.end() == i) {
        map.try_emplace(domain, handler, false);

        return true;
    } else {
        const auto& [endpoint, _] = i->second;

        if (handler == endpoint) {

            return true;
        } else {
            LogError()()("domain ")(domain)(" already bound to ")(endpoint)
                .Flush();

            return false;
        }
    }
}

auto ZAPPrivate::send_reply(
    opentxs::network::zeromq::Envelope&& envelope,
    std::span<opentxs::network::zeromq::Frame> payload,
    zap::Status answer) noexcept -> void
{
    upstream_.SendDeferred([&] {
        const auto result =
            static_cast<std::underlying_type_t<zap::Status>>(answer);
        auto out = reply_to_message(std::move(envelope), true);
        out.AddFrame(std::move(payload[0]));
        out.AddFrame(std::move(payload[1]));
        out.AddFrame(std::to_string(result));
        out.AddFrame();
        out.AddFrame();
        out.AddFrame();

        return out;
    }());
}

auto ZAPPrivate::SetDefaultPolicy(ZAPPolicy policy) const noexcept -> bool
{
    policy_.store(policy);

    return true;
}

ZAPPrivate::~ZAPPrivate() { handle_.Release(); }
}  // namespace opentxs::api::network
