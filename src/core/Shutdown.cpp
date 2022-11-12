// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"       // IWYU pragma: associated
#include "core/Shutdown.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>  // IWYU pragma: keep
#include <chrono>
#include <ratio>

#include "internal/api/network/Asio.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::internal
{
using namespace std::literals;

ShutdownSender::ShutdownSender(
    const api::network::Asio& asio,
    const network::zeromq::Context& zmq,
    std::string_view endpoint,
    std::string_view name) noexcept
    : endpoint_(endpoint)
    , name_(name)
    , activated_(false)
    , socket_(zmq.Internal().PublishSocket())
    , repeat_(asio.Internal().GetTimer())
{
    auto init = socket_->SetTimeouts(1s, 10s, 0s);

    OT_ASSERT(init);

    init = socket_->Start(endpoint_);

    OT_ASSERT(init);
}

auto ShutdownSender::Activate() noexcept -> void
{
    LogInsane()(OT_PRETTY_CLASS())(name_).Flush();
    activated_ = true;
    socket_->Send([&] {
        auto work = MakeWork(WorkType::Shutdown);
        work.AddFrame("shutdown");

        return work;
    }());
    repeat_.SetRelative(1s);
    repeat_.Wait([this](const auto& ec) {
        if (false == ec.operator bool()) { Activate(); }
    });
}

auto ShutdownSender::Close() noexcept -> void
{
    repeat_.Cancel();
    socket_->Close();
}

ShutdownSender::~ShutdownSender()
{
    if (false == activated_) { Activate(); }

    Close();
}
}  // namespace opentxs::internal
