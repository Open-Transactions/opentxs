// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identifier/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <functional>  // IWYU pragma: keep
#include <utility>

#include "opentxs/contract/ContractType.hpp"      // IWYU pragma: keep
#include "opentxs/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Algorithm.hpp"       // IWYU pragma: keep
#include "opentxs/identifier/Type.hpp"            // IWYU pragma: keep

namespace opentxs::identifier
{
using namespace std::literals;

auto print(AccountSubtype in) noexcept -> std::string_view
{
    using enum AccountSubtype;
    static constexpr auto map =
        frozen::make_unordered_map<AccountSubtype, std::string_view>({
            {invalid_subtype, "invalid_subtype"sv},
            {custodial_account, "custodial_account"sv},
            {blockchain_account, "blockchain_account"sv},
            {blockchain_subaccount, "blockchain_subaccount"sv},
            {blockchain_subchain, "blockchain_subchain"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "invalid_subtype";
    }
}

auto print(Algorithm in) noexcept -> std::string_view
{
    using enum Algorithm;
    static constexpr auto map =
        frozen::make_unordered_map<Algorithm, std::string_view>({
            {invalid, "invalid"sv},
            {sha256, "sha256"sv},
            {blake2b160, "blake2b160"sv},
            {blake2b256, "blake2b256"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown";
    }
}

auto print(Type in) noexcept -> std::string_view
{
    using enum Type;
    static constexpr auto map =
        frozen::make_unordered_map<Type, std::string_view>({
            {invalid, "invalid"sv},
            {generic, "generic"sv},
            {nym, "nym"sv},
            {notary, "notary"sv},
            {unitdefinition, "unit definition"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown";
    }
}

auto translate(Type in) noexcept -> contract::Type
{
    using enum Type;
    static constexpr auto map =
        frozen::make_unordered_map<Type, contract::Type>({
            {invalid, contract::Type::invalid},
            {generic, contract::Type::invalid},
            {nym, contract::Type::nym},
            {notary, contract::Type::notary},
            {unitdefinition, contract::Type::unit},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return contract::Type::invalid;
    }
}
}  // namespace opentxs::identifier
