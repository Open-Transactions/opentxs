// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/inv/Imp.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::network::blockchain::bitcoin::message::inv
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    Vector<internal::Inv::value_type> payload,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , inv::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::inv,
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
              const auto count = decode_compact_size(payload, "item count");
              auto out = decltype(payload_){alloc};
              out.reserve(count);
              out.clear();

              for (auto n = 0_uz; n < count; ++n) {
                  static const auto size =
                      internal::Inv::value_type::EncodedSize;
                  out.emplace_back(extract_prefix(payload, size, "inv"));
              }

              return out;
          }(),
          alloc)
{
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , inv::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , payload_(rhs.payload_, alloc)
    , cached_size_(rhs.cached_size_)
{
}

auto Message::get_payload(Transport type, WriteBuffer& buf) const
    noexcept(false) -> void
{
    const auto cs = CompactSize(payload_.size());
    serialize_compact_size(cs, buf, "inv count");
    static const auto size = internal::Inv::value_type::EncodedSize;

    for (const auto& inv : payload_) {
        if (false == inv.Serialize(buf.Write(size))) {
            throw std::runtime_error{"failed to serialize inv"};
        }
    }
}

auto Message::get_size() const noexcept -> std::size_t
{
    if (false == cached_size_.has_value()) {
        const auto cs = CompactSize(payload_.size());
        static const auto size = internal::Inv::value_type::EncodedSize;
        cached_size_.emplace(cs.Size() + (payload_.size() * size));
    }

    return *cached_size_;
}
}  // namespace opentxs::network::blockchain::bitcoin::message::inv
