// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/getcfilters/Imp.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "internal/util/Bytes.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::getcfilters
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    opentxs::blockchain::cfilter::Type type,
    opentxs::blockchain::block::Height start,
    opentxs::blockchain::block::Hash stop,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , getcfilters::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::getcfilters,
          std::move(checksum),
          alloc)
    , type_(std::move(type))
    , start_(std::move(start))
    , stop_(std::move(stop))
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
              auto out = FilterRequest{};
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
    FilterRequest data,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          data.Type(chain),
          data.Start(),
          data.Stop(),
          alloc)
{
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , getcfilters::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , type_(rhs.type_)
    , start_(rhs.start_)
    , stop_(rhs.stop_)
{
}

auto Message::get_payload(Transport type, WriteBuffer& buf) const
    noexcept(false) -> void
{
    const auto prefix = FilterRequest{chain_, type_, start_, stop_};
    serialize_object(prefix, buf, "prefix");
}
}  // namespace opentxs::network::blockchain::bitcoin::message::getcfilters
