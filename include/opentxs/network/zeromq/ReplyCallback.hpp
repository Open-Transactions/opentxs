// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <functional>

#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace zeromq
{
class Message;
class ReplyCallback;
}  // namespace zeromq
}  // namespace network

using OTZMQReplyCallback = Pimpl<network::zeromq::ReplyCallback>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class OPENTXS_EXPORT ReplyCallback
{
public:
    using ReceiveCallback = std::function<Message(Message&&)>;

    static auto Factory() -> OTZMQReplyCallback;
    static auto Factory(ReceiveCallback callback) -> OTZMQReplyCallback;

    /// Deactivate will block until Process is no longer executing, unless it is
    /// called by the same thread as the one currently executing the callback.
    virtual auto Deactivate() const noexcept -> void = 0;
    virtual auto Process(Message&& message) const noexcept -> Message = 0;

    virtual ~ReplyCallback() = default;

protected:
    ReplyCallback() = default;

private:
    friend OTZMQReplyCallback;

    virtual auto clone() const -> ReplyCallback* = 0;

    ReplyCallback(const ReplyCallback&) = delete;
    ReplyCallback(ReplyCallback&&) = default;
    auto operator=(const ReplyCallback&) -> ReplyCallback& = delete;
    auto operator=(ReplyCallback&&) -> ReplyCallback& = default;
};
}  // namespace opentxs::network::zeromq
