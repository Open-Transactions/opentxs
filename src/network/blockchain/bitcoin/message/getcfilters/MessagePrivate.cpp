// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/getcfilters/MessagePrivate.hpp"

#include <utility>

#include "opentxs/blockchain/block/Hash.hpp"

namespace opentxs::network::blockchain::bitcoin::message::getcfilters
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

auto MessagePrivate::Start() const noexcept
    -> opentxs::blockchain::block::Height
{
    return {};
}

auto MessagePrivate::Stop() const noexcept
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
}  // namespace opentxs::network::blockchain::bitcoin::message::getcfilters