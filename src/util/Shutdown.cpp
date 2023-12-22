// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/Shutdown.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>  // IWYU pragma: keep
#include <chrono>

#include "internal/api/network/Asio.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/WorkType.internal.hpp"

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

    assert_true(init);

    init = socket_->Start(endpoint_);

    assert_true(init);
}

auto ShutdownSender::Activate() noexcept -> void
{
    LogInsane()()(name_).Flush();
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
