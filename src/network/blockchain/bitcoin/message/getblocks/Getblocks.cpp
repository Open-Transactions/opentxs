// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/bitcoin/message/Getblocks.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/PMR.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "network/blockchain/bitcoin/message/getblocks/MessagePrivate.hpp"

namespace opentxs::network::blockchain::bitcoin::message::internal
{
Getblocks::Getblocks(allocator_type alloc) noexcept
    : Getblocks(MessagePrivate::Blank(alloc))
{
}

Getblocks::Getblocks(MessagePrivate* imp) noexcept
    : Message(std::move(imp))
{
}

Getblocks::Getblocks(const Getblocks& rhs, allocator_type alloc) noexcept
    : Message(rhs, alloc)
{
}

Getblocks::Getblocks(Getblocks&& rhs) noexcept
    : Message(std::move(rhs))
{
}

Getblocks::Getblocks(Getblocks&& rhs, allocator_type alloc) noexcept
    : Message(std::move(rhs), alloc)
{
}

auto Getblocks::Blank() noexcept -> Getblocks&
{
    static auto blank = Getblocks{};

    return blank;
}

auto Getblocks::get() const noexcept
    -> std::span<const opentxs::blockchain::block::Hash>
{
    return imp_->asGetblocksPrivate()->get();
}

auto Getblocks::operator=(const Getblocks& rhs) noexcept -> Getblocks&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Message>(*this, rhs);
}

auto Getblocks::operator=(Getblocks&& rhs) noexcept -> Getblocks&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Message>(*this, std::move(rhs));
}

auto Getblocks::Stop() const noexcept -> const opentxs::blockchain::block::Hash&
{
    return imp_->asGetblocksPrivate()->Stop();
}

auto Getblocks::Version() const noexcept -> ProtocolVersionUnsigned
{
    return imp_->asGetblocksPrivate()->Version();
}

Getblocks::~Getblocks() = default;
}  // namespace opentxs::network::blockchain::bitcoin::message::internal
