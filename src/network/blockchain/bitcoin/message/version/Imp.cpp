// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/bitcoin/message/version/Imp.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <limits>
#include <string_view>

#include "BoostAsio.hpp"
#include "internal/network/asio/Types.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Size.hpp"
#include "internal/util/Time.hpp"
#include "network/blockchain/bitcoin/message/base/MessagePrivate.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs::network::blockchain::bitcoin::message::version
{
using namespace std::literals;

Message::Message(
    const api::Session& api,
    const opentxs::blockchain::Type chain,
    std::optional<ByteArray> checksum,
    message::ProtocolVersion version,
    tcp::endpoint localAddress,
    tcp::endpoint remoteAddress,
    Set<network::blockchain::bitcoin::Service> services,
    Set<network::blockchain::bitcoin::Service> localServices,
    Set<network::blockchain::bitcoin::Service> remoteServices,
    message::Nonce nonce,
    CString userAgent,
    opentxs::blockchain::block::Height height,
    bool bip37,
    Time timestamp,
    allocator_type alloc) noexcept
    : internal::MessagePrivate(alloc)
    , version::MessagePrivate(alloc)
    , implementation::Message(
          api,
          chain,
          Command::version,
          std::move(checksum),
          alloc)
    , version_(std::move(version))
    , local_address_(std::move(localAddress))
    , remote_address_(std::move(remoteAddress))
    , services_(std::move(services), alloc)
    , local_services_(std::move(localServices), alloc)
    , remote_services_(std::move(remoteServices), alloc)
    , nonce_(std::move(nonce))
    , user_agent_(std::move(userAgent), alloc)
    , height_(std::move(height))
    , bip37_(std::move(bip37))
    , timestamp_(std::move(timestamp))
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
              auto out = BitcoinFormat_1{};
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
    BitcoinFormat_1 data,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          payload,
          data.version_.value(),
          TranslateServices(
              chain,
              data.version_.value(),
              GetServices(data.services_.value())),
          tcp::endpoint{
              *asio::address_from_binary(
                  {reinterpret_cast<const char*>(data.remote_.address_.data()),
                   data.remote_.address_.size()}),
              data.remote_.port_.value()},
          TranslateServices(
              chain,
              data.version_.value(),
              GetServices(data.remote_.services_.value())),
          convert_stime(data.timestamp_.value()),
          [&] {
              auto out = std::make_pair(BitcoinFormat_106{}, CString{alloc});
              out.second.clear();

              if (106 <= data.version_.value()) {
                  deserialize_object(payload, out.first, "version 106 data");
                  const auto size =
                      decode_compact_size(payload, "user agent bytes");
                  out.second.assign(
                      extract_prefix(payload, size, "user agent"));
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
    message::ProtocolVersion version,
    Set<network::blockchain::bitcoin::Service> services,
    tcp::endpoint remoteAddress,
    Set<network::blockchain::bitcoin::Service> remoteServices,
    Time timestamp,
    std::pair<BitcoinFormat_106, CString> data,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          payload,
          std::move(version),
          std::move(services),
          std::move(remoteAddress),
          std::move(remoteServices),
          std::move(timestamp),
          tcp::endpoint{
              *asio::address_from_binary(
                  {reinterpret_cast<const char*>(
                       data.first.local_.address_.data()),
                   data.first.local_.address_.size()}),
              data.first.local_.port_.value()},
          TranslateServices(
              chain,
              version,
              GetServices(data.first.local_.services_.value())),
          data.first.nonce_.value(),
          std::move(data.second),
          [&] {
              auto out = BitcoinFormat_209{};

              if (209 <= version) {
                  deserialize_object(payload, out, "version 209 data");
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
    message::ProtocolVersion version,
    Set<network::blockchain::bitcoin::Service> services,
    tcp::endpoint remoteAddress,
    Set<network::blockchain::bitcoin::Service> remoteServices,
    Time timestamp,
    tcp::endpoint localAddress,
    Set<network::blockchain::bitcoin::Service> localServices,
    message::Nonce nonce,
    CString userAgent,
    const BitcoinFormat_209& data,
    allocator_type alloc) noexcept(false)
    : Message(
          api,
          chain,
          std::move(checksum),
          std::move(version),
          std::move(localAddress),
          std::move(remoteAddress),
          std::move(services),
          std::move(localServices),
          std::move(remoteServices),
          std::move(nonce),
          std::move(userAgent),
          data.height_.value(),
          [&] {
              if (70001 <= version) {
                  auto bip37 = std::byte{};
                  deserialize_object(payload, bip37, "bip37");

                  return std::byte{0x1} == bip37;
              } else {

                  return false;
              }
          }(),
          timestamp,
          alloc)
{
}

Message::Message(const Message& rhs, allocator_type alloc) noexcept
    : internal::MessagePrivate(rhs, alloc)
    , version::MessagePrivate(rhs, alloc)
    , implementation::Message(rhs, alloc)
    , version_(rhs.version_)
    , local_address_(rhs.local_address_)
    , remote_address_(rhs.remote_address_)
    , services_(rhs.services_, alloc)
    , local_services_(rhs.local_services_, alloc)
    , remote_services_(rhs.remote_services_, alloc)
    , nonce_(rhs.nonce_)
    , user_agent_(rhs.user_agent_, alloc)
    , height_(rhs.height_)
    , bip37_(rhs.bip37_)
    , timestamp_(rhs.timestamp_)
    , cached_size_(rhs.cached_size_)
{
}

auto Message::get_payload(Transport type, WriteBuffer& buf) const
    noexcept(false) -> void
{
    {
        const auto data = BitcoinFormat_1{
            version_,
            TranslateServices(chain_, version_, services_),
            TranslateServices(chain_, version_, remote_services_),
            remote_address_,
            timestamp_};
        serialize_object(data, buf, "prefix");
    }

    if (106 <= version_) {
        const auto data = BitcoinFormat_106{
            TranslateServices(chain_, version_, local_services_),
            local_address_,
            nonce_};
        serialize_object(data, buf, "version 106 data");
        const auto cs = CompactSize{user_agent_.size()};
        serialize_compact_size(cs, buf, "user agent bytes");
        copy(user_agent_, buf, "user agent");
    }

    if (209 <= version_) {
        const auto data = BitcoinFormat_209{height_};
        serialize_object(data, buf, "version 209 data");
    }

    if (70001 <= version_) {
        const auto bip37 = [&] {
            if (bip37_) {

                return std::byte{0x0};
            } else {

                return std::byte{0x1};
            }
        }();
        serialize_object(bip37, buf, "bip37 relay flag");
    }
}

auto Message::get_size() const noexcept -> std::size_t
{
    if (false == cached_size_.has_value()) {
        auto size = sizeof(BitcoinFormat_1);

        if (106 <= version_) {
            const auto cs = CompactSize{user_agent_.size()};
            size += sizeof(BitcoinFormat_106) + cs.Total();
        }

        if (209 <= version_) { size += sizeof(BitcoinFormat_209); }

        if (70001 <= version_) { size += sizeof(std::byte); }

        cached_size_.emplace(size);
    }

    return *cached_size_;
}
}  // namespace opentxs::network::blockchain::bitcoin::message::version

namespace opentxs::network::blockchain::bitcoin::message::version
{
Message::BitcoinFormat_1::BitcoinFormat_1() noexcept
    : version_()
    , services_()
    , timestamp_()
    , remote_()
{
    static_assert(46 == sizeof(BitcoinFormat_1));
}

Message::BitcoinFormat_1::BitcoinFormat_1(
    const network::blockchain::bitcoin::message::ProtocolVersion version,
    const UnallocatedSet<message::Service>& localServices,
    const UnallocatedSet<message::Service>& remoteServices,
    const tcp::endpoint& remoteAddress,
    const Time time) noexcept
    : version_(version)
    , services_(GetServiceBytes(localServices))
    , timestamp_(Clock::to_time_t(time))
    , remote_(remoteServices, remoteAddress)
{
    static_assert(46 == sizeof(BitcoinFormat_1));
}

Message::BitcoinFormat_106::BitcoinFormat_106() noexcept
    : local_()
    , nonce_()
{
    static_assert(34 == sizeof(BitcoinFormat_106));
}

Message::BitcoinFormat_106::BitcoinFormat_106(
    const UnallocatedSet<message::Service>& services,
    const tcp::endpoint address,
    const message::Nonce nonce) noexcept
    : local_(services, address)
    , nonce_(nonce)
{
    static_assert(34 == sizeof(BitcoinFormat_106));
}

Message::BitcoinFormat_209::BitcoinFormat_209() noexcept
    : height_()
{
    static_assert(4 == sizeof(BitcoinFormat_209));
}

Message::BitcoinFormat_209::BitcoinFormat_209(
    const opentxs::blockchain::block::Height height) noexcept
    : height_(static_cast<std::uint32_t>(height))
{
    static_assert(4 == sizeof(BitcoinFormat_209));
    static_assert(sizeof(height_) == sizeof(std::uint32_t));

    OT_ASSERT(std::numeric_limits<std::uint32_t>::max() >= height);
}
}  // namespace opentxs::network::blockchain::bitcoin::message::version
