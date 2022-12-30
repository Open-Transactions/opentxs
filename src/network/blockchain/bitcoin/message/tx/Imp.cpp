// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/tx/Imp.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::tx
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ByteArray payload,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , tx::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::tx,
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
    , tx::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , payload_(rhs.payload_, alloc)
{
}

auto Message::get_payload(Transport type, WriteBuffer& buf) const
    noexcept(false) -> void
{
    copy(payload_.Bytes(), buf, "tx");
}
}  // namespace opentxs::network::blockchain::bitcoin::message::tx
