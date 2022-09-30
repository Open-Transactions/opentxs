// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/Export.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"

namespace opentxs::network::zeromq
{
template <
    typename Tag,
    typename = std::enable_if_t<std::is_trivially_copyable<Tag>::value>>
auto tagged_message(const Tag& tag) noexcept -> Message
{
    return tagged_message(&tag, sizeof(tag));
}
template <
    typename Tag,
    typename = std::enable_if_t<std::is_trivially_copyable<Tag>::value>>
auto tagged_reply_to_connection(
    const ReadView connectionID,
    const Tag& tag) noexcept -> Message
{
    return reply_to_connection(connectionID, &tag, sizeof(tag));
}
template <
    typename Tag,
    typename = std::enable_if_t<std::is_trivially_copyable<Tag>::value>>
auto tagged_reply_to_message(const Message& request, const Tag& tag) noexcept
    -> Message
{
    return reply_to_message(request, &tag, sizeof(tag));
}
}  // namespace opentxs::network::zeromq
