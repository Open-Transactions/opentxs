// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/peer/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <functional>  // IWYU pragma: keep
#include <utility>

#include "internal/core/contract/peer/PairEventType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/ConnectionInfoType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/ObjectType.hpp"   // IWYU pragma: keep
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/SecretType.hpp"   // IWYU pragma: keep

namespace opentxs::contract::peer
{
using namespace std::literals;

auto print(ConnectionInfoType in) noexcept -> std::string_view
{
    using enum ConnectionInfoType;
    static constexpr auto map =
        frozen::make_unordered_map<ConnectionInfoType, std::string_view>({
            {Error, "Error"sv},
            {Bitcoin, "Bitcoin"sv},
            {BtcRpc, "BtcRpc"sv},
            {BitMessage, "BitMessage"sv},
            {BitMessageRPC, "BitMessageRPC"sv},
            {SSH, "SSH"sv},
            {CJDNS, "CJDNS"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown ConnectionInfoType"sv;
    }
}

auto print(ObjectType in) noexcept -> std::string_view
{
    using enum ObjectType;
    static constexpr auto map =
        frozen::make_unordered_map<ObjectType, std::string_view>({
            {Error, "Error"sv},
            {Message, "Message"sv},
            {Request, "Request"sv},
            {Response, "Response"sv},
            {Payment, "Payment"sv},
            {Cash, "Cash"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown ObjectType"sv;
    }
}

auto print(RequestType in) noexcept -> std::string_view
{
    using enum RequestType;
    static constexpr auto map =
        frozen::make_unordered_map<RequestType, std::string_view>({
            {Error, "invalid"sv},
            {Bailment, "bailment"sv},
            {OutBailment, "outbailment"sv},
            {PendingBailment, "pending bailment"sv},
            {ConnectionInfo, "connection info"sv},
            {StoreSecret, "store secret"sv},
            {VerifiedClaim, "verified claim"sv},
            {Faucet, "faucet"sv},
            {Verification, "verification"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown RequestType"sv;
    }
}

auto print(SecretType in) noexcept -> std::string_view
{
    using enum SecretType;
    static constexpr auto map =
        frozen::make_unordered_map<SecretType, std::string_view>({
            {SecretType::Error, "Error"sv},
            {SecretType::Bip39, "Bip39"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown SecretType"sv;
    }
}
}  // namespace opentxs::contract::peer
