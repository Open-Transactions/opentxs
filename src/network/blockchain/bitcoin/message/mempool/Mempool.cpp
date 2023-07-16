// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Mempool.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Mempool::Mempool(allocator_type alloc) noexcept
    : Mempool(MessagePrivate::Blank(alloc))
{
}

Mempool::Mempool(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Mempool::Mempool(const Mempool& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Mempool::Mempool(Mempool&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Mempool::Mempool(Mempool&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Mempool::Blank() noexcept -> Mempool&
{
    static auto blank = Mempool{};

    return blank;
}

auto Mempool::operator=(const Mempool& rhs) noexcept -> Mempool&
{
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Mempool::operator=(Mempool&& rhs) noexcept -> Mempool&
{
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

Mempool::~Mempool() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
