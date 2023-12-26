// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/flat_map_types.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/table.hpp>

#include "opentxs/network/blockchain/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <functional>  // IWYU pragma: keep
#include <utility>

#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"     // IWYU pragma: keep
#include "opentxs/network/blockchain/Protocol.hpp"   // IWYU pragma: keep
#include "opentxs/network/blockchain/Subchain.hpp"   // IWYU pragma: keep
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep

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

        return "unknown blockchain::Protocol"sv;
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

        return "unknown blockchain::Subchain"sv;
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

        return "unknown blockchain::Transport"sv;
    }
}
}  // namespace opentxs::network::blockchain
