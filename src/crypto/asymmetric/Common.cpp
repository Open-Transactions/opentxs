// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/crypto/asymmetric/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <string_view>
#include <utility>

#include "opentxs/crypto/asymmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Mode.hpp"       // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Role.hpp"       // IWYU pragma: keep

namespace opentxs::crypto::asymmetric
{
using namespace std::literals;

auto print(Algorithm in) noexcept -> std::string_view
{
    using Type = Algorithm;
    static constexpr auto map =
        frozen::make_unordered_map<Type, std::string_view>({
            {Type::Error, "error"sv},
            {Type::Null, "Null"sv},
            {Type::Legacy, "Legacy"sv},
            {Type::Secp256k1, "Secp256k1"sv},
            {Type::ED25519, "ED25519"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown crypto::asymmetric::Algorithm"sv;
    }
}

auto print(Mode in) noexcept -> std::string_view
{
    using Type = Mode;
    static constexpr auto map =
        frozen::make_unordered_map<Type, std::string_view>({
            {Type::Error, "error"sv},
            {Type::Null, "Null"sv},
            {Type::Public, "Public"sv},
            {Type::Private, "Private"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown crypto::asymmetric::Mode"sv;
    }
}

auto print(Role in) noexcept -> std::string_view
{
    using Type = Role;
    static constexpr auto map =
        frozen::make_unordered_map<Type, std::string_view>({
            {Type::Error, "error"sv},
            {Type::Auth, "Auth"sv},
            {Type::Encrypt, "Encrypt"sv},
            {Type::Sign, "Sign"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown crypto::asymmetric::Role"sv;
    }
}
}  // namespace opentxs::crypto::asymmetric
