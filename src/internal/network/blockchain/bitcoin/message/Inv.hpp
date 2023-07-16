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
namespace bitcoin
{
class Inventory;
}  // namespace bitcoin
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
class Inv final : virtual public Message
{
public:
    using value_type = opentxs::blockchain::bitcoin::Inventory;

    static auto Blank() noexcept -> Inv&;

    auto get() const noexcept -> std::span<const value_type>;

    auto get() noexcept -> std::span<value_type>;
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Inv(MessagePrivate* imp) noexcept;
    Inv(allocator_type alloc = {}) noexcept;
    Inv(const Inv& rhs, allocator_type alloc = {}) noexcept;
    Inv(Inv&& rhs) noexcept;
    Inv(Inv&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Inv& rhs) noexcept -> Inv&;
    auto operator=(Inv&& rhs) noexcept -> Inv&;

    ~Inv() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
