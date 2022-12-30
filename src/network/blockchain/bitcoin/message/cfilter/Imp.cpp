// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/cfilter/Imp.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::cfilter
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    opentxs::blockchain::cfilter::Type type,
    opentxs::blockchain::block::Hash hash,
    std::uint32_t count,
    ByteArray compressed,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , cfilter::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::cfilter,
          std::move(checksum),
          alloc)
    , type_(std::move(type))
    , hash_(std::move(hash))
    , count_(std::move(count))
    , filter_(std::move(compressed), alloc)
    , params_(opentxs::blockchain::internal::GetFilterParams(type_))
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
              auto out = FilterPrefixBasic{};
              deserialize_object(payload, out, "prefix");

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
    FilterPrefixBasic data,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          data.Type(chain),
          data.Hash(),
          [&] {
              const auto size =
                  decode_compact_size(payload, "serialized cfilter size");
              check_at_least(payload, size, "serialized cfilter");
              auto out = payload.substr(0_uz, size);
              payload.remove_prefix(size);

              return out;
          }(),
          alloc)
{
}

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    opentxs::blockchain::cfilter::Type type,
    opentxs::blockchain::block::Hash hash,
    ReadView cfilter,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          std::move(type),
          std::move(hash),
          [&] {
              const auto count =
                  decode_compact_size(cfilter, "cfilter element count");

              return shorten(count);
          }(),
          cfilter,
          alloc)
{
}

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    opentxs::blockchain::cfilter::Type type,
    opentxs::blockchain::block::Hash hash,
    std::uint32_t count,
    ReadView& compressed,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          std::move(type),
          std::move(hash),
          std::move(count),
          ByteArray{compressed, alloc},
          alloc)
{
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , cfilter::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , type_(rhs.type_)
    , hash_(rhs.hash_)
    , count_(rhs.count_)
    , filter_(rhs.filter_, alloc)
    , params_(rhs.params_)
    , cached_size_(rhs.cached_size_)
{
}

auto Message::get_payload(Transport type, WriteBuffer& buf) const
    noexcept(false) -> void
{
    const auto prefix = FilterPrefixBasic{chain_, type_, hash_};
    serialize_object(prefix, buf, "prefix");
    const auto inner = CompactSize{count_};
    const auto outer = CompactSize{inner.Size() + filter_.size()};
    serialize_compact_size(outer, buf, "filter bytes");
    serialize_compact_size(inner, buf, "element count");
    copy(filter_.Bytes(), buf, "cfilter");
}

auto Message::get_size() const noexcept -> std::size_t
{
    if (false == cached_size_.has_value()) {
        const auto inner = CompactSize{count_};
        const auto outer = CompactSize{inner.Size() + filter_.size()};
        cached_size_.emplace(prefix_ + outer.Total());
    }

    return *cached_size_;
}
}  // namespace opentxs::network::blockchain::bitcoin::message::cfilter
