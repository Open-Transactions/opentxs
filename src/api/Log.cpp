// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "api/Log.hpp"   // IWYU pragma: associated

#include <chrono>
#include <cstdlib>
#include <memory>
#include <string_view>
#include <utility>

#include "internal/api/Factory.hpp"
#include "internal/util/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Time.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto Log(const zmq::Context& zmq, std::string_view endpoint) noexcept
    -> std::unique_ptr<api::internal::Log>
{
    using ReturnType = api::imp::Log;

    return std::make_unique<ReturnType>(zmq, UnallocatedCString{endpoint});
}
}  // namespace opentxs::factory

namespace opentxs::api::imp
{
Log::Log(const zmq::Context& zmq, const UnallocatedCString endpoint)
    : callback_(opentxs::network::zeromq::ListenCallback::Factory(
          [&](auto&& msg) -> void { callback(std::move(msg)); }))
    , socket_(zmq.PullSocket(callback_, zmq::socket::Direction::Bind, "Logger"))
    , publish_socket_(zmq.PublishSocket())
    , publish_{!endpoint.empty()}
{
    auto rc = socket_->Start(opentxs::internal::Log::Endpoint());

    if (false == rc) { std::abort(); }

    if (publish_) {
        rc = publish_socket_->Start(endpoint);

        if (false == rc) { std::abort(); }
    }

    opentxs::internal::Log::Start();
}

auto Log::callback(zmq::Message&& in) noexcept -> void
{
    const auto body = in.Body();
    const auto level = body.at(0).as<int>();
    const auto text = body.at(1).Bytes();
    const auto thread = body.at(2).Bytes();
    const auto action = body.at(3).as<LogAction>();
    const auto console = body.at(4).as<Console>();

    if (false == text.empty()) {
        print(level, console, text, thread);

        if (publish_) { publish_socket_->Send(std::move(in)); }
    }

    if (LogAction::terminate == action) {
        if (publish_) { Sleep(1s); }

        std::abort();
    }
}

Log::~Log() { opentxs::internal::Log::Shutdown(); }
}  // namespace opentxs::api::imp
