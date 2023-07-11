// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

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
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin::message::internal
{
class Mempool final : virtual public Message
{
public:
    static auto Blank() noexcept -> Mempool&;

    auto get_deleter() noexcept -> delete_function final
    {
        return make_deleter(this);
    }

    Mempool(MessagePrivate* imp) noexcept;
    Mempool(allocator_type alloc = {}) noexcept;
    Mempool(const Mempool& rhs, allocator_type alloc = {}) noexcept;
    Mempool(Mempool&& rhs) noexcept;
    Mempool(Mempool&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Mempool& rhs) noexcept -> Mempool&;
    auto operator=(Mempool&& rhs) noexcept -> Mempool&;

    ~Mempool() final;
};
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
