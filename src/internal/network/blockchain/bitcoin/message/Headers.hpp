// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>

#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/util/PMR.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Header;
}  // namespace block
}  // namespace blockchain

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
class Headers final : virtual public Message
{
public:
    using value_type = opentxs::blockchain::block::Header;

    static auto Blank() noexcept -> Headers&;

    auto get() const noexcept -> std::span<const value_type>;

    auto get() noexcept -> std::span<value_type>;
    auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }

    Headers(MessagePrivate* imp) noexcept;
    Headers(allocator_type alloc = {}) noexcept;
    Headers(const Headers& rhs, allocator_type alloc = {}) noexcept;
    Headers(Headers&& rhs) noexcept;
    Headers(Headers&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Headers& rhs) noexcept -> Headers&;
    auto operator=(Headers&& rhs) noexcept -> Headers&;

    ~Headers() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
