// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/tx/MessagePrivate.hpp"

#include <utility>

#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::network::blockchain::bitcoin::message::tx
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

auto MessagePrivate::Transaction(alloc::Default alloc) const noexcept
    -> opentxs::blockchain::block::Transaction
{
    return opentxs::blockchain::block::Transaction{alloc};
}

MessagePrivate::~MessagePrivate() { Reset(self_); }
}  // namespace opentxs::network::blockchain::bitcoin::message::tx
