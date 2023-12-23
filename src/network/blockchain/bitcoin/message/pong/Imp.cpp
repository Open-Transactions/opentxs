// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/pong/Imp.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "internal/util/Bytes.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::pong
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    message::Nonce nonce,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , pong::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::pong,
          std::move(checksum),
          alloc)
    , nonce_(nonce)
{
}

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ReadView& payload,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          [&] {
              auto out = NonceField{};
              deserialize_object(payload, out, "nonce");

              return out.value();
          }(),
          alloc)
{
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , pong::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , nonce_(rhs.nonce_)
{
}

auto Message::get_payload(Transport type, WriteBuffer& buf) const
    noexcept(false) -> void
{
    const auto data = NonceField{nonce_};
    serialize_object(data, buf, "nonce");
}
}  // namespace opentxs::network::blockchain::bitcoin::message::pong
