// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/blockchain/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <utility>

#include "opentxs/network/blockchain/Protocol.hpp"   // IWYU pragma: keep
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"

namespace opentxs::network::blockchain
{
using namespace std::literals;

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

auto print(Protocol in) noexcept -> std::string_view
{
    using enum Protocol;
    static constexpr auto map =
        frozen::make_unordered_map<Protocol, std::string_view>({
            {opentxs, "opentxs"sv},
            {bitcoin, "bitcoin"sv},
            {ethereum, "ethereum"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid Protocol: ")(
            static_cast<std::uint8_t>(in))
            .Abort();
    }
}
}  // namespace opentxs::network::blockchain
