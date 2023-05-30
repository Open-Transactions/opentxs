// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/Log.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdlib>
#include <memory>
#include <span>
#include <string_view>
#include <utility>

#include "internal/api/Factory.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Pull.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
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
    , socket_(zmq.Internal().PullSocket(
          callback_,
          zmq::socket::Direction::Bind,
          "Logger"))
    , publish_socket_(zmq.Internal().PublishSocket())
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
    const auto body = in.Payload();
    const auto level = body[0].as<int>();
    const auto text = body[1].Bytes();
    const auto thread = body[2].Bytes();
    const auto action = body[3].as<LogAction>();
    const auto console = body[4].as<Console>();

    if (false == text.empty()) {
        print(level, console, text, thread);

        if (publish_) { publish_socket_->Send(std::move(in)); }
    }

    if (LogAction::terminate == action) {
        if (publish_) { Sleep(1s); }

        std::abort();
    }
}

// NOTE: Log::get_suffix defined in src/util/platform
// NOTE: Log::print defined in src/util/platform

Log::~Log() { opentxs::internal::Log::Shutdown(); }
}  // namespace opentxs::api::imp
