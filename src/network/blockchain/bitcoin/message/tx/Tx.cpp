// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Tx.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/tx/MessagePrivate.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Tx::Tx(allocator_type alloc) noexcept
    : Tx(MessagePrivate::Blank(alloc))
{
}

Tx::Tx(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Tx::Tx(const Tx& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Tx::Tx(Tx&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Tx::Tx(Tx&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Tx::Blank() noexcept -> Tx&
{
    static auto blank = Tx{};

    return blank;
}

auto Tx::Transaction(alloc::Default alloc) const noexcept
    -> opentxs::blockchain::block::Transaction
{
    return imp_->asTxPrivate()->Transaction(alloc);
}

auto Tx::operator=(const Tx& rhs) noexcept -> Tx&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Tx::operator=(Tx&& rhs) noexcept -> Tx&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

Tx::~Tx() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
