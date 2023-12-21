// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/flat_map_types.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/table.hpp>

#include "internal/network/blockchain/Types.hpp"  // IWYU pragma: associated
#include "opentxs/network/blockchain/Types.hpp"   // IWYU pragma: associated

#include <boost/endian/conversion.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <span>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/params/ChainData.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/blockchain/Protocol.hpp"   // IWYU pragma: keep
#include "opentxs/network/blockchain/Subchain.hpp"   // IWYU pragma: keep
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::network::blockchain
{
using namespace std::literals;

auto expected_address_size(Transport in) noexcept -> std::optional<std::size_t>
{
    using enum Transport;
    static constexpr auto map =
        frozen::make_unordered_map<Transport, std::size_t>({
            {ipv4, 4_uz},
            {ipv6, 16_uz},
            {onion2, 10_uz},
            {onion3, 32_uz},
            {eep, 32_uz},
            {cjdns, 16_uz},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return std::nullopt;
    }
}

auto print(Protocol in) noexcept -> std::string_view
{
    using enum Protocol;
    static constexpr auto map =
        frozen::make_unordered_map<Protocol, std::string_view>({
            {opentxs, "opentxs"sv},
            {bitcoin, "bitcoin"sv},
            {ethereum, "ethereum"sv},
            {unknown_protocol, "unknown"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid Protocol: ")(
            static_cast<std::uint8_t>(in))
            .Abort();
    }
}

auto print(Subchain in) noexcept -> std::string_view
{
    using enum Subchain;
    static constexpr auto map =
        frozen::make_unordered_map<Subchain, std::string_view>({
            {invalid, "invalid"sv},
            {primary, "primary"sv},
            {testnet1, "testnet1"sv},
            {testnet2, "testnet2"sv},
            {testnet3, "testnet3"sv},
            {testnet4, "testnet4"sv},
            {chipnet, "chipnet"sv},
            {scalenet, "scalenet"sv},
            {signet, "signet"sv},
            {regtest, "regtest"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid Subchain: ")(
            static_cast<std::uint8_t>(in))
            .Abort();
    }
}

auto print(Transport in) noexcept -> std::string_view
{
    using enum Transport;
    static constexpr auto map =
        frozen::make_unordered_map<Transport, std::string_view>({
            {invalid, "invalid"sv},
            {ipv4, "ipv4"sv},
            {ipv6, "ipv6"sv},
            {onion2, "onion2"sv},
            {onion3, "onion3"sv},
            {eep, "eep"sv},
            {cjdns, "cjdns"sv},
            {zmq, "zmq"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid Network: ")(
            static_cast<std::uint8_t>(in))
            .Abort();
    }
}
}  // namespace opentxs::network::blockchain

namespace opentxs::network::blockchain
{
auto decode(const zeromq::Message& in) noexcept -> opentxs::blockchain::Type
{
    using enum opentxs::blockchain::Type;
    using namespace opentxs::blockchain::params;
    static const auto map = [] {
        using Key = opentxs::blockchain::params::ChainData::ZMQParams;
        using Value = opentxs::blockchain::Type;
        auto out = boost::unordered_flat_map<Key, Value>{};

        for (const auto chain : chains()) {
            const auto& data = get(chain);
            out.try_emplace(data.ZMQ(), chain);
        }

        out.reserve(out.size());

        return out;
    }();

    try {
        const auto payload = in.Payload();

        if (auto count = payload.size(); 5_uz > count) {
            const auto error =
                UnallocatedCString{
                    "expected at least 5 frames in payload but only have "}
                    .append(std::to_string(count));

            throw std::runtime_error{error};
        }

        const auto key = std::make_pair(
            payload[1].as<Bip44Type>(), payload[2].as<Subchain>());

        if (const auto i = map.find(key); map.end() != i) {

            return i->second;
        } else {
            const auto error =
                UnallocatedCString{
                    "unable to decode combination of bip44 type "}
                    .append(
                        std::to_string(static_cast<std::uint32_t>(key.first)))
                    .append(" and subchain ")
                    .append(
                        std::to_string(static_cast<std::uint8_t>(key.second)));

            throw std::runtime_error{error};
        }
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return UnknownBlockchain;
    }
}

auto encode(opentxs::blockchain::Type chain, zeromq::Message& out) noexcept
    -> void
{
    auto [bip44, subchain] = opentxs::blockchain::params::get(chain).ZMQ();
    static_assert(sizeof(subchain) == sizeof(std::uint8_t));
    boost::endian::native_to_little_inplace(bip44);
    out.AddFrame(bip44);
    out.AddFrame(subchain);
}
}  // namespace opentxs::network::blockchain
