// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

#include "internal/blockchain/Params.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/Container.hpp"

namespace opentxs::blockchain::p2p::bitcoin
{
const CommandMap command_map_{
    {Command::addr, "addr"},
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
    {Command::sendcmpct, "sendcmpct"},
    {Command::sendheaders, "sendheaders"},
    {Command::submitorder, "submitorder"},
    {Command::tx, "tx"},
    {Command::verack, "verack"},
    {Command::version, "version"},
};
const CommandReverseMap command_reverse_map_{reverse_map(command_map_)};

const OTData AddressVersion::cjdns_prefix_{
    Data::Factory("0xfc", Data::Mode::Hex)};
const OTData AddressVersion::ipv4_prefix_{
    Data::Factory("0x00000000000000000000ffff", Data::Mode::Hex)};
const OTData AddressVersion::onion_prefix_{
    Data::Factory("0xfd87d87eeb43", Data::Mode::Hex)};

AddressVersion::AddressVersion(
    const std::pmr::set<bitcoin::Service>& services,
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
    const internal::Address& address) noexcept
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
            auto encoded{ipv4_prefix_};
            encoded += bytes;

            OT_ASSERT(output.size() == encoded->size());

            std::memcpy(output.data(), encoded->data(), output.size());
        } break;
        case Network::onion2: {
            auto encoded{onion_prefix_};
            encoded += bytes;

            OT_ASSERT(output.size() == encoded->size());

            std::memcpy(output.data(), encoded->data(), output.size());
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

auto BitcoinString(const std::string& in) noexcept -> OTData
{
    const auto size = CompactSize(in.size()).Encode();
    auto output = Data::Factory(size.data(), size.size());

    if (false == in.empty()) { output->Concatenate(in.data(), in.size()); }

    return output;
}

auto CommandName(const Command command) noexcept -> std::string
{
    try {
        return command_map_.at(command);
    } catch (...) {
        return {};
    }
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
        const std::string raw{
            reinterpret_cast<const char*>(bytes.data()), bytes.size()};
        const auto command = std::string{raw.c_str()};

        return command_reverse_map_.at(command);
    } catch (...) {

        return Command::unknown;
    }
}

auto GetServiceBytes(const std::pmr::set<bitcoin::Service>& services) noexcept
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
    const BitVector8 data) noexcept -> std::pmr::set<bitcoin::Service>
{
    if (0 == data) { return {}; }

    auto output = std::pmr::set<bitcoin::Service>{};
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

auto SerializeCommand(const Command command) noexcept -> CommandField
{
    CommandField output{};

    try {
        const auto string = CommandName(command);

        OT_ASSERT(output.size() >= string.size());

        std::memcpy(output.data(), string.data(), string.size());
    } catch (...) {
    }

    return output;
}

auto TranslateServices(
    const blockchain::Type chain,
    [[maybe_unused]] const ProtocolVersion version,
    const std::pmr::set<p2p::Service>& input) noexcept
    -> std::pmr::set<bitcoin::Service>
{
    using InnerMap = std::pmr::map<p2p::Service, bitcoin::Service>;
    using Map = std::pmr::map<blockchain::Type, InnerMap>;

    static std::mutex lock_{};
    static auto cache = Map{};
    auto it{cache.end()};

    {
        Lock lock(lock_);
        it = cache.find(chain);

        if (cache.end() == it) {
            const auto& bits = params::Data::Services().at(chain);
            auto [it2, added] = cache.emplace(chain, InnerMap{});

            OT_ASSERT(added);

            for (const auto& [s, b] : bits) { it2->second.emplace(b, s); }

            it = it2;
        }
    }

    OT_ASSERT(cache.end() != it);

    auto output = std::pmr::set<bitcoin::Service>{};
    const auto& map = it->second;

    std::for_each(
        std::begin(input),
        std::end(input),
        [&output, &map](const auto& in) -> void {
            try {
                output.emplace(map.at(in));
            } catch (...) {
            }
        });

    return output;
}

auto TranslateServices(
    const blockchain::Type chain,
    [[maybe_unused]] const ProtocolVersion version,
    const std::pmr::set<bitcoin::Service>& input) noexcept
    -> std::pmr::set<p2p::Service>
{
    std::pmr::set<p2p::Service> output{};
    std::for_each(
        std::begin(input),
        std::end(input),
        [&output, chain](const auto& in) -> void {
            try {
                output.emplace(params::Data::Services().at(chain).at(in));
            } catch (...) {
            }
        });

    return output;
}
}  // namespace opentxs::blockchain::p2p::bitcoin
