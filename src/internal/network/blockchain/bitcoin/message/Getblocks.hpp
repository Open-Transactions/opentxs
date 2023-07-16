// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <span>

#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
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
class Getblocks final : virtual public Message
{
public:
    static auto Blank() noexcept -> Getblocks&;

    auto get() const noexcept
        -> std::span<const opentxs::blockchain::block::Hash>;
    auto Stop() const noexcept -> const opentxs::blockchain::block::Hash&;
    auto Version() const noexcept -> ProtocolVersionUnsigned;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Getblocks(MessagePrivate* imp) noexcept;
    Getblocks(allocator_type alloc = {}) noexcept;
    Getblocks(const Getblocks& rhs, allocator_type alloc = {}) noexcept;
    Getblocks(Getblocks&& rhs) noexcept;
    Getblocks(Getblocks&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Getblocks& rhs) noexcept -> Getblocks&;
    auto operator=(Getblocks&& rhs) noexcept -> Getblocks&;

    ~Getblocks() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
