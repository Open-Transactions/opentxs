// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "network/zeromq/PairEventListener.hpp"  // IWYU pragma: associated

#include <sstream>

#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/PairEventCallback.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

#define PAIR_EVENT_ENDPOINT_PATH "pairevent"
#define PAIR_EVENT_ENDPOINT_VERSION 1

namespace opentxs::network::zeromq::implementation
{
PairEventListener::PairEventListener(
    const zeromq::Context& context,
    const zeromq::PairEventCallback& callback,
    const int instance,
    const std::string_view threadname)
    : ot_super(context, callback, threadname)
    , instance_(instance)
{
    const auto endpoint = MakeDeterministicInproc(
        PAIR_EVENT_ENDPOINT_PATH, instance, PAIR_EVENT_ENDPOINT_VERSION);
    const bool started = Start(endpoint);

    OT_ASSERT(started);

    LogVerbose()(OT_PRETTY_CLASS())("listening on ")(endpoint).Flush();
}

auto PairEventListener::clone() const noexcept -> PairEventListener*
{
    return new PairEventListener(
        context_,
        dynamic_cast<const zeromq::PairEventCallback&>(callback_),
        instance_,
        [&](const auto& threadname) {
            auto out = std::stringstream{};
            out << threadname;
            out << std::to_string(instance_);
            return out.str();
        }(thread_name_));
}
}  // namespace opentxs::network::zeromq::implementation
