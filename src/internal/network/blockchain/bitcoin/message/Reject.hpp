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
class Reject final : virtual public Message
{
public:
    static auto Blank() noexcept -> Reject&;

    Reject(MessagePrivate* imp) noexcept;
    Reject(allocator_type alloc = {}) noexcept;
    Reject(const Reject& rhs, allocator_type alloc = {}) noexcept;
    Reject(Reject&& rhs) noexcept;
    Reject(Reject&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Reject& rhs) noexcept -> Reject&;
    auto operator=(Reject&& rhs) noexcept -> Reject&;

    ~Reject() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
