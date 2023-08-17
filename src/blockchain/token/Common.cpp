// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/token/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <utility>

#include "opentxs/blockchain/token/TokenType.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::token
{
using namespace std::literals;

auto print(Type in) noexcept -> std::string_view
{
    using enum Type;
    static constexpr auto map =
        frozen::make_unordered_map<Type, std::string_view>({
            {null, "null"sv},
            {slp, "SLP"sv},
            {cashtoken, "Cashtoken"sv},
            {erc20, "ERC-20"sv},
            {erc721, "ERC-721"sv},
            {erc1155, "ERC-1155"sv},
            {erc4626, "ERC-4626"sv},
            {eos, "EOS"sv},
            {nep5, "NEP-5"sv},
            {nep11, "NEP-11"sv},
            {nep17, "NEP-17"sv},
            {tzip7, "TZIP-7"sv},
            {tzip12, "TZIP-12"sv},
            {trc20, "TRC-20"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "invalid token type"sv;
    }
}
}  // namespace opentxs::blockchain::token
