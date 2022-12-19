// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/blockchain/bitcoin/message/Message.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace blockchain
{
namespace bitcoin
{
namespace message
{
namespace internal
{
class MessagePrivate;
}  // namespace internal
}  // namespace message
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin::message::internal
{
class Sendaddr2 final : virtual public Message
{
public:
    static auto Blank() noexcept -> Sendaddr2&;

    Sendaddr2(MessagePrivate* imp) noexcept;
    Sendaddr2(allocator_type alloc = {}) noexcept;
    Sendaddr2(const Sendaddr2& rhs, allocator_type alloc = {}) noexcept;
    Sendaddr2(Sendaddr2&& rhs) noexcept;
    Sendaddr2(Sendaddr2&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Sendaddr2& rhs) noexcept -> Sendaddr2&;
    auto operator=(Sendaddr2&& rhs) noexcept -> Sendaddr2&;

    ~Sendaddr2() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
