// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/bitcoin/p2p/message/Version.hpp"  // IWYU pragma: associated

#include <boost/asio.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/p2p/Header.hpp"
#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinP2PVersion(
    const api::Session& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Version*
{
    try {
        namespace bitcoin = blockchain::p2p::bitcoin;
        using ReturnType = bitcoin::message::implementation::Version;

        if (false == bool(pHeader)) {

            throw std::runtime_error{"invalid header"};
        }

        const auto& header = *pHeader;
        auto expectedSize = sizeof(ReturnType::BitcoinFormat_1);

        if (expectedSize > size) {

            throw std::runtime_error{"size below minimum for version 1"};
        }

        const auto* it{static_cast<const std::byte*>(payload)};
        ReturnType::BitcoinFormat_1 raw1{};
        std::memcpy(reinterpret_cast<std::byte*>(&raw1), it, sizeof(raw1));
        it += sizeof(raw1);
        bitcoin::ProtocolVersion version{raw1.version_.value()};
        auto services = bitcoin::GetServices(raw1.services_.value());
        auto localServices{services};
        auto timestamp = convert_stime(raw1.timestamp_.value());
        auto remoteServices =
            bitcoin::GetServices(raw1.remote_.services_.value());
        tcp::endpoint remoteAddress{
            ip::make_address_v6(raw1.remote_.address_),
            raw1.remote_.port_.value()};
        tcp::endpoint localAddress{};
        bitcoin::Nonce nonce{};
        UnallocatedCString userAgent{};
        blockchain::block::Height height{};
        bool relay{true};

        if (106 <= version) {
            expectedSize += (1 + sizeof(ReturnType::BitcoinFormat_106));

            if (expectedSize > size) {

                throw std::runtime_error{"size below minimum for version 106"};
            }

            ReturnType::BitcoinFormat_106 raw2{};
            std::memcpy(reinterpret_cast<std::byte*>(&raw2), it, sizeof(raw2));
            it += sizeof(raw2);
            localServices = bitcoin::GetServices(raw2.local_.services_.value());
            localAddress = {
                ip::make_address_v6(raw2.local_.address_),
                raw2.local_.port_.value()};
            nonce = raw2.nonce_.value();
            auto uaSize = 0_uz;

            if (std::byte{0} == *it) {
                it += 1;
            } else {
                const auto csBytes = bitcoin::CompactSize::CalculateSize(*it);
                expectedSize += csBytes;

                if (expectedSize > size) {

                    throw std::runtime_error{"CompactSize incomplete"};
                }

                if (0 == csBytes) {
                    uaSize = std::to_integer<std::uint8_t>(*it);
                    it += 1;
                } else {
                    it += 1;
                    uaSize = convert_to_size(
                        bitcoin::CompactSize(
                            bitcoin::CompactSize::Bytes{it, it + csBytes})
                            .Value());
                    it += csBytes;
                }

                expectedSize += uaSize;

                if (expectedSize > size) {

                    throw std::runtime_error{"user agent string incomplete"};
                }

                userAgent = {reinterpret_cast<const char*>(it), uaSize};
                it += uaSize;
            }
        }

        if (209 <= version) {
            expectedSize += sizeof(ReturnType::BitcoinFormat_209);

            if (expectedSize > size) {

                throw std::runtime_error{"required height field is missing"};
            }

            ReturnType::BitcoinFormat_209 raw3{};
            std::memcpy(reinterpret_cast<std::byte*>(&raw3), it, sizeof(raw3));
            it += sizeof(raw3);

            height = raw3.height_.value();
        }

        if (70001 <= version) {
            expectedSize += 1;

            if (expectedSize == size) {
                auto value = std::to_integer<std::uint8_t>(*it);
                relay = (0 == value) ? false : true;
            }
        }

        const auto chain = header.Network();

        return new ReturnType(
            api,
            std::move(pHeader),
            version,
            localAddress,
            remoteAddress,
            TranslateServices(chain, version, services),
            TranslateServices(chain, version, localServices),
            TranslateServices(chain, version, remoteServices),
            nonce,
            userAgent,
            height,
            relay,
            timestamp);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto BitcoinP2PVersion(
    const api::Session& api,
    const blockchain::Type network,
    const blockchain::p2p::Network style,
    const std::int32_t version,
    const UnallocatedSet<blockchain::p2p::Service>& localServices,
    const UnallocatedCString& localAddress,
    const std::uint16_t localPort,
    const UnallocatedSet<blockchain::p2p::Service>& remoteServices,
    const UnallocatedCString& remoteAddress,
    const std::uint16_t remotePort,
    const std::uint64_t nonce,
    const std::string_view userAgent,
    const blockchain::block::Height height,
    const bool relay) -> blockchain::p2p::bitcoin::message::internal::Version*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Version;
    tcp::endpoint local{};
    tcp::endpoint remote{};

    if (blockchain::p2p::Network::zmq == style) {
        local =
            tcp::endpoint(ip::make_address_v6("::FFFF:7f0e:5801"), localPort);
        remote =
            tcp::endpoint(ip::make_address_v6("::FFFF:7f0e:5802"), remotePort);
    } else {
        try {
            local = tcp::endpoint(ip::make_address_v6(localAddress), localPort);
        } catch (...) {
            LogError()("opentxs::factory::")(__func__)(
                ": Invalid local address: ")(localAddress)
                .Flush();

            OT_FAIL;
        }

        try {
            remote =
                tcp::endpoint(ip::make_address_v6(remoteAddress), remotePort);
        } catch (...) {
            LogError()("opentxs::factory::")(__func__)(
                ": Invalid remote address: ")(remoteAddress)
                .Flush();

            OT_FAIL;
        }
    }

    return new ReturnType(
        api,
        network,
        version,
        local,
        remote,
        localServices,
        localServices,
        remoteServices,
        nonce,
        userAgent,
        height,
        relay);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Version::Version(
    const api::Session& api,
    const blockchain::Type network,
    const bitcoin::ProtocolVersion version,
    const tcp::endpoint localAddress,
    const tcp::endpoint remoteAddress,
    const UnallocatedSet<blockchain::p2p::Service>& services,
    const UnallocatedSet<blockchain::p2p::Service>& localServices,
    const UnallocatedSet<blockchain::p2p::Service>& remoteServices,
    const bitcoin::Nonce nonce,
    const std::string_view userAgent,
    const block::Height height,
    const bool relay,
    const Time time) noexcept
    : Message(api, network, bitcoin::Command::version)
    , version_(version)
    , local_address_(localAddress)
    , remote_address_(remoteAddress)
    , services_(services)
    , local_services_(localServices)
    , remote_services_(remoteServices)
    , nonce_(nonce)
    , user_agent_(userAgent)
    , height_(height)
    , relay_(relay)
    , timestamp_(time)
{
    init_hash();
}

Version::Version(
    const api::Session& api,
    std::unique_ptr<Header> header,
    const bitcoin::ProtocolVersion version,
    const tcp::endpoint localAddress,
    const tcp::endpoint remoteAddress,
    const UnallocatedSet<blockchain::p2p::Service>& services,
    const UnallocatedSet<blockchain::p2p::Service>& localServices,
    const UnallocatedSet<blockchain::p2p::Service>& remoteServices,
    const bitcoin::Nonce nonce,
    const std::string_view userAgent,
    const block::Height height,
    const bool relay,
    const Time time) noexcept
    : Message(api, std::move(header))
    , version_(version)
    , local_address_(localAddress)
    , remote_address_(remoteAddress)
    , services_(services)
    , local_services_(localServices)
    , remote_services_(remoteServices)
    , nonce_(nonce)
    , user_agent_(userAgent)
    , height_(height)
    , relay_(relay)
    , timestamp_(time)
{
}

Version::BitcoinFormat_1::BitcoinFormat_1() noexcept
    : version_()
    , services_()
    , timestamp_()
    , remote_()
{
    static_assert(46 == sizeof(BitcoinFormat_1));
}

Version::BitcoinFormat_1::BitcoinFormat_1(
    const bitcoin::ProtocolVersion version,
    const UnallocatedSet<bitcoin::Service>& localServices,
    const UnallocatedSet<bitcoin::Service>& remoteServices,
    const tcp::endpoint& remoteAddress,
    const Time time) noexcept
    : version_(version)
    , services_(GetServiceBytes(localServices))
    , timestamp_(Clock::to_time_t(time))
    , remote_(remoteServices, remoteAddress)
{
    static_assert(46 == sizeof(BitcoinFormat_1));
}

Version::BitcoinFormat_106::BitcoinFormat_106() noexcept
    : local_()
    , nonce_()
{
    static_assert(34 == sizeof(BitcoinFormat_106));
}

Version::BitcoinFormat_106::BitcoinFormat_106(
    const UnallocatedSet<bitcoin::Service>& services,
    const tcp::endpoint address,
    const bitcoin::Nonce nonce) noexcept
    : local_(services, address)
    , nonce_(nonce)
{
    static_assert(34 == sizeof(BitcoinFormat_106));
}

Version::BitcoinFormat_209::BitcoinFormat_209() noexcept
    : height_()
{
    static_assert(4 == sizeof(BitcoinFormat_209));
}

Version::BitcoinFormat_209::BitcoinFormat_209(
    const block::Height height) noexcept
    : height_(static_cast<std::uint32_t>(height))
{
    static_assert(4 == sizeof(BitcoinFormat_209));
    static_assert(sizeof(height_) == sizeof(std::uint32_t));

    OT_ASSERT(std::numeric_limits<std::uint32_t>::max() >= height);
}

auto Version::payload(AllocateOutput out) const noexcept -> bool
{
    try {
        if (!out) { throw std::runtime_error{"invalid output allocator"}; }

        auto userAgent = ByteArray{};
        const auto bytes = [&] {
            auto output = sizeof(BitcoinFormat_1);

            if (106 <= version_) {
                userAgent = BitcoinString(user_agent_);
                output += sizeof(BitcoinFormat_106) + userAgent.size();
            }

            if (209 <= version_) { output += sizeof(BitcoinFormat_209); }

            if (70001 <= version_) { output += sizeof(std::byte); }

            return output;
        }();

        auto output = out(bytes);

        if (false == output.valid(bytes)) {
            throw std::runtime_error{"failed to allocate output space"};
        }

        const auto data1 = BitcoinFormat_1{
            version_,
            TranslateServices(header_->Network(), version_, services_),
            TranslateServices(header_->Network(), version_, remote_services_),
            remote_address_,
            timestamp_};
        auto* i = output.as<std::byte>();
        std::memcpy(i, static_cast<const void*>(&data1), sizeof(data1));
        std::advance(i, sizeof(data1));

        if (106 <= version_) {
            const auto data2 = BitcoinFormat_106{
                TranslateServices(
                    header_->Network(), version_, local_services_),
                local_address_,
                nonce_};
            std::memcpy(i, static_cast<const void*>(&data2), sizeof(data2));
            std::advance(i, sizeof(data2));
            std::memcpy(i, userAgent.data(), userAgent.size());
            std::advance(i, userAgent.size());
        }

        if (209 <= version_) {
            const auto data3 = BitcoinFormat_209{height_};
            std::memcpy(i, static_cast<const void*>(&data3), sizeof(data3));
            std::advance(i, sizeof(data3));
        }

        if (70001 <= version_) {
            static constexpr auto relayTrue = std::byte{1};
            static constexpr auto relayFalse = std::byte{0};
            const auto& relay = (relay_) ? relayTrue : relayFalse;
            std::memcpy(i, &relay, sizeof(relay));
            std::advance(i, sizeof(relay));
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
