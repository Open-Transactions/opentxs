// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/block/Imp.hpp"  // IWYU pragma: associated

#include <chrono>
#include <utility>

#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::block
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ByteArray payload,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , block::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::block,
          std::move(checksum),
          alloc)
    , payload_(std::move(payload), alloc)
{
}

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ReadView& payload,
    allocator_type alloc) noexcept(false)
    : Message(api, chain, std::move(checksum), ByteArray{payload, alloc}, alloc)
{
    payload.remove_prefix(payload_.size());
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , block::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , payload_(rhs.payload_, alloc)
{
}

auto Message::get_payload(Transport type, WriteBuffer& buf) const
    noexcept(false) -> void
{
    copy(payload_.Bytes(), buf, "block");
}
}  // namespace opentxs::network::blockchain::bitcoin::message::block
