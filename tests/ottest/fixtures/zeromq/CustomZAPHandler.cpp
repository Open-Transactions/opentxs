// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/zeromq/CustomZAPHandler.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <span>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "ottest/fixtures/zeromq/ZAP.hpp"

namespace ottest
{
CustomZAPHandler::CustomZAPHandler(
    const opentxs::api::Context& ot,
    std::string_view domain,
    std::string_view endpoint,
    bool accept) noexcept
    : accept_(accept)
    , cb_(opentxs::network::zeromq::ListenCallback::Factory(
          [this](auto&& m) { this->process(std::move(m)); }))
    , socket_(ot.ZMQ().Internal().RouterSocket(
          cb_,
          opentxs::network::zeromq::socket::Direction::Bind))
{
    auto rc = socket_->SetIdentity(domain);
    opentxs::assert_true(rc);
    rc = socket_->Start(endpoint);
    opentxs::assert_true(rc);
}

auto CustomZAPHandler::process(opentxs::network::zeromq::Message&& msg) noexcept
    -> void
{
    auto payload = msg.Payload();
    auto envelope = std::move(msg).Envelope();
    socket_->Send([&] {
        auto out = reply_to_message(std::move(envelope), true);
        out.AddFrame(std::move(payload[0]));
        out.AddFrame(std::move(payload[1]));

        if (accept_) {
            out.AddFrame(
                ZAP::success_status_.data(), ZAP::success_status_.size());
        } else {
            out.AddFrame(ZAP::fail_status_.data(), ZAP::fail_status_.size());
        }
        out.AddFrame();
        out.AddFrame();
        out.AddFrame();

        return out;
    }());
}
}  // namespace ottest
