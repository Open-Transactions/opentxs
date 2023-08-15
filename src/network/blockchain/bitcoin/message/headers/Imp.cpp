// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/headers/Imp.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/blockchain/block/Header.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::network::blockchain::bitcoin::message::headers
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    Vector<internal::Headers::value_type> payload,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , headers::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::headers,
          std::move(checksum),
          alloc)
    , payload_(std::move(payload), alloc)
    , cached_size_(std::nullopt)
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
              const auto count = decode_compact_size(payload, "header count");
              auto out = decltype(payload_){alloc};
              out.reserve(count);
              out.clear();
              auto txCount = std::uint8_t{};

              for (auto n = 0_uz; n < count; ++n) {
                  out.emplace_back(factory::BitcoinBlockHeader(
                      api.Crypto(),
                      chain,
                      extract_prefix(payload, header_, "header"),
                      alloc));
                  deserialize_object(payload, txCount, "txcount");
              }

              return out;
          }(),
          alloc)
{
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , headers::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , payload_(rhs.payload_, alloc)
    , cached_size_(rhs.cached_size_)
{
}

auto Message::get_payload(Transport type, WriteBuffer& buf) const
    noexcept(false) -> void
{
    const auto cs = CompactSize(payload_.size());
    serialize_compact_size(cs, buf, "header count");
    static const auto txCount = std::uint8_t{};

    for (const auto& header : payload_) {
        if (false == header.Internal().Serialize(buf.Write(header_), true)) {

            throw std::runtime_error{"failed to serialize header"};
        }

        serialize_object(txCount, buf, "txcount");
    }
}

auto Message::get_size() const noexcept -> std::size_t
{
    if (false == cached_size_.has_value()) {
        const auto cs = CompactSize(payload_.size());
        cached_size_.emplace(cs.Size() + (payload_.size() * (header_ + 1_uz)));
    }

    return *cached_size_;
}
}  // namespace opentxs::network::blockchain::bitcoin::message::headers
