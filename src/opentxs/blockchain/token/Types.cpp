// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/token/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <array>
#include <cstddef>
#include <exception>
#include <span>
#include <utility>

#include "core/FixedByteArray.tpp"  // IWYU pragma: keep
#include "internal/util/Bytes.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/token/Descriptor.hpp"
#include "opentxs/blockchain/token/TokenType.hpp"  // IWYU pragma: keep
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/core/UnitType.hpp"  // IWYU pragma: keep

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
            {trc10, "TRC-10"sv},
            {bep2, "BEP-2"sv},
            {bsc20, "BSC-20"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "invalid token type"sv;
    }
}

constexpr auto make_id(
    blockchain::Type chain,
    token::Type type,
    std::string_view hex)
{
    static_assert(test_decode_hex());

    auto out = Descriptor{chain, type};

    if (false == decode_hex(hex, out.id_.get())) { std::terminate(); }

    return out;
};

constexpr auto unit_to_token_map_ = [] {
    using opentxs::blockchain::Type;
    using enum opentxs::UnitType;
    using enum opentxs::blockchain::token::Type;
    using T = opentxs::UnitType;
    using U = Descriptor;
    constexpr auto N = 1793_uz;

    return frozen::unordered_map<T, U, N>{std::array<std::pair<T, U>, N>{
#include "opentxs/blockchain/token/unit_to_token.inc"  // IWYU pragma: keep
    }};
}();

auto token_to_unit(const Descriptor& in) noexcept -> opentxs::UnitType
{
    struct Hasher {
        constexpr auto operator()(const Descriptor& in, std::size_t seed)
            const noexcept -> std::size_t
        {
            auto out = frozen::elsa<decltype(in.host_)>{}(in.host_, seed);
            out = frozen::elsa<decltype(in.type_)>{}(in.type_, out);
            const auto id = in.id_.get();

            for (auto n = 0_uz, stop = id.size(); n < stop; ++n) {
                out = frozen::elsa<char>{}(std::to_integer<char>(id[n]), out);
            }

            return out;
        }
    };
    struct Equal {
        constexpr auto operator()(const Descriptor& lhs, const Descriptor& rhs)
            const noexcept -> bool
        {
            if (lhs.host_ != rhs.host_) { return false; }

            if (lhs.type_ != rhs.type_) { return false; }

            const auto lID = lhs.id_.get();
            const auto rID = rhs.id_.get();

            for (auto n = 0_uz; n < lID.size(); ++n) {
                if (lID[n] != rID[n]) { return false; }
            }

            return true;
        }
    };
    constexpr auto hasher = Hasher{};
    constexpr auto equal = Equal{};
    constexpr auto N = unit_to_token_map_.size();
    static constexpr auto map = frozen::
        make_unordered_map<Descriptor, opentxs::UnitType, N, Hasher, Equal>(
            [] {
                const auto& source = unit_to_token_map_;
                auto items =
                    std::array<std::pair<Descriptor, opentxs::UnitType>, N>{};
                auto o = items.begin();  // NOLINT(readability-qualified-auto)
                const auto* i = source.cbegin();

                for (const auto* end = source.end(); i != end; ++i, ++o) {
                    o->first = i->second;
                    o->second = i->first;
                }

                return items;
            }(),
            hasher,
            equal);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return opentxs::UnitType::Error;
    }
}

auto unit_to_token(opentxs::UnitType in) noexcept -> const Descriptor&
{
    const auto& map = unit_to_token_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        static constexpr auto blank = make_id(
            blockchain::Type::UnknownBlockchain, token::Type::null, ""sv);

        return blank;
    }
}
}  // namespace opentxs::blockchain::token
