// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/Types.hpp"

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
class Block final : virtual public Message
{
public:
    static auto Blank() noexcept -> Block&;

    auto get() const noexcept -> ReadView;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Block(MessagePrivate* imp) noexcept;
    Block(allocator_type alloc = {}) noexcept;
    Block(const Block& rhs, allocator_type alloc = {}) noexcept;
    Block(Block&& rhs) noexcept;
    Block(Block&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Block& rhs) noexcept -> Block&;
    auto operator=(Block&& rhs) noexcept -> Block&;

    ~Block() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
