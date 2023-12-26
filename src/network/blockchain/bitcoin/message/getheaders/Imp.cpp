// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/getheaders/Imp.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"

namespace opentxs::network::blockchain::bitcoin::message::getheaders
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ProtocolVersionUnsigned version,
    opentxs::blockchain::block::Hash stop,
    Vector<opentxs::blockchain::block::Hash> payload,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , getheaders::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::getheaders,
          std::move(checksum),
          alloc)
    , version_(std::move(version))
    , stop_(std::move(stop))
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
          payload,
          [&] {
              auto out = ProtocolVersionUnsigned{};
              deserialize_object(payload, out, "version"sv);

              return out;
          }(),
          alloc)
{
}

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ReadView& payload,
    ProtocolVersionUnsigned version,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          payload,
          version,
          decode_compact_size(payload, "hash count"sv),
          alloc)
{
}

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ReadView& payload,
    ProtocolVersionUnsigned version,
    std::size_t count,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          payload,
          version,
          [&] {
              auto out = Vector<opentxs::blockchain::block::Hash>{alloc};
              out.reserve(count);
              out.clear();

              for (auto n = 0_uz; n < count; ++n) {
                  out.emplace_back(
                      extract_prefix(payload, hash_, "block hash"sv));
              }

              return out;
          }(),
          alloc)
{
}

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ReadView& payload,
    ProtocolVersionUnsigned version,
    Vector<opentxs::blockchain::block::Hash> hashes,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          version,
          [&]() -> opentxs::blockchain::block::Hash {
              return extract_prefix(payload, hash_, "block hash"sv);
          }(),
          std::move(hashes),
          alloc)
{
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , getheaders::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , version_(rhs.version_)
    , stop_(rhs.stop_)
    , payload_(rhs.payload_, alloc)
    , cached_size_(rhs.cached_size_)
{
}

auto Message::get_payload(Transport type, WriteBuffer& buf) const
    noexcept(false) -> void
{
    serialize_object(version_, buf, "version");
    const auto cs = CompactSize(payload_.size());
    serialize_compact_size(cs, buf, "hash count");

    for (const auto& hash : payload_) { copy(hash.Bytes(), buf, "block hash"); }

    copy(stop_.Bytes(), buf, "stop hash");
}

auto Message::get_size() const noexcept -> std::size_t
{
    if (false == cached_size_.has_value()) {
        const auto cs = CompactSize(payload_.size());

        cached_size_.emplace(
            sizeof(ProtocolVersionUnsigned) + cs.Size() +
            ((payload_.size() + 1_uz) * hash_));
    }

    return *cached_size_;
}
}  // namespace opentxs::network::blockchain::bitcoin::message::getheaders
