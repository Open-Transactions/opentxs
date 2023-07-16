// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/blockchain/bitcoin/message/Message.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Transaction;
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
class Tx final : virtual public Message
{
public:
    static auto Blank() noexcept -> Tx&;

    auto Transaction(alloc::Default alloc) const noexcept
        -> opentxs::blockchain::block::Transaction;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Tx(MessagePrivate* imp) noexcept;
    Tx(allocator_type alloc = {}) noexcept;
    Tx(const Tx& rhs, allocator_type alloc = {}) noexcept;
    Tx(Tx&& rhs) noexcept;
    Tx(Tx&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Tx& rhs) noexcept -> Tx&;
    auto operator=(Tx&& rhs) noexcept -> Tx&;

    ~Tx() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
