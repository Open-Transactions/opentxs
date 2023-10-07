// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/mempool/Imp.hpp"  // IWYU pragma: associated

#include <chrono>
#include <utility>

#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::mempool
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , mempool::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::mempool,
          std::move(checksum),
          alloc)
{
}

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ReadView&,
    allocator_type alloc) noexcept(false)
    : Message(api, chain, std::move(checksum), alloc)
{
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , mempool::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
{
}
}  // namespace opentxs::network::blockchain::bitcoin::message::mempool
