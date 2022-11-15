// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "opentxs/crypto/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <utility>

#include "opentxs/crypto/SeedStyle.hpp"

namespace opentxs::crypto
{
using namespace std::literals;

auto print(SeedStyle in) noexcept -> std::string_view
{
    using enum SeedStyle;
    static constexpr auto map =
        frozen::make_unordered_map<SeedStyle, std::string_view>({
            {Error, "invalid"sv},
            {BIP32, "BIP-32"sv},
            {BIP39, "BIP-39"sv},
            {PKT, "pktwallet"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown crypto::SeedStyle"sv;
    }
}
}  // namespace opentxs::crypto
