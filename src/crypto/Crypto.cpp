// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/crypto/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <utility>

#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStrength.hpp"
#include "opentxs/crypto/SeedStyle.hpp"

namespace opentxs::crypto
{
using namespace std::literals;

auto print(Language in) noexcept -> std::string_view
{
    using enum Language;
    static constexpr auto map =
        frozen::make_unordered_map<Language, std::string_view>({
            {none, "none"sv},
            {en, "English"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown crypto::Language"sv;
    }
}

auto print(SeedStyle in) noexcept -> std::string_view
{
    using enum SeedStyle;
    static constexpr auto map =
        frozen::make_unordered_map<SeedStyle, std::string_view>({
            {Error, "invalid"sv},
            {BIP32, "BIP-32"sv},
            {BIP39, "BIP-39"sv},
            {PKT, "Legacy pktwallet"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown crypto::SeedStyle"sv;
    }
}

auto print(SeedStrength in) noexcept -> std::string_view
{
    using enum SeedStrength;
    static constexpr auto map =
        frozen::make_unordered_map<SeedStrength, std::string_view>({
            {Twelve, "12 words"sv},
            {Fifteen, "15 words"sv},
            {Eighteen, "18 words"sv},
            {TwentyOne, "21 words"sv},
            {TwentyFour, "24 words"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown crypto::SeedStrength"sv;
    }
}
}  // namespace opentxs::crypto
