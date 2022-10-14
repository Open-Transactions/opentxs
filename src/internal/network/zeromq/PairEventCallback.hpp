// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class PairEventCallback;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class PairEvent;
}  // namespace proto

using OTZMQPairEventCallback = Pimpl<network::zeromq::PairEventCallback>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class PairEventCallback : virtual public ListenCallback
{
public:
    using ReceiveCallback = std::function<void(const proto::PairEvent&)>;

    static auto Factory(ReceiveCallback callback) -> OTZMQPairEventCallback;

    PairEventCallback(const PairEventCallback&) = delete;
    PairEventCallback(PairEventCallback&&) = delete;
    auto operator=(const PairEventCallback&) -> PairEventCallback& = delete;
    auto operator=(PairEventCallback&&) -> PairEventCallback& = delete;

    ~PairEventCallback() override = default;

protected:
    PairEventCallback() = default;

private:
    friend OTZMQPairEventCallback;

#ifndef _WIN32
    auto clone() const -> PairEventCallback* override = 0;
#endif
};
}  // namespace opentxs::network::zeromq
