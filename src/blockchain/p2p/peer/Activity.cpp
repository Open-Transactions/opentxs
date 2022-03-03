// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "blockchain/p2p/peer/Activity.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>

#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::p2p::peer
{
Activity::Activity(
    const api::Session& api,
    const network::zeromq::Pipeline& pipeline) noexcept
    : pipeline_(pipeline)
    , lock_()
    , disconnect_(api.Network().Asio().Internal().GetTimer())
    , ping_(api.Network().Asio().Internal().GetTimer())
    , activity_(Clock::now())
{
    reset_disconnect();
    reset_ping();
}

auto Activity::Bump() noexcept -> void
{
    {
        auto lock = Lock{lock_};
        activity_ = Clock::now();
    }

    reset_disconnect();
    reset_ping();
}

auto Activity::get() const noexcept -> Time
{
    auto lock = Lock{lock_};

    return activity_;
}

auto Activity::reset_disconnect() noexcept -> void
{
    disconnect_.SetRelative(activity_timeout_);
    disconnect_.Wait([this](const auto& error) {
        if (error) {
            if (boost::system::errc::operation_canceled != error.value()) {
                LogError()(OT_PRETTY_CLASS())(error).Flush();
            }
        } else {
            using Task = node::internal::PeerManager::Task;
            pipeline_.Push(MakeWork(Task::ActivityTimeout));
        }
    });
}

auto Activity::reset_ping() noexcept -> void
{
    ping_.SetRelative(ping_interval_);
    ping_.Wait([this](const auto& error) {
        if (error) {
            if (boost::system::errc::operation_canceled != error.value()) {
                LogError()(OT_PRETTY_CLASS())(error).Flush();
            }
        } else {
            using Task = node::internal::PeerManager::Task;
            pipeline_.Push(MakeWork(Task::NeedPing));
            reset_ping();
        }
    });
}
}  // namespace opentxs::blockchain::p2p::peer
