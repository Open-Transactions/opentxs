// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/reject/Imp.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::reject
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ReadView name,
    std::byte code,
    ReadView reason,
    ReadView extra,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , reject::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::reject,
          std::move(checksum),
          alloc)
    , name_(name, alloc)
    , reason_(reason, alloc)
    , code_(code)
    , extra_(extra, alloc)
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
              const auto size =
                  decode_compact_size(payload, "rejected message name size");
              check_at_least(payload, size, "rejected message name");
              auto out = payload.substr(0_uz, size);
              payload.remove_prefix(size);

              return out;
          }(),
          payload,
          alloc)
{
}

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ReadView name,
    ReadView& payload,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          name,
          [&] {
              auto out = std::byte{};
              deserialize_object(payload, out, "rejection code");

              return out;
          }(),
          payload,
          alloc)
{
}

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ReadView name,
    std::byte code,
    ReadView& payload,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          name,
          code,
          [&] {
              const auto size =
                  decode_compact_size(payload, "rejection reason size");
              check_at_least(payload, size, "rejection reason name");
              auto out = payload.substr(0_uz, size);
              payload.remove_prefix(size);

              return out;
          }(),
          [&]() -> ReadView {
              const auto size = extra_data(name, code);

              if (0_uz < size) {
                  check_at_least(payload, size, "extra data");
                  auto out = payload.substr(0_uz, size);
                  payload.remove_prefix(size);

                  return out;
              } else {

                  return {};
              }
          }(),
          alloc)
{
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , reject::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , name_(rhs.name_, alloc)
    , reason_(rhs.reason_, alloc)
    , code_(rhs.code_)
    , extra_(rhs.extra_, alloc)
{
}

auto Message::extra_data(ReadView name, std::byte code) noexcept -> std::size_t
{
    switch (code) {
        case std::byte{0x0}:
        case std::byte{0x10}: {

            return 32_uz;
        }
        case std::byte{0x11}: {
            if ("block"sv == name) { return 32_uz; }
        } break;
        case std::byte{0x12}: {
            if ("tx"sv == name) { return 32_uz; }
        } break;
        case std::byte{0x40}: {

            return 32_uz;
        }
        case std::byte{0x41}: {

            return 32_uz;
        }
        case std::byte{0x42}: {

            return 32_uz;
        }
        case std::byte{0x43}: {

            return 32_uz;
        }
        default: {
        }
    }

    return 0_uz;
}
}  // namespace opentxs::network::blockchain::bitcoin::message::reject
