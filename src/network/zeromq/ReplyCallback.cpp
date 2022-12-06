// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/ReplyCallback.hpp"  // IWYU pragma: associated

#include <functional>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Writer.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::ReplyCallback>;

namespace opentxs::network::zeromq
{
auto ReplyCallback::Factory(zeromq::ReplyCallback::ReceiveCallback callback)
    -> OTZMQReplyCallback
{
    return OTZMQReplyCallback(new implementation::ReplyCallback(callback));
}

auto ReplyCallback::Factory() -> OTZMQReplyCallback
{
    return OTZMQReplyCallback(
        new implementation::ReplyCallback([](auto&&) { return Message{}; }));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
ReplyCallback::ReplyCallback(zeromq::ReplyCallback::ReceiveCallback callback)
    : execute_lock_()
    , callback_lock_()
    , callback_(callback)
{
    OT_ASSERT(callback_);
}

auto ReplyCallback::clone() const -> ReplyCallback*
{
    return new ReplyCallback(callback_);
}

auto ReplyCallback::Deactivate() const noexcept -> void
{
    using Message = zeromq::Message;
    static const auto null = [](Message&&) -> Message { return {}; };
    auto rlock = rLock{execute_lock_};
    auto lock = Lock{callback_lock_};
    callback_ = null;
}

auto ReplyCallback::Process(zeromq::Message&& message) const noexcept -> Message
{
    auto rlock = rLock{execute_lock_};
    auto cb = [this] {
        auto lock = Lock{callback_lock_};

        return callback_;
    }();

    return cb(std::move(message));
}

ReplyCallback::~ReplyCallback() = default;
}  // namespace opentxs::network::zeromq::implementation
