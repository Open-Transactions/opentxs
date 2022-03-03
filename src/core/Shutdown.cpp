// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"       // IWYU pragma: associated
#include "1_Internal.hpp"     // IWYU pragma: associated
#include "core/Shutdown.hpp"  // IWYU pragma: associated

#include <chrono>

#include "internal/util/LogMacros.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::internal
{
ShutdownSender::ShutdownSender(
    const network::zeromq::Context& zmq,
    const UnallocatedCString endpoint) noexcept
    : endpoint_(endpoint)
    , socket_(zmq.PublishSocket())
{
    auto init = socket_->SetTimeouts(1s, 10s, 0s);

    OT_ASSERT(init);

    init = socket_->Start(endpoint_);

    OT_ASSERT(init);
}

auto ShutdownSender::Activate() const noexcept -> void
{
    socket_->Send([&] {
        auto work = network::zeromq::tagged_message(WorkType::Shutdown);
        work.AddFrame("shutdown");

        return work;
    }());
}

auto ShutdownSender::Close() noexcept -> void { socket_->Close(); }

ShutdownSender::~ShutdownSender()
{
    Activate();
    Close();
}

ShutdownReceiver::ShutdownReceiver(
    const network::zeromq::Context& zmq,
    const Endpoints endpoints,
    Callback cb) noexcept
    : promise_()
    , future_(promise_.get_future())
    , callback_(zmq::ListenCallback::Factory([cb, this](auto&&) {
        if (bool(cb)) { cb(this->promise_); }
    }))
    , socket_(zmq.SubscribeSocket(callback_))
{
    auto init{false};

    for (const auto& endpoint : endpoints) {
        init = socket_->Start(endpoint);

        OT_ASSERT(init);
    }
}

auto ShutdownReceiver::Close() noexcept -> void { socket_->Close(); }

ShutdownReceiver::~ShutdownReceiver()
{
    Close();

    try {
        promise_.set_value();
    } catch (...) {
    }
}
}  // namespace opentxs::internal
