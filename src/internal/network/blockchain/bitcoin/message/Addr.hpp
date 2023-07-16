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
class Addr final : virtual public Message
{
public:
    using value_type = network::blockchain::Address;

    static auto Blank() noexcept -> Addr&;

    auto get() const noexcept -> std::span<const value_type>;

    auto get() noexcept -> std::span<value_type>;
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Addr(MessagePrivate* imp) noexcept;
    Addr(allocator_type alloc = {}) noexcept;
    Addr(const Addr& rhs, allocator_type alloc = {}) noexcept;
    Addr(Addr&& rhs) noexcept;
    Addr(Addr&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Addr& rhs) noexcept -> Addr&;
    auto operator=(Addr&& rhs) noexcept -> Addr&;

    ~Addr() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
