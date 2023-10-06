// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/addr2/Imp.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::addr2
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    const ProtocolVersion version,
    AddressVector&& addresses,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , addr2::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::addr2,
          std::move(checksum),
          alloc)
    , version_(version)
    , payload_(std::move(addresses), alloc)
    , cached_size_(std::nullopt)
    , cached_items_(std::nullopt)
{
}

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    ReadView& payload,
    const ProtocolVersion version,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          version,
          [&] {
              const auto count = decode_compact_size(payload, "address count");
              auto out = decltype(payload_){alloc};
              out.reserve(count);
              out.clear();

              for (auto i = 0_uz; i < count; ++i) {
                  auto bip155 = Bip155::Decode(payload);
                  using enum blockchain::Transport;

                  if (invalid != bip155.GetNetwork()) {
                      const auto& address = out.emplace_back(
                          bip155.ToAddress(api, chain, version));

                      if (false == address.IsValid()) {
                          LogError()(OT_PRETTY_CLASS())(
                              "error decoding address")
                              .Flush();
                          out.pop_back();
                      }
                  } else {
                      LogError()(OT_PRETTY_CLASS())("ignoring invalid addr")
                          .Flush();
                  }
              }

              return out;
          }(),
          alloc)
{
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , addr2::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , version_(rhs.version_)
    , payload_(rhs.payload_, alloc)
    , cached_size_(rhs.cached_size_)
    , cached_items_(rhs.cached_items_)
{
}

auto Message::get_payload(Transport type, WriteBuffer& buf) const
    noexcept(false) -> void
{
    const auto cs = CompactSize(payload_.size());
    serialize_compact_size(cs, buf, "address count");

    for (const auto& addr2 : *cached_items_) {
        if (false == addr2.Serialize(buf)) {

            throw std::runtime_error{"failed to serialize addresst"};
        }
    }
}

auto Message::get_size() const noexcept -> std::size_t
{
    if (false == cached_size_.has_value()) {
        const auto count = payload_.size();
        const auto cs = CompactSize(count);
        auto value = cs.Size();

        if (cached_items_.has_value()) {
            const auto& data = *cached_items_;
            value += std::accumulate(
                data.begin(),
                data.end(),
                0_uz,
                [](const auto lhs, const auto& rhs) {
                    return lhs + rhs.size();
                });
        } else {
            auto& v = cached_items_.emplace(get_allocator());
            v.reserve(count);
            v.clear();
            std::ranges::transform(
                payload_,
                std::back_inserter(v),
                [&](const auto& item) -> Bip155 {
                    auto out = Bip155{chain_, version_, item};
                    value += out.size();

                    return out;
                });
        }

        cached_size_.emplace(value);
    }

    return *cached_size_;
}
}  // namespace opentxs::network::blockchain::bitcoin::message::addr2
