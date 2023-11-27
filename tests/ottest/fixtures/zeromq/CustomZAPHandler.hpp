// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <string_view>

#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Router.hpp"

namespace ottest
{
class CustomZAPHandler
{
public:
    CustomZAPHandler(
        const opentxs::api::Context& ot,
        std::string_view domain,
        std::string_view endpoint,
        bool accept) noexcept;

    ~CustomZAPHandler() = default;

private:
    const bool accept_;
    opentxs::OTZMQListenCallback cb_;
    opentxs::OTZMQRouterSocket socket_;

    auto process(opentxs::network::zeromq::Message&& msg) noexcept -> void;
};
}  // namespace ottest
