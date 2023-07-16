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

class Address;
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin::message::internal
{
class Addr2 final : virtual public Message
{
public:
    using value_type = network::blockchain::Address;

    static auto Blank() noexcept -> Addr2&;

    auto get() const noexcept -> std::span<const value_type>;

    auto get() noexcept -> std::span<value_type>;
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Addr2(MessagePrivate* imp) noexcept;
    Addr2(allocator_type alloc = {}) noexcept;
    Addr2(const Addr2& rhs, allocator_type alloc = {}) noexcept;
    Addr2(Addr2&& rhs) noexcept;
    Addr2(Addr2&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Addr2& rhs) noexcept -> Addr2&;
    auto operator=(Addr2&& rhs) noexcept -> Addr2&;

    ~Addr2() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
