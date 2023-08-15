// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/cfheaders/Imp.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "internal/blockchain/protocol/bitcoin/base/Bitcoin.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::cfheaders
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    opentxs::blockchain::cfilter::Type type,
    opentxs::blockchain::block::Hash stop,
    opentxs::blockchain::cfilter::Header previous,
    Vector<internal::Cfheaders::value_type> hashes,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , cfheaders::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::cfheaders,
          std::move(checksum),
          alloc)
    , type_(std::move(type))
    , stop_(std::move(stop))
    , previous_(std::move(previous))
    , payload_(std::move(hashes), alloc)
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
              auto out = FilterPrefixChained{};
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
    FilterPrefixChained data,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          data.Type(chain),
          data.Stop(),
          data.Previous(),
          [&] {
              const auto count = decode_compact_size(payload, "cfheader count");
              auto out = decltype(payload_){alloc};

              for (auto i = 0_uz; i < count; ++i) {
                  constexpr auto size =
                      opentxs::blockchain::cfilter::Header::payload_size_;
                  out.emplace_back(extract_prefix(payload, size, "cfheader"));
              }

              return out;
          }(),
          alloc)
{
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , cfheaders::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , type_(rhs.type_)
    , stop_(rhs.stop_)
    , previous_(rhs.previous_)
    , payload_(rhs.payload_, alloc)
    , cached_size_(rhs.cached_size_)
{
}

auto Message::get_payload(Transport type, WriteBuffer& buf) const
    noexcept(false) -> void
{
    const auto prefix = FilterPrefixChained{chain_, type_, stop_, previous_};
    serialize_object(prefix, buf, "prefix");
    const auto cs = CompactSize(payload_.size());
    serialize_compact_size(cs, buf, "cfheader count");

    for (const auto& hash : payload_) { copy(hash.Bytes(), buf, "cfheader"); }
}

auto Message::get_size() const noexcept -> std::size_t
{
    if (false == cached_size_.has_value()) {
        using opentxs::blockchain::standard_hash_size_;
        const auto hashes = payload_.size();
        const auto cs = CompactSize(hashes);
        cached_size_.emplace(
            prefix_ + cs.Size() + (hashes * standard_hash_size_));
    }

    return *cached_size_;
}
}  // namespace opentxs::network::blockchain::bitcoin::message::cfheaders
