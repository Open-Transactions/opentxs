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
class Hash;
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
class Getheaders final : virtual public Message
{
public:
    using value_type = opentxs::blockchain::block::Hash;

    static auto Blank() noexcept -> Getheaders&;

    auto get() const noexcept -> std::span<const value_type>;
    auto Stop() const noexcept -> const opentxs::blockchain::block::Hash&;

    auto get() noexcept -> std::span<value_type>;
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Getheaders(MessagePrivate* imp) noexcept;
    Getheaders(allocator_type alloc = {}) noexcept;
    Getheaders(const Getheaders& rhs, allocator_type alloc = {}) noexcept;
    Getheaders(Getheaders&& rhs) noexcept;
    Getheaders(Getheaders&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Getheaders& rhs) noexcept -> Getheaders&;
    auto operator=(Getheaders&& rhs) noexcept -> Getheaders&;

    ~Getheaders() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
