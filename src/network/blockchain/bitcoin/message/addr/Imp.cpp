// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/addr/Imp.hpp"  // IWYU pragma: associated

#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Protocol.hpp"   // IWYU pragma: keep
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs::network::blockchain::bitcoin::message::addr
{
Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    const ProtocolVersion version,
    AddressVector&& addresses,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , addr::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::addr,
          std::move(checksum),
          alloc)
    , version_(version)
    , payload_(std::move(addresses), alloc)
    , cached_size_(std::nullopt)
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
              auto out = decltype(payload_){alloc};
              out.clear();
              const auto count = decode_compact_size(payload, "address count");

              for (auto i = 0_uz; i < count; ++i) {
                  const bool timestamp = SerializeTimestamp(version);
                  using enum network::blockchain::Transport;

                  if (timestamp) {
                      auto raw = BitcoinFormat_31402{};
                      deserialize_object(payload, raw, "address");
                      const auto [network, bytearray] =
                          ExtractAddress(raw.data_.address_);
                      out.emplace_back(api.Factory().BlockchainAddress(
                          network::blockchain::Protocol::bitcoin,
                          network,
                          bytearray.Bytes(),
                          raw.data_.port_.value(),
                          chain,
                          convert_stime(raw.time_.value()),
                          TranslateServices(
                              chain,
                              version,
                              GetServices(raw.data_.services_.value()))));
                  } else {
                      auto raw = AddressVersion{};
                      deserialize_object(payload, raw, "address");
                      const auto [network, bytearray] =
                          ExtractAddress(raw.address_);
                      out.emplace_back(api.Factory().BlockchainAddress(
                          network::blockchain::Protocol::bitcoin,
                          network,
                          bytearray.Bytes(),
                          raw.port_.value(),
                          chain,
                          Time{},
                          TranslateServices(
                              chain,
                              version,
                              GetServices(raw.services_.value()))));
                  }
              }

              return out;
          }(),
          alloc)
{
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , addr::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , version_(rhs.version_)
    , payload_(rhs.payload_, alloc)
    , cached_size_(rhs.cached_size_)
{
}

Message::BitcoinFormat_31402::BitcoinFormat_31402(
    const opentxs::blockchain::Type chain,
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    const network::blockchain::Address& address)
    : time_(shorten(Clock::to_time_t(address.LastConnected())))
    , data_(chain, version, address)
{
}

Message::BitcoinFormat_31402::BitcoinFormat_31402()
    : time_()
    , data_()
{
}

auto Message::ExtractAddress(AddressByteField in) noexcept
    -> std::pair<network::blockchain::Transport, ByteArray>
{
    std::pair<network::blockchain::Transport, ByteArray> output{
        network::blockchain::Transport::ipv6, ByteArray{in.data(), in.size()}};
    auto& [type, bytes] = output;
    auto prefix = ByteArray{};

    if (bytes.Extract(AddressVersion::cjdns_prefix().size(), prefix) &&
        AddressVersion::cjdns_prefix() == prefix) {
        type = network::blockchain::Transport::cjdns;
    } else if (
        bytes.Extract(AddressVersion::ipv4_prefix().size(), prefix) &&
        AddressVersion::ipv4_prefix() == prefix) {
        type = network::blockchain::Transport::ipv4;
        bytes.Extract(
            bytes.size() - AddressVersion::ipv4_prefix().size(),
            prefix,
            AddressVersion::ipv4_prefix().size());
        bytes = prefix;

        assert_true(4 == bytes.size());
    } else if (
        bytes.Extract(AddressVersion::onion_prefix().size(), prefix) &&
        AddressVersion::onion_prefix() == prefix) {
        type = network::blockchain::Transport::onion2;
        bytes.Extract(
            bytes.size() - AddressVersion::onion_prefix().size(),
            prefix,
            AddressVersion::onion_prefix().size());
        bytes = prefix;

        assert_true(10 == bytes.size());
    }

    return output;
}

auto Message::get_payload(Transport type, WriteBuffer& buf) const
    noexcept(false) -> void
{
    const auto cs = CompactSize(payload_.size());
    serialize_compact_size(cs, buf, "address count");

    for (const auto& address : payload_) {
        if (SerializeTimestamp()) {
            const auto data = BitcoinFormat_31402{chain_, version_, address};
            serialize_object(data, buf, "address");
        } else {
            const auto data = AddressVersion{chain_, version_, address};
            serialize_object(data, buf, "address");
        }
    }
}

auto Message::get_size() const noexcept -> std::size_t
{
    if (false == cached_size_.has_value()) {
        const auto cs = CompactSize(payload_.size());
        const auto size = [this] {
            if (SerializeTimestamp()) {

                return sizeof(BitcoinFormat_31402);
            } else {

                return sizeof(AddressVersion);
            }
        }();
        cached_size_.emplace(cs.Size() + (payload_.size() * size));
    }

    return *cached_size_;
}

auto Message::SerializeTimestamp(
    const network::blockchain::bitcoin::message::ProtocolVersion
        version) noexcept -> bool
{
    return version >= 31402;
}
}  // namespace opentxs::network::blockchain::bitcoin::message::addr
