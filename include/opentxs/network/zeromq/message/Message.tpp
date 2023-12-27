// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>
#include <utility>

#include "opentxs/Export.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Envelope;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
template <
    typename Tag,
    typename = std::enable_if_t<std::is_trivially_copyable_v<Tag>>>
auto tagged_message(const Tag& tag, bool addDelimiter) noexcept -> Message
{
    return tagged_message(&tag, sizeof(tag), addDelimiter);
}
template <
    typename Tag,
    typename = std::enable_if_t<std::is_trivially_copyable_v<Tag>>>
auto tagged_reply_to_message(
    const Envelope& envelope,
    const Tag& tag,
    bool addDelimiter) noexcept -> Message
{
    return reply_to_message(envelope, &tag, sizeof(tag), addDelimiter);
}
template <
    typename Tag,
    typename = std::enable_if_t<std::is_trivially_copyable_v<Tag>>>
auto tagged_reply_to_message(
    Envelope&& envelope,
    const Tag& tag,
    bool addDelimiter) noexcept -> Message
{
    return reply_to_message(
        std::move(envelope), &tag, sizeof(tag), addDelimiter);
}
template <
    typename Tag,
    typename = std::enable_if_t<std::is_trivially_copyable_v<Tag>>>
auto tagged_reply_to_message(const Message& request, const Tag& tag) noexcept
    -> Message
{
    return reply_to_message(request, &tag, sizeof(tag));
}
}  // namespace opentxs::network::zeromq
