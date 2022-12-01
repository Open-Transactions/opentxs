// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/Params.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Container.hpp"

namespace opentxs::blockchain::p2p::bitcoin
{
using namespace std::literals;

static constexpr auto bip155_to_opentxs_ = [] {
    using enum Bip155::Network;

    return frozen::make_unordered_map<Bip155::Network, p2p::Network>({
        {ipv4, p2p::Network::ipv4},
        {ipv6, p2p::Network::ipv6},
        {tor2, p2p::Network::onion2},
        {tor3, p2p::Network::onion3},
        {i2p, p2p::Network::eep},
        {cjdns, p2p::Network::cjdns},
    });
}();
static constexpr auto opentxs_to_bip155_ =
    frozen::invert_unordered_map(bip155_to_opentxs_);

const CommandMap command_map_{
    {Command::addr, "addr"},
    {Command::addr2, "addrv2"},
    {Command::alert, "alert"},
    {Command::block, "block"},
    {Command::blocktxn, "blocktxn"},
    {Command::cfcheckpt, "cfcheckpt"},
    {Command::cfheaders, "cfheaders"},
    {Command::cfilter, "cfilter"},
    {Command::checkorder, "checkorder"},
    {Command::cmpctblock, "cmpctblock"},
    {Command::feefilter, "feefilter"},
    {Command::filteradd, "filteradd"},
    {Command::filterclear, "filterclear"},
    {Command::filterload, "filterload"},
    {Command::getaddr, "getaddr"},
    {Command::getblocks, "getblocks"},
    {Command::getblocktxn, "getblocktxn"},
    {Command::getcfcheckpt, "getcfcheckpt"},
    {Command::getcfheaders, "getcfheaders"},
    {Command::getcfilters, "getcfilters"},
    {Command::getdata, "getdata"},
    {Command::getheaders, "getheaders"},
    {Command::headers, "headers"},
    {Command::inv, "inv"},
    {Command::mempool, "mempool"},
    {Command::merkleblock, "merkleblock"},
    {Command::notfound, "notfound"},
    {Command::ping, "ping"},
    {Command::pong, "pong"},
    {Command::reject, "reject"},
    {Command::reply, "reply"},
    {Command::sendaddr2, "sendaddrv2"},
    {Command::sendcmpct, "sendcmpct"},
    {Command::sendheaders, "sendheaders"},
    {Command::submitorder, "submitorder"},
    {Command::tx, "tx"},
    {Command::verack, "verack"},
    {Command::version, "version"},
};
const CommandReverseMap command_reverse_map_{reverse_map(command_map_)};

auto AddressVersion::cjdns_prefix() -> ByteArray
{
    static auto out = []() {
        auto out = ByteArray{};
        out.DecodeHex("0xfc");
        return out;
    }();
    return out;
}
auto AddressVersion::ipv4_prefix() -> ByteArray
{
    static auto out = []() {
        auto out = ByteArray{};
        out.DecodeHex("0x00000000000000000000ffff");
        return out;
    }();
    return out;
}
auto AddressVersion::onion_prefix() -> ByteArray
{
    static auto out = []() {
        auto out = ByteArray{};
        out.DecodeHex("0xfd87d87eeb43");
        return out;
    }();
    return out;
}

AddressVersion::AddressVersion(
    const UnallocatedSet<bitcoin::Service>& services,
    const tcp::endpoint& endpoint) noexcept
    : services_(GetServiceBytes(services))
    , address_(endpoint.address().to_v6().to_bytes())
    , port_(endpoint.port())
{
    static_assert(26 == sizeof(AddressVersion));
}

AddressVersion::AddressVersion(
    const blockchain::Type chain,
    const ProtocolVersion version,
    const p2p::Address& address) noexcept
    : services_(GetServiceBytes(
          TranslateServices(chain, version, address.Services())))
    , address_(Encode(address.Type(), address.Bytes()))
    , port_(address.Port())
{
    static_assert(26 == sizeof(AddressVersion));
}

AddressVersion::AddressVersion() noexcept
    : services_()
    , address_()
    , port_()
{
    static_assert(26 == sizeof(AddressVersion));
}

auto AddressVersion::Encode(const Network type, const Data& bytes)
    -> AddressByteField
{
    AddressByteField output{};

    switch (type) {
        case Network::ipv6:
        case Network::cjdns: {
            OT_ASSERT(output.size() == bytes.size());

            std::memcpy(output.data(), bytes.data(), output.size());
        } break;
        case Network::ipv4: {
            auto encoded{ipv4_prefix()};
            encoded += bytes;

            OT_ASSERT(output.size() == encoded.size());

            std::memcpy(output.data(), encoded.data(), output.size());
        } break;
        case Network::onion2: {
            auto encoded{onion_prefix()};
            encoded += bytes;

            OT_ASSERT(output.size() == encoded.size());

            std::memcpy(output.data(), encoded.data(), output.size());
        } break;
        case Network::onion3:
        case Network::eep:
        case Network::zmq:
        default: {
            OT_FAIL;
        }
    }

    return output;
}

Bip155::Bip155() noexcept
    : time_()
    , services_()
    , network_id_()
    , addr_()
    , port_()
{
}

Bip155::Bip155(
    const ProtocolVersion version,
    const p2p::Address& address) noexcept
    : time_(shorten(Clock::to_time_t(address.LastConnected())))
    , services_(GetServiceBytes(
          TranslateServices(address.Chain(), version, address.Services())))
    , network_id_(opentxs_to_bip155_.at(address.Type()))
    , addr_(address.Bytes())
    , port_(address.Port())
{
}

auto Bip155::Decode(ReadView& in) noexcept(false) -> Bip155
{
    auto notUsed = ReadView{};
    auto out = Bip155{};
    auto& id = out.network_id_;
    deserialize_object(in, out.time_, "time field"sv);
    DecodeCompactSize(in, notUsed, std::addressof(out.services_));
    deserialize_object(in, id, "network id"sv);
    deserialize(in, out.addr_.WriteInto(), GetSize(id), "address"sv);
    deserialize_object(in, out.port_, "port"sv);

    return out;
}

auto Bip155::GetNetwork() const noexcept -> p2p::Network
{
    return bip155_to_opentxs_.at(network_id_);
}

auto Bip155::GetSize(Network network) noexcept(false) -> std::size_t
{
    using enum Network;

    switch (network) {
        case ipv4: {

            return 4_uz;
        }
        case ipv6: {

            return 16_uz;
        }
        case tor2: {

            return 10_uz;
        }
        case tor3: {

            return 32_uz;
        }
        case i2p: {

            return 32_uz;
        }
        case cjdns: {

            return 16_uz;
        }
        default: {
            const auto error =
                UnallocatedCString{"unknown address type: "}.append(
                    std::to_string(static_cast<std::uint8_t>(network)));

            throw std::runtime_error{error};
        }
    }
}

auto Bip155::Serialize(WriteBuffer& out) const noexcept -> bool
{
    try {
        serialize_object(time_, out, "time"sv);
        serialize_compact_size(services_, out, "services"sv);
        serialize_object(network_id_, out, "network id"sv);
        copy(addr_.Bytes(), out, "address");
        serialize_object(port_, out, "port"sv);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(": ")(e.what()).Flush();

        return false;
    }
}

auto Bip155::size() const noexcept -> std::size_t
{
    return sizeof(time_) + sizeof(network_id_) + sizeof(port_) +
           services_.Size() + addr_.size();
}

auto Bip155::ToAddress(
    const api::Session& api,
    const blockchain::Type chain,
    const p2p::bitcoin::ProtocolVersion version) const noexcept -> p2p::Address
{
    return factory::BlockchainAddress(
        api,
        p2p::Protocol::bitcoin,
        GetNetwork(),
        addr_.Bytes(),
        port_.value(),
        chain,
        convert_time(time_.value()),
        bitcoin::TranslateServices(
            chain, version, bitcoin::GetServices(services_.Value())),
        false);
}

auto BitcoinString(const UnallocatedCString& in) noexcept -> ByteArray
{
    auto output = CompactSize(in.size()).Encode();

    if (false == in.empty()) { output.Concatenate(in.data(), in.size()); }

    return output;
}

auto convert_service_bit(BitVector8 value) noexcept -> bitcoin::Service
{
    if (0 == value) { return Service::None; }

    auto log{1};

    while (value >>= 1) { ++log; }

    return static_cast<bitcoin::Service>(log);
}

auto convert_service_bit(const bitcoin::Service value) noexcept -> BitVector8
{
    if (bitcoin::Service::None == value) { return {}; }

    BitVector8 output{1u};
    output <<= (static_cast<std::uint8_t>(value) - 1);

    return output;
}

auto GetCommand(const CommandField& bytes) noexcept -> Command
{
    try {
        const UnallocatedCString raw{
            reinterpret_cast<const char*>(bytes.data()), bytes.size()};
        const auto command = UnallocatedCString{raw.c_str()};

        return command_reverse_map_.at(command);
    } catch (...) {

        return Command::unknown;
    }
}

auto GetServiceBytes(const UnallocatedSet<bitcoin::Service>& services) noexcept
    -> BitVector8
{
    BitVector8 output{0};

    for (const auto& bit : services) {
        if (Service::None == bit) { continue; }

        output |= convert_service_bit(bit);
    }

    return output;
}

#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
__attribute__((no_sanitize("unsigned-integer-overflow")))
#endif
auto GetServices(
    const BitVector8 data) noexcept -> UnallocatedSet<bitcoin::Service>
{
    if (0 == data) { return {}; }

    auto output = UnallocatedSet<bitcoin::Service>{};
    auto mask = BitVector8{1};

    for (std::size_t i = 0; i < (8 * sizeof(data)); ++i) {
        const auto value = data & mask;

        if (0 != value) { output.emplace(convert_service_bit(value)); }

        // NOTE mask will overflow on the last iteration of the loop but it
        // doesn't matter since it will never be accessed again
        mask *= 2;
    }

    return output;
}
#ifndef _WIN32
#pragma GCC diagnostic pop
#endif

auto print(const Command command) noexcept -> std::string_view
{
    try {
        return command_map_.at(command);
    } catch (...) {
        return {};
    }
}

auto SerializeCommand(const Command command) noexcept -> CommandField
{
    CommandField output{};

    try {
        const auto string = print(command);

        OT_ASSERT(output.size() >= string.size());

        std::memcpy(output.data(), string.data(), string.size());
    } catch (...) {
    }

    return output;
}

auto TranslateServices(
    const blockchain::Type chain,
    [[maybe_unused]] const ProtocolVersion version,
    const UnallocatedSet<p2p::Service>& input) noexcept
    -> UnallocatedSet<bitcoin::Service>
{
    auto output = UnallocatedSet<bitcoin::Service>{};
    const auto& data = params::get(chain);
    std::for_each(
        std::begin(input), std::end(input), [&](const auto& in) -> void {
            if (auto value = data.TranslateService(in); value) {
                output.emplace(*value);
            }
        });

    return output;
}

auto TranslateServices(
    const blockchain::Type chain,
    [[maybe_unused]] const ProtocolVersion version,
    const UnallocatedSet<bitcoin::Service>& input) noexcept
    -> UnallocatedSet<p2p::Service>
{
    UnallocatedSet<p2p::Service> output{};
    const auto& data = params::get(chain);
    std::for_each(
        std::begin(input), std::end(input), [&](const auto& in) -> void {
            if (auto value = data.TranslateService(in); value) {
                output.emplace(*value);
            }
        });

    return output;
}
}  // namespace opentxs::blockchain::p2p::bitcoin
