// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/flat_map_types.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/table.hpp>

#include "opentxs/blockchain/Types.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <functional>
#include <iterator>
#include <optional>
#include <string_view>

#include "internal/blockchain/params/ChainData.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/UnitType.hpp"             // IWYU pragma: keep
#include "opentxs/blockchain/Category.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Type.hpp"      // IWYU pragma: keep
#include "opentxs/crypto/HashType.hpp"      // IWYU pragma: keep
#include "opentxs/display/Definition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain
{
using namespace std::literals;

auto associated_mainnet(Type type) noexcept -> Type
{
    try {
        return blockchain::params::get(type).AssociatedMainnet();
    } catch (...) {
        return Type::UnknownBlockchain;
    }
}

auto blockchain_to_unit(const blockchain::Type type) noexcept -> UnitType
{
    try {
        return blockchain::params::get(type).CurrencyType();
    } catch (...) {
        return UnitType::Unknown;
    }
}

auto category(Type type) noexcept -> Category
{
    try {
        return blockchain::params::get(type).Category();
    } catch (...) {
        return Category::unknown_category;
    }
}

auto defined_chains() noexcept -> std::span<const Type>
{
    static const auto data = [] {
        auto out = Vector<Type>{};
        const auto& chains = params::chains();
        out.reserve(chains.size());
        std::ranges::copy(chains, std::back_inserter(out));

        return out;
    }();

    return data;
}

auto display(Type type) noexcept -> const display::Definition&
{
    return display::GetDefinition(blockchain_to_unit(type));
}

auto has_segwit(const Type type) noexcept -> bool
{
    try {

        return params::get(type).SupportsSegwit();
    } catch (...) {

        return false;
    }
}

auto is_defined(Type in) noexcept -> bool
{
    return params::chains().contains(in);
}

auto is_descended_from(Type descendant, Type ancestor) noexcept -> bool
{
    static const auto map = [] {
        using Descendants = boost::container::flat_set<blockchain::Type>;
        using Map = boost::container::flat_map<blockchain::Type, Descendants>;
        auto out = Map{};

        for (const auto& chain : params::chains()) {
            out[chain].emplace(chain);
            auto parent = std::optional<blockchain::Type>{std::nullopt};
            auto current{chain};

            for (;;) {
                if (parent = params::get(current).ForkedFrom(); parent) {
                    current = *parent;
                    out[current].emplace(chain);
                } else {

                    break;
                }
            }
        }

        for (auto& [_, value] : out) { value.shrink_to_fit(); }

        out.shrink_to_fit();

        return out;
    }();

    if (auto i = map.find(ancestor); map.end() != i) {

        return i->second.contains(descendant);
    } else {

        return false;
    }
}

auto is_supported(Type in) noexcept -> bool
{
    return params::supported().contains(in);
}

auto is_testnet(const Type type) noexcept -> bool
{
    try {

        return params::get(type).IsTestnet();
    } catch (...) {

        return false;
    }
}

auto print(Type in) noexcept -> std::string_view
{
    return print(blockchain_to_unit(in));
}

auto print(Category in) noexcept -> std::string_view
{
    using enum Category;
    static constexpr auto map =
        frozen::make_unordered_map<Category, std::string_view>({
            {unknown_category, "unknown category"sv},
            {output_based, "bitcoin derived"sv},
            {balance_based, "ethereum derived"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid Category: ")(value(in)).Abort();
    }
}

auto supported_chains() noexcept -> std::span<const Type>
{
    static const auto output = [] {
        auto out = Vector<Type>{};
        const auto& chains = params::supported();
        out.reserve(chains.size());
        std::ranges::copy(chains, std::back_inserter(out));

        return out;
    }();

    return output;
}

auto ticker_symbol(const Type type) noexcept -> UnallocatedCString
{
    return UnallocatedCString{display(type).ShortName()};
}

auto ticker_symbol(const Type type, alloc::Strategy alloc) noexcept -> CString
{
    return CString{display(type).ShortName(), alloc.result_};
}
}  // namespace opentxs::blockchain

namespace opentxs
{
auto unit_to_blockchain(const UnitType type) noexcept -> blockchain::Type
{
    static const auto map = [] {
        auto out = boost::unordered_flat_map<UnitType, blockchain::Type>{};

        for (const auto chain : blockchain::defined_chains()) {
            out.emplace(blockchain::params::get(chain).CurrencyType(), chain);
        }

        out.reserve(out.size());

        return out;
    }();

    if (auto i = map.find(type); map.end() != i) {

        return i->second;
    } else {

        return blockchain::Type::UnknownBlockchain;
    }
}
}  // namespace opentxs
