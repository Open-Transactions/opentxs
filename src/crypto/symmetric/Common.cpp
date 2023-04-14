// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/crypto/symmetric/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <string_view>
#include <utility>

#include "opentxs/crypto/symmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/symmetric/Source.hpp"     // IWYU pragma: keep

namespace opentxs::crypto::symmetric
{
using namespace std::literals;

auto print(Algorithm in) noexcept -> std::string_view
{
    using Type = Algorithm;
    static constexpr auto map =
        frozen::make_unordered_map<Type, std::string_view>({
            {Type::Error, "error"sv},
            {Type::ChaCha20Poly1305, "ChaCha20Poly1305"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown crypto::symmetric::Algorithm"sv;
    }
}

auto print(Source in) noexcept -> std::string_view
{
    using Type = Source;
    static constexpr auto map =
        frozen::make_unordered_map<Type, std::string_view>({
            {Type::Error, "error"sv},
            {Type::Raw, "Raw"sv},
            {Type::ECDH, "ECDH"sv},
            {Type::Argon2i, "Argon2i"sv},
            {Type::Argon2id, "Argon2id"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown crypto::symmetric::Source"sv;
    }
}
}  // namespace opentxs::crypto::symmetric
