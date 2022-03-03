// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"

namespace opentxs::network::zeromq::implementation
{
class ReplyCallback : virtual public zeromq::ReplyCallback
{
public:
    auto Deactivate() const noexcept -> void final;
    auto Process(zeromq::Message&& message) const noexcept -> Message override;

    ~ReplyCallback() override;

private:
    friend zeromq::ReplyCallback;

    mutable std::recursive_mutex execute_lock_;
    mutable std::mutex callback_lock_;
    mutable zeromq::ReplyCallback::ReceiveCallback callback_;

    auto clone() const -> ReplyCallback* override;

    ReplyCallback(zeromq::ReplyCallback::ReceiveCallback callback);
    ReplyCallback() = delete;
    ReplyCallback(const ReplyCallback&) = delete;
    ReplyCallback(ReplyCallback&&) = delete;
    auto operator=(const ReplyCallback&) -> ReplyCallback& = delete;
    auto operator=(ReplyCallback&&) -> ReplyCallback& = delete;
};
}  // namespace opentxs::network::zeromq::implementation
