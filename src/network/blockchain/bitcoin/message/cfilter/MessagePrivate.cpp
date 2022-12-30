// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/cfilter/MessagePrivate.hpp"

#include <utility>

#include "opentxs/blockchain/block/Hash.hpp"

namespace opentxs::network::blockchain::bitcoin::message::cfilter
{
MessagePrivate::MessagePrivate(allocator_type alloc) noexcept
    : internal::MessagePrivate(std::move(alloc))
    , self_(this)
{
}

MessagePrivate::MessagePrivate(
    const MessagePrivate& rhs,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, std::move(alloc))
    , self_(this)
{
}

auto MessagePrivate::Bits() const noexcept -> std::uint8_t { return {}; }

auto MessagePrivate::ElementCount() const noexcept -> std::uint32_t
{
    return {};
}

auto MessagePrivate::FPRate() const noexcept -> std::uint32_t { return {}; }

auto MessagePrivate::Filter() const noexcept -> ReadView { return {}; }

auto MessagePrivate::Hash() const noexcept
    -> const opentxs::blockchain::block::Hash&
{
    static const auto blank = opentxs::blockchain::block::Hash{};

    return blank;
}

auto MessagePrivate::Type() const noexcept -> opentxs::blockchain::cfilter::Type
{
    return {};
}

MessagePrivate::~MessagePrivate() { Reset(self_); }
}  // namespace opentxs::network::blockchain::bitcoin::message::cfilter
