// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/block/MessagePrivate.hpp"

#include <utility>

#include "internal/util/PMR.hpp"

namespace opentxs::network::blockchain::bitcoin::message::block
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

auto MessagePrivate::Blank(allocator_type alloc) noexcept -> MessagePrivate*
{
    return pmr::default_construct<MessagePrivate>(alloc);
}

auto MessagePrivate::clone(allocator_type alloc) const noexcept
    -> internal::MessagePrivate*
{
    return pmr::clone(this, {alloc});
}

auto MessagePrivate::get() const noexcept -> ReadView { return {}; }

auto MessagePrivate::get_deleter() noexcept -> delete_function
{
    return pmr::make_deleter(this);
}

MessagePrivate::~MessagePrivate() { Reset(self_); }
}  // namespace opentxs::network::blockchain::bitcoin::message::block
