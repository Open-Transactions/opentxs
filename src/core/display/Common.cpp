// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/display/Definition.hpp"  // IWYU pragma: associated

#include <ankerl/unordered_dense.h>
#include <optional>
#include <utility>

#include "internal/core/display/Factory.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UnitType.hpp"       // IWYU pragma: keep
#include "opentxs/core/display/Scale.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Literals.hpp"

namespace opentxs::display
{
auto to_scale(int value) noexcept -> SpecifiedScale
{
    if (0 > value) {

        return std::nullopt;
    } else {

        return static_cast<ScaleIndex>(value);
    }
}

auto GetDefinition(UnitType in) noexcept -> const Definition&
{
    using factory::DisplayDefinition;
    using factory::DisplayScale;
    static const auto map = ankerl::unordered_dense::map<UnitType, Definition>{
        {UnitType::Btc,
         DisplayDefinition(
             u8"BTC"_cstr,
             0,
             4,
             {
                 {u8"BTC"_cstr, DisplayScale("", u8"₿"_cstr, {{10, 8}}, 0, 8)},
                 {u8"mBTC"_cstr,
                  DisplayScale("", u8"mBTC"_cstr, {{10, 5}}, 0, 5)},
                 {u8"bits"_cstr,
                  DisplayScale("", u8"bits"_cstr, {{10, 2}}, 0, 2)},
                 {u8"μBTC"_cstr,
                  DisplayScale("", u8"μBTC"_cstr, {{10, 2}}, 0, 2)},
                 {u8"satoshi"_cstr,
                  DisplayScale("", u8"sats"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tnbtc,
         DisplayDefinition(
             u8"tnBTC"_cstr,
             0,
             4,
             {
                 {u8"BTC"_cstr,
                  DisplayScale("", u8"tBTC"_cstr, {{10, 8}}, 0, 8)},
                 {u8"mBTC"_cstr,
                  DisplayScale("", u8"mBTC"_cstr, {{10, 5}}, 0, 5)},
                 {u8"bits"_cstr,
                  DisplayScale("", u8"bits"_cstr, {{10, 2}}, 0, 2)},
                 {u8"μBTC"_cstr,
                  DisplayScale("", u8"μBTC"_cstr, {{10, 2}}, 0, 2)},
                 {u8"satoshi"_cstr,
                  DisplayScale("", u8"sats"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Bch,
         DisplayDefinition(
             u8"BCH"_cstr,
             0,
             4,
             {
                 {u8"BCH"_cstr,
                  DisplayScale("", u8"BCH"_cstr, {{10, 8}}, 0, 8)},
                 {u8"mBCH"_cstr,
                  DisplayScale("", u8"mBCH"_cstr, {{10, 5}}, 0, 5)},
                 {u8"bits"_cstr,
                  DisplayScale("", u8"bits"_cstr, {{10, 2}}, 0, 2)},
                 {u8"μBCH"_cstr,
                  DisplayScale("", u8"μBCH"_cstr, {{10, 2}}, 0, 2)},
                 {u8"satoshi"_cstr,
                  DisplayScale("", u8"sats"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tnbch,
         DisplayDefinition(
             u8"tnBCH"_cstr,
             0,
             4,
             {
                 {u8"BCH"_cstr,
                  DisplayScale("", u8"tBCH"_cstr, {{10, 8}}, 0, 8)},
                 {u8"mBCH"_cstr,
                  DisplayScale("", u8"mBCH"_cstr, {{10, 5}}, 0, 5)},
                 {u8"bits"_cstr,
                  DisplayScale("", u8"bits"_cstr, {{10, 2}}, 0, 2)},
                 {u8"μBCH"_cstr,
                  DisplayScale("", u8"μBCH"_cstr, {{10, 2}}, 0, 2)},
                 {u8"satoshi"_cstr,
                  DisplayScale("", u8"sats"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Eth,
         DisplayDefinition(
             u8"ETH"_cstr,
             0,
             9,
             {
                 {u8"Gether"_cstr,
                  DisplayScale("", u8"Gether"_cstr, {{10, 27}}, 0, 18)},
                 {u8"Mether"_cstr,
                  DisplayScale("", u8"Mether"_cstr, {{10, 24}}, 0, 15)},
                 {u8"Kether"_cstr,
                  DisplayScale("", u8"Kether"_cstr, {{10, 21}}, 0, 12)},
                 {u8"ETH"_cstr,
                  DisplayScale("", u8"ETH"_cstr, {{10, 18}}, 0, 9)},
                 {u8"Finney"_cstr,
                  DisplayScale("", u8"Finney"_cstr, {{10, 15}}, 0, 6)},
                 {u8"Szabo"_cstr,
                  DisplayScale("", u8"Szabo"_cstr, {{10, 12}}, 0, 3)},
                 {u8"Gwei"_cstr,
                  DisplayScale("", u8"Gwei"_cstr, {{10, 9}}, 0, 0)},
                 {u8"Mwei"_cstr,
                  DisplayScale("", u8"Mwei"_cstr, {{10, 6}}, 0, 0)},
                 {u8"Kwei"_cstr,
                  DisplayScale("", u8"Kwei"_cstr, {{10, 3}}, 0, 0)},
                 {u8"wei"_cstr,
                  DisplayScale("", u8"wei"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ethereum_ropsten,
         DisplayDefinition(
             u8"ETHropsten"_cstr,
             0,
             9,
             {
                 {u8"Gether"_cstr,
                  DisplayScale("", u8"Gether"_cstr, {{10, 27}}, 0, 18)},
                 {u8"Mether"_cstr,
                  DisplayScale("", u8"Mether"_cstr, {{10, 24}}, 0, 15)},
                 {u8"Kether"_cstr,
                  DisplayScale("", u8"Kether"_cstr, {{10, 21}}, 0, 12)},
                 {u8"ETH"_cstr,
                  DisplayScale("", u8"ETH"_cstr, {{10, 18}}, 0, 9)},
                 {u8"Finney"_cstr,
                  DisplayScale("", u8"Finney"_cstr, {{10, 15}}, 0, 6)},
                 {u8"Szabo"_cstr,
                  DisplayScale("", u8"Szabo"_cstr, {{10, 12}}, 0, 3)},
                 {u8"Gwei"_cstr,
                  DisplayScale("", u8"Gwei"_cstr, {{10, 9}}, 0, 0)},
                 {u8"Mwei"_cstr,
                  DisplayScale("", u8"Mwei"_cstr, {{10, 6}}, 0, 0)},
                 {u8"Kwei"_cstr,
                  DisplayScale("", u8"Kwei"_cstr, {{10, 3}}, 0, 0)},
                 {u8"wei"_cstr,
                  DisplayScale("", u8"wei"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ethereum_goerli,
         DisplayDefinition(
             u8"ETHgoerli"_cstr,
             0,
             9,
             {
                 {u8"Gether"_cstr,
                  DisplayScale("", u8"Gether"_cstr, {{10, 27}}, 0, 18)},
                 {u8"Mether"_cstr,
                  DisplayScale("", u8"Mether"_cstr, {{10, 24}}, 0, 15)},
                 {u8"Kether"_cstr,
                  DisplayScale("", u8"Kether"_cstr, {{10, 21}}, 0, 12)},
                 {u8"ETH"_cstr,
                  DisplayScale("", u8"ETH"_cstr, {{10, 18}}, 0, 9)},
                 {u8"Finney"_cstr,
                  DisplayScale("", u8"Finney"_cstr, {{10, 15}}, 0, 6)},
                 {u8"Szabo"_cstr,
                  DisplayScale("", u8"Szabo"_cstr, {{10, 12}}, 0, 3)},
                 {u8"Gwei"_cstr,
                  DisplayScale("", u8"Gwei"_cstr, {{10, 9}}, 0, 0)},
                 {u8"Mwei"_cstr,
                  DisplayScale("", u8"Mwei"_cstr, {{10, 6}}, 0, 0)},
                 {u8"Kwei"_cstr,
                  DisplayScale("", u8"Kwei"_cstr, {{10, 3}}, 0, 0)},
                 {u8"wei"_cstr,
                  DisplayScale("", u8"wei"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ethereum_sepolia,
         DisplayDefinition(
             u8"ETHsepolia"_cstr,
             0,
             9,
             {
                 {u8"Gether"_cstr,
                  DisplayScale("", u8"Gether"_cstr, {{10, 27}}, 0, 18)},
                 {u8"Mether"_cstr,
                  DisplayScale("", u8"Mether"_cstr, {{10, 24}}, 0, 15)},
                 {u8"Kether"_cstr,
                  DisplayScale("", u8"Kether"_cstr, {{10, 21}}, 0, 12)},
                 {u8"ETH"_cstr,
                  DisplayScale("", u8"ETH"_cstr, {{10, 18}}, 0, 9)},
                 {u8"Finney"_cstr,
                  DisplayScale("", u8"Finney"_cstr, {{10, 15}}, 0, 6)},
                 {u8"Szabo"_cstr,
                  DisplayScale("", u8"Szabo"_cstr, {{10, 12}}, 0, 3)},
                 {u8"Gwei"_cstr,
                  DisplayScale("", u8"Gwei"_cstr, {{10, 9}}, 0, 0)},
                 {u8"Mwei"_cstr,
                  DisplayScale("", u8"Mwei"_cstr, {{10, 6}}, 0, 0)},
                 {u8"Kwei"_cstr,
                  DisplayScale("", u8"Kwei"_cstr, {{10, 3}}, 0, 0)},
                 {u8"wei"_cstr,
                  DisplayScale("", u8"wei"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ethereum_holesovice,
         DisplayDefinition(
             u8"ETHholesovice"_cstr,
             0,
             9,
             {
                 {u8"Gether"_cstr,
                  DisplayScale("", u8"Gether"_cstr, {{10, 27}}, 0, 18)},
                 {u8"Mether"_cstr,
                  DisplayScale("", u8"Mether"_cstr, {{10, 24}}, 0, 15)},
                 {u8"Kether"_cstr,
                  DisplayScale("", u8"Kether"_cstr, {{10, 21}}, 0, 12)},
                 {u8"ETH"_cstr,
                  DisplayScale("", u8"ETH"_cstr, {{10, 18}}, 0, 9)},
                 {u8"Finney"_cstr,
                  DisplayScale("", u8"Finney"_cstr, {{10, 15}}, 0, 6)},
                 {u8"Szabo"_cstr,
                  DisplayScale("", u8"Szabo"_cstr, {{10, 12}}, 0, 3)},
                 {u8"Gwei"_cstr,
                  DisplayScale("", u8"Gwei"_cstr, {{10, 9}}, 0, 0)},
                 {u8"Mwei"_cstr,
                  DisplayScale("", u8"Mwei"_cstr, {{10, 6}}, 0, 0)},
                 {u8"Kwei"_cstr,
                  DisplayScale("", u8"Kwei"_cstr, {{10, 3}}, 0, 0)},
                 {u8"wei"_cstr,
                  DisplayScale("", u8"wei"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ltc,
         DisplayDefinition(
             u8"LTC"_cstr,
             0,
             4,
             {
                 {u8"LTC"_cstr, DisplayScale("", u8"Ł"_cstr, {{10, 8}}, 0, 6)},
                 {u8"mLTC"_cstr,
                  DisplayScale("", u8"mŁ"_cstr, {{10, 5}}, 0, 3)},
                 {u8"μLTC"_cstr,
                  DisplayScale("", u8"μŁ"_cstr, {{10, 2}}, 0, 0)},
                 {u8"photons"_cstr,
                  DisplayScale("", u8"photons"_cstr, {{10, 2}}, 0, 0)},
             })},
        {UnitType::Tnltx,
         DisplayDefinition(
             u8"tnLTC"_cstr,
             0,
             5,
             {
                 {u8"LTC"_cstr, DisplayScale("", u8"Ł"_cstr, {{10, 8}}, 0, 6)},
                 {u8"mLTC"_cstr,
                  DisplayScale("", u8"mŁ"_cstr, {{10, 5}}, 0, 3)},
                 {u8"μLTC"_cstr,
                  DisplayScale("", u8"μŁ"_cstr, {{10, 2}}, 0, 0)},
                 {u8"photons"_cstr,
                  DisplayScale("", u8"photons"_cstr, {{10, 2}}, 0, 0)},
                 {u8"satoshi"_cstr,
                  DisplayScale("", u8"sats"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Pkt,
         DisplayDefinition(
             u8"PKT"_cstr,
             0,
             5,
             {
                 {u8"PKT"_cstr,
                  DisplayScale("", u8"PKT"_cstr, {{2, 30}}, 0, 11)},
                 {u8"mPKT"_cstr,
                  DisplayScale("", u8"mPKT"_cstr, {{2, 30}, {10, -3}}, 0, 8)},
                 {u8"μPKT"_cstr,
                  DisplayScale("", u8"μPKT"_cstr, {{2, 30}, {10, -6}}, 0, 5)},
                 {u8"nPKT"_cstr,
                  DisplayScale("", u8"nPKT"_cstr, {{2, 30}, {10, -9}}, 0, 2)},
                 {u8"pack"_cstr,
                  DisplayScale("", u8"pack"_cstr, {{10, 0}}, 0, 0)},
                 {u8"satoshi"_cstr,
                  DisplayScale("", u8"sats"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tnpkt,
         DisplayDefinition(
             u8"tnPKT"_cstr,
             0,
             4,
             {
                 {u8"PKT"_cstr,
                  DisplayScale("", u8"PKT"_cstr, {{2, 30}}, 0, 11)},
                 {u8"mPKT"_cstr,
                  DisplayScale("", u8"mPKT"_cstr, {{2, 30}, {10, -3}}, 0, 8)},
                 {u8"μPKT"_cstr,
                  DisplayScale("", u8"μPKT"_cstr, {{2, 30}, {10, -6}}, 0, 5)},
                 {u8"nPKT"_cstr,
                  DisplayScale("", u8"nPKT"_cstr, {{2, 30}, {10, -9}}, 0, 2)},
                 {u8"pack"_cstr,
                  DisplayScale("", u8"pack"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Bsv,
         DisplayDefinition(
             u8"BSV"_cstr,
             0,
             4,
             {
                 {u8"BSV"_cstr,
                  DisplayScale("", u8"BSV"_cstr, {{10, 8}}, 0, 8)},
                 {u8"mBSV"_cstr,
                  DisplayScale("", u8"mBSV"_cstr, {{10, 5}}, 0, 5)},
                 {u8"bits"_cstr,
                  DisplayScale("", u8"bits"_cstr, {{10, 2}}, 0, 2)},
                 {u8"μBSV"_cstr,
                  DisplayScale("", u8"μBSV"_cstr, {{10, 2}}, 0, 2)},
                 {u8"satoshi"_cstr,
                  DisplayScale("", u8"sats"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tnbsv,
         DisplayDefinition(
             u8"tnBSV"_cstr,
             0,
             4,
             {
                 {u8"BSV"_cstr,
                  DisplayScale("", u8"tBSV"_cstr, {{10, 8}}, 0, 8)},
                 {u8"mBSV"_cstr,
                  DisplayScale("", u8"mBSV"_cstr, {{10, 5}}, 0, 5)},
                 {u8"bits"_cstr,
                  DisplayScale("", u8"bits"_cstr, {{10, 2}}, 0, 2)},
                 {u8"μBSV"_cstr,
                  DisplayScale("", u8"μBSV"_cstr, {{10, 2}}, 0, 2)},
                 {u8"satoshi"_cstr,
                  DisplayScale("", u8"sats"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Xec,
         DisplayDefinition(
             u8"XEC"_cstr,
             2,
             3,
             {
                 {u8"MXEC"_cstr,
                  DisplayScale("", u8"MXEC"_cstr, {{10, 8}}, 0, 8)},
                 {u8"kXEC"_cstr,
                  DisplayScale("", u8"kXEC"_cstr, {{10, 5}}, 0, 5)},
                 {u8"XEC"_cstr,
                  DisplayScale("", u8"XEC"_cstr, {{10, 2}}, 0, 2)},
                 {u8"satoshi"_cstr,
                  DisplayScale("", u8"sats"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::TnXec,
         DisplayDefinition(
             u8"tnXEC"_cstr,
             2,
             3,
             {
                 {u8"MtXEC"_cstr,
                  DisplayScale("", u8"MtXEC"_cstr, {{10, 8}}, 0, 8)},
                 {u8"ktXEC"_cstr,
                  DisplayScale("", u8"ktXEC"_cstr, {{10, 5}}, 0, 5)},
                 {u8"tXEC"_cstr,
                  DisplayScale("", u8"tXEC"_cstr, {{10, 2}}, 0, 2)},
                 {u8"satoshi"_cstr,
                  DisplayScale("", u8"sats"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Regtest,
         DisplayDefinition(
             u8"UNITTEST"_cstr,
             0,
             0,
             {
                 {u8"Unit"_cstr,
                  DisplayScale("", u8"units"_cstr, {{10, 8}}, 0, 8)},
             })},
        {UnitType::Usd,
         DisplayDefinition(
             u8"USD"_cstr,
             0,
             0,
             {
                 {u8"dollars"_cstr,
                  DisplayScale(u8"$"_cstr, u8""_cstr, {{10, 0}}, 2, 3)},
                 {u8"cents"_cstr,
                  DisplayScale(u8""_cstr, u8"¢"_cstr, {{10, -2}}, 0, 1)},
                 {u8"millions"_cstr,
                  DisplayScale(u8"$"_cstr, u8"MM"_cstr, {{10, 6}}, 0, 9)},
                 {u8"mills"_cstr,
                  DisplayScale(u8""_cstr, u8"₥"_cstr, {{10, -3}}, 0, 0)},
             })},
        {UnitType::Cspr,
         DisplayDefinition(
             u8"CSPR"_cstr,
             0,
             1,
             {
                 {u8"CSPR"_cstr,
                  DisplayScale(u8""_cstr, u8""_cstr, {{10, 9}}, 0, 9)},
                 {u8"mote"_cstr,
                  DisplayScale(u8""_cstr, u8""_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::TnCspr,
         DisplayDefinition(
             u8"tCSPR"_cstr,
             0,
             1,
             {
                 {u8"tCSPR"_cstr,
                  DisplayScale(u8""_cstr, u8""_cstr, {{10, 9}}, 0, 9)},
                 {u8"mote"_cstr,
                  DisplayScale(u8""_cstr, u8""_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tn4bch,
         DisplayDefinition(
             u8"tn4BCH"_cstr,
             0,
             4,
             {
                 {u8"BCH"_cstr,
                  DisplayScale("", u8"tBCH"_cstr, {{10, 8}}, 0, 8)},
                 {u8"mBCH"_cstr,
                  DisplayScale("", u8"mBCH"_cstr, {{10, 5}}, 0, 5)},
                 {u8"bits"_cstr,
                  DisplayScale("", u8"bits"_cstr, {{10, 2}}, 0, 2)},
                 {u8"μBCH"_cstr,
                  DisplayScale("", u8"μBCH"_cstr, {{10, 2}}, 0, 2)},
                 {u8"satoshi"_cstr,
                  DisplayScale("", u8"sats"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Dash,
         DisplayDefinition(
             u8"DASH"_cstr,
             0,
             4,
             {
                 {u8"DASH"_cstr, DisplayScale("", u8"₿"_cstr, {{10, 8}}, 0, 8)},
                 {u8"mDASH"_cstr,
                  DisplayScale("", u8"mDASH"_cstr, {{10, 5}}, 0, 5)},
                 {u8"bits"_cstr,
                  DisplayScale("", u8"bits"_cstr, {{10, 2}}, 0, 2)},
                 {u8"μDASH"_cstr,
                  DisplayScale("", u8"μDASH"_cstr, {{10, 2}}, 0, 2)},
                 {u8"satoshi"_cstr,
                  DisplayScale("", u8"sats"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tndash,
         DisplayDefinition(
             u8"tnDASH"_cstr,
             0,
             4,
             {
                 {u8"DASH"_cstr,
                  DisplayScale("", u8"tDASH"_cstr, {{10, 8}}, 0, 8)},
                 {u8"mDASH"_cstr,
                  DisplayScale("", u8"mDASH"_cstr, {{10, 5}}, 0, 5)},
                 {u8"bits"_cstr,
                  DisplayScale("", u8"bits"_cstr, {{10, 2}}, 0, 2)},
                 {u8"μDASH"_cstr,
                  DisplayScale("", u8"μDASH"_cstr, {{10, 2}}, 0, 2)},
                 {u8"satoshi"_cstr,
                  DisplayScale("", u8"sats"_cstr, {{10, 0}}, 0, 0)},
             })},
    };

    if (auto i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        static const auto defaultDefinition = Definition{};

        return defaultDefinition;
    }
}
}  // namespace opentxs::display
