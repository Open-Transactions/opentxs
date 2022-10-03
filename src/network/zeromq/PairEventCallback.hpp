// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/PairEventCallback.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::implementation
{
class PairEventCallback final : virtual public zeromq::PairEventCallback
{
public:
    auto Deactivate() const noexcept -> void final;
    auto Process(zeromq::Message&& message) const noexcept -> void final;

    auto Replace(ListenCallback::ReceiveCallback callback) noexcept
        -> void final
    {
    }

    PairEventCallback() = delete;
    PairEventCallback(const PairEventCallback&) = delete;
    PairEventCallback(PairEventCallback&&) = delete;
    auto operator=(const PairEventCallback&) -> PairEventCallback& = delete;
    auto operator=(PairEventCallback&&) -> PairEventCallback& = delete;

    ~PairEventCallback() final;

private:
    friend zeromq::PairEventCallback;

    mutable std::recursive_mutex execute_lock_;
    mutable std::mutex callback_lock_;
    mutable zeromq::PairEventCallback::ReceiveCallback callback_;

    auto clone() const -> PairEventCallback* final;

    PairEventCallback(zeromq::PairEventCallback::ReceiveCallback callback);
};
}  // namespace opentxs::network::zeromq::implementation
