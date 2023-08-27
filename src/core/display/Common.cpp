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
        {UnitType::Xrp,
         DisplayDefinition(
             u8"XRP"_cstr,
             0,
             1,
             {
                 {u8"XRP"_cstr, DisplayScale("", "", {{10, 6}}, 0, 6)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
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
        {UnitType::erc20_eth_dao,
         DisplayDefinition(
             u8"DAO"_cstr,
             0,
             1,
             {
                 {u8"DAO"_cstr, DisplayScale("", "", {{10, 16}}, 0, 16)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Xem,
         DisplayDefinition(
             u8"XEM"_cstr,
             0,
             1,
             {
                 {u8"XEM"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
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
        {UnitType::Maid,
         DisplayDefinition(
             u8"MAID"_cstr,
             0,
             1,
             {
                 {u8"MAID"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_lsk,
         DisplayDefinition(
             u8"LML"_cstr,
             0,
             1,
             {
                 {u8"LML"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Doge,
         DisplayDefinition(
             u8"DOGE"_cstr,
             0,
             1,
             {
                 {u8"DOGE"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_dgd,
         DisplayDefinition(
             u8"DGD"_cstr,
             0,
             1,
             {
                 {u8"DGD"_cstr, DisplayScale("", "", {{10, 9}}, 0, 9)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Xmr,
         DisplayDefinition(
             u8"XMR"_cstr,
             0,
             1,
             {
                 {u8"XMR"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Waves,
         DisplayDefinition(
             u8"WAVES"_cstr,
             0,
             1,
             {
                 {u8"WAVES"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Nxt,
         DisplayDefinition(
             u8"NXT"_cstr,
             0,
             1,
             {
                 {u8"NXT"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Sc,
         DisplayDefinition(
             u8"SC"_cstr,
             0,
             1,
             {
                 {u8"SC"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Steem,
         DisplayDefinition(
             u8"STEEM"_cstr,
             0,
             1,
             {
                 {u8"STEEM"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_amp,
         DisplayDefinition(
             u8"AMP"_cstr,
             0,
             1,
             {
                 {u8"AMP"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Xlm,
         DisplayDefinition(
             u8"XLM"_cstr,
             0,
             1,
             {
                 {u8"XLM"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Fct,
         DisplayDefinition(
             u8"FCT"_cstr,
             0,
             1,
             {
                 {u8"FCT"_cstr, DisplayScale("", "", {{10, 8}}, 0, 8)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Bts,
         DisplayDefinition(
             u8"BTS"_cstr,
             0,
             1,
             {
                 {u8"BTS"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
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
        {UnitType::Tnxrp,
         DisplayDefinition(
             u8"tnXRP"_cstr,
             0,
             1,
             {
                 {u8"tnXRP"_cstr, DisplayScale("", "", {{10, 6}}, 0, 6)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tnltc,
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
        {UnitType::Tnxem,
         DisplayDefinition(
             u8"tnNEM"_cstr,
             0,
             1,
             {
                 {u8"TNXEM"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
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
        {UnitType::Tnmaid,
         DisplayDefinition(
             u8"tnMAID"_cstr,
             0,
             1,
             {
                 {u8"TNMAID"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tndoge,
         DisplayDefinition(
             u8"tnDOGE"_cstr,
             0,
             1,
             {
                 {u8"TNDOGE"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tnxmr,
         DisplayDefinition(
             u8"tnXMR"_cstr,
             0,
             1,
             {
                 {u8"TNXMR"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tnwaves,
         DisplayDefinition(
             u8"tnWAVES"_cstr,
             0,
             1,
             {
                 {u8"TNWAVES"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tnnxt,
         DisplayDefinition(
             u8"tnNXT"_cstr,
             0,
             1,
             {
                 {u8"TNNXT"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tnsc,
         DisplayDefinition(
             u8"tnSC"_cstr,
             0,
             1,
             {
                 {u8"TNSC"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tnsteem,
         DisplayDefinition(
             u8"tnSTEEM"_cstr,
             0,
             1,
             {
                 {u8"TNSTEEM"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
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
        {UnitType::Ethereum_olympic,
         DisplayDefinition(
             u8"ETHolympic"_cstr,
             0,
             1,
             {
                 {u8"TNETH"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ethereum_expanse,
         DisplayDefinition(
             u8"EXP"_cstr,
             0,
             1,
             {
                 {u8"EXP"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ethereum_morden,
         DisplayDefinition(
             u8"ETHmorden"_cstr,
             0,
             1,
             {
                 {u8"TNETH"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
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
        {UnitType::Ethereum_rinkeby,
         DisplayDefinition(
             u8"ETHrinkeby"_cstr,
             0,
             1,
             {
                 {u8"TNETH"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ethereum_kovan,
         DisplayDefinition(
             u8"ETHkovan"_cstr,
             0,
             1,
             {
                 {u8"TNETH"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ethereum_sokol,
         DisplayDefinition(
             u8"tnPOA"_cstr,
             0,
             1,
             {
                 {u8"TNPOA"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ethereum_core,
         DisplayDefinition(
             u8"POA"_cstr,
             0,
             1,
             {
                 {u8"POA"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
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
        {UnitType::Bnb,
         DisplayDefinition(
             u8"BNB"_cstr,
             0,
             1,
             {
                 {u8"BNB"_cstr,
                  DisplayScale("", u8"BNB"_cstr, {{10, 8}}, 0, 8)},
                 {u8"jager"_cstr,
                  DisplayScale("", u8"jager"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::Sol,
         DisplayDefinition(
             u8"SOL"_cstr,
             0,
             1,
             {
                 {u8"SOL"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_usdt,
         DisplayDefinition(
             u8"USDT"_cstr,
             0,
             1,
             {
                 {u8"USDT"_cstr, DisplayScale("", "", {{10, 6}}, 0, 6)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ada,
         DisplayDefinition(
             u8"ADA"_cstr,
             0,
             1,
             {
                 {u8"ADA"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Dot,
         DisplayDefinition(
             u8"DOT"_cstr,
             0,
             1,
             {
                 {u8"DOT"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_usdc,
         DisplayDefinition(
             u8"USDC"_cstr,
             0,
             1,
             {
                 {u8"USDC"_cstr, DisplayScale("", "", {{10, 6}}, 0, 6)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_shib,
         DisplayDefinition(
             u8"SHIB"_cstr,
             0,
             1,
             {
                 {u8"SHIB"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Luna,
         DisplayDefinition(
             u8"LUNA"_cstr,
             0,
             1,
             {
                 {u8"LUNA"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Avax,
         DisplayDefinition(
             u8"AVAX"_cstr,
             0,
             1,
             {
                 {u8"AVAX"_cstr, DisplayScale("", "", {{10, 9}}, 0, 9)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_uni,
         DisplayDefinition(
             u8"UNI"_cstr,
             0,
             1,
             {
                 {u8"UNI"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_link,
         DisplayDefinition(
             u8"LINK"_cstr,
             0,
             1,
             {
                 {u8"LINK"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_wbtc,
         DisplayDefinition(
             u8"WBTC"_cstr,
             0,
             1,
             {
                 {u8"WBTC"_cstr, DisplayScale("", "", {{10, 8}}, 0, 8)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_busd,
         DisplayDefinition(
             u8"BUSD"_cstr,
             0,
             1,
             {
                 {u8"BUSD"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::MatiC,
         DisplayDefinition(
             u8"MATIC"_cstr,
             0,
             1,
             {
                 {u8"MATIC"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Algo,
         DisplayDefinition(
             u8"ALGO"_cstr,
             0,
             1,
             {
                 {u8"ALGO"_cstr, DisplayScale("", "", {{10, 6}}, 0, 6)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Vet,
         DisplayDefinition(
             u8"VET"_cstr,
             0,
             1,
             {
                 {u8"VET"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_axs,
         DisplayDefinition(
             u8"AXS"_cstr,
             0,
             1,
             {
                 {u8"AXS"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Icp,
         DisplayDefinition(
             u8"ICP"_cstr,
             0,
             1,
             {
                 {u8"ICP"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_cro,
         DisplayDefinition(
             u8"CRO"_cstr,
             0,
             1,
             {
                 {u8"CRO"_cstr, DisplayScale("", "", {{10, 8}}, 0, 8)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Atom,
         DisplayDefinition(
             u8"ATOM"_cstr,
             0,
             1,
             {
                 {u8"ATOM"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Theta,
         DisplayDefinition(
             u8"THETA"_cstr,
             0,
             1,
             {
                 {u8"THETA"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Fil,
         DisplayDefinition(
             u8"FIL"_cstr,
             0,
             1,
             {
                 {u8"FIL"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Trx,
         DisplayDefinition(
             u8"TRX"_cstr,
             0,
             1,
             {
                 {u8"DASH"_cstr,
                  DisplayScale("", u8"TRX"_cstr, {{10, 6}}, 0, 6)},
                 {u8"sun"_cstr,
                  DisplayScale("", u8"sun"_cstr, {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_ftt,
         DisplayDefinition(
             u8"FTT"_cstr,
             0,
             1,
             {
                 {u8"FTT"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Etc,
         DisplayDefinition(
             u8"ETC"_cstr,
             0,
             1,
             {
                 {u8"ETC"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ftm,
         DisplayDefinition(
             u8"FTM"_cstr,
             0,
             1,
             {
                 {u8"FTM"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_dai,
         DisplayDefinition(
             u8"DAI"_cstr,
             0,
             1,
             {
                 {u8"DAI"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::bep2_bnb_btcb,
         DisplayDefinition(
             u8"BTCB"_cstr,
             0,
             1,
             {
                 {u8"BTCB"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Egld,
         DisplayDefinition(
             u8"EGLD"_cstr,
             0,
             1,
             {
                 {u8"EGLD"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Hbar,
         DisplayDefinition(
             u8"HBAR"_cstr,
             0,
             1,
             {
                 {u8"HBAR"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Xtz,
         DisplayDefinition(
             u8"XTZ"_cstr,
             0,
             1,
             {
                 {u8"XTZ"_cstr, DisplayScale("", "", {{10, 6}}, 0, 6)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_mana,
         DisplayDefinition(
             u8"MANA"_cstr,
             0,
             1,
             {
                 {u8"MANA"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Near,
         DisplayDefinition(
             u8"NEAR"_cstr,
             0,
             1,
             {
                 {u8"NEAR"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_grt,
         DisplayDefinition(
             u8"GRT"_cstr,
             0,
             1,
             {
                 {u8"GRT"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::bsc20_bsc_cake,
         DisplayDefinition(
             u8"CAKE"_cstr,
             0,
             1,
             {
                 {u8"CAKE"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Eos,
         DisplayDefinition(
             u8"EOS"_cstr,
             0,
             1,
             {
                 {u8"EOS"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Flow,
         DisplayDefinition(
             u8"FLOW"_cstr,
             0,
             1,
             {
                 {u8"FLOW"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_aave,
         DisplayDefinition(
             u8"AAVE"_cstr,
             0,
             1,
             {
                 {u8"AAVE"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Klay,
         DisplayDefinition(
             u8"KLAY"_cstr,
             0,
             1,
             {
                 {u8"KLAY"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ksm,
         DisplayDefinition(
             u8"KSM"_cstr,
             0,
             1,
             {
                 {u8"KSM"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
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
        {UnitType::Miota,
         DisplayDefinition(
             u8"MIOTA"_cstr,
             0,
             1,
             {
                 {u8"MIOTA"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Hnt,
         DisplayDefinition(
             u8"HNT"_cstr,
             0,
             1,
             {
                 {u8"HNT"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Rune,
         DisplayDefinition(
             u8"RUNE"_cstr,
             0,
             1,
             {
                 {u8"RUNE"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
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
        {UnitType::erc20_eth_leo,
         DisplayDefinition(
             u8"LEO"_cstr,
             0,
             1,
             {
                 {u8"LEO"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Neo,
         DisplayDefinition(
             u8"NEO"_cstr,
             0,
             1,
             {
                 {u8"NEO"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::One,
         DisplayDefinition(
             u8"ONE"_cstr,
             0,
             1,
             {
                 {u8"ONE"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Qnt,
         DisplayDefinition(
             u8"QNT"_cstr,
             0,
             1,
             {
                 {u8"QNT"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_ust,
         DisplayDefinition(
             u8"UST"_cstr,
             0,
             1,
             {
                 {u8"UST"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_mkr,
         DisplayDefinition(
             u8"MKR"_cstr,
             0,
             1,
             {
                 {u8"MKR"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_enj,
         DisplayDefinition(
             u8"ENJ"_cstr,
             0,
             1,
             {
                 {u8"ENJ"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Chz,
         DisplayDefinition(
             u8"CHZ"_cstr,
             0,
             1,
             {
                 {u8"CHZ"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ar,
         DisplayDefinition(
             u8"AR"_cstr,
             0,
             1,
             {
                 {u8"AR"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Stx,
         DisplayDefinition(
             u8"STX"_cstr,
             0,
             1,
             {
                 {u8"STX"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::trc20_trx_btt,
         DisplayDefinition(
             u8"BTT"_cstr,
             0,
             1,
             {
                 {u8"BTT"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_hot,
         DisplayDefinition(
             u8"HOT"_cstr,
             0,
             1,
             {
                 {u8"HOT"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_sand,
         DisplayDefinition(
             u8"SAND"_cstr,
             0,
             1,
             {
                 {u8"SAND"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_omg,
         DisplayDefinition(
             u8"OMG"_cstr,
             0,
             1,
             {
                 {u8"OMG"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Celo,
         DisplayDefinition(
             u8"CELO"_cstr,
             0,
             1,
             {
                 {u8"CELO"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Zec,
         DisplayDefinition(
             u8"ZEC"_cstr,
             0,
             1,
             {
                 {u8"ZEC"_cstr, DisplayScale("", "", {{10, 8}}, 0, 8)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_comp,
         DisplayDefinition(
             u8"COMP"_cstr,
             0,
             1,
             {
                 {u8"COMP"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Tfuel,
         DisplayDefinition(
             u8"TFUEL"_cstr,
             0,
             1,
             {
                 {u8"TFUEL"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Kda,
         DisplayDefinition(
             u8"KDA"_cstr,
             0,
             1,
             {
                 {u8"KDA"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_lrc,
         DisplayDefinition(
             u8"LRC"_cstr,
             0,
             1,
             {
                 {u8"LRC"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Qtum,
         DisplayDefinition(
             u8"QTUM"_cstr,
             0,
             1,
             {
                 {u8"QTUM"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_crv,
         DisplayDefinition(
             u8"CRV"_cstr,
             0,
             1,
             {
                 {u8"CRV"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Ht,
         DisplayDefinition(
             u8"HT"_cstr,
             0,
             1,
             {
                 {u8"HT"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_nexo,
         DisplayDefinition(
             u8"NEXO"_cstr,
             0,
             1,
             {
                 {u8"NEXO"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_sushi,
         DisplayDefinition(
             u8"SUSHI"_cstr,
             0,
             1,
             {
                 {u8"SUSHI"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_kcs,
         DisplayDefinition(
             u8"KCS"_cstr,
             0,
             1,
             {
                 {u8"KCS"_cstr, DisplayScale("", "", {{10, 6}}, 0, 6)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_bat,
         DisplayDefinition(
             u8"BAT"_cstr,
             0,
             1,
             {
                 {u8"BAT"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Okb,
         DisplayDefinition(
             u8"OKB"_cstr,
             0,
             1,
             {
                 {u8"OKB"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Dcr,
         DisplayDefinition(
             u8"DCR"_cstr,
             0,
             1,
             {
                 {u8"DCR"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Icx,
         DisplayDefinition(
             u8"ICX"_cstr,
             0,
             1,
             {
                 {u8"ICX"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Rvn,
         DisplayDefinition(
             u8"RVN"_cstr,
             0,
             1,
             {
                 {u8"RVN"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Scrt,
         DisplayDefinition(
             u8"SCRT"_cstr,
             0,
             1,
             {
                 {u8"SCRT"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_rev,
         DisplayDefinition(
             u8"REV"_cstr,
             0,
             1,
             {
                 {u8"REV"_cstr, DisplayScale("", "", {{10, 6}}, 0, 6)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_audio,
         DisplayDefinition(
             u8"AUDIO"_cstr,
             0,
             1,
             {
                 {u8"AUDIO"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Zil,
         DisplayDefinition(
             u8"ZIL"_cstr,
             0,
             1,
             {
                 {u8"ZIL"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_tusd,
         DisplayDefinition(
             u8"TUSD"_cstr,
             0,
             1,
             {
                 {u8"TUSD"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_yfi,
         DisplayDefinition(
             u8"YFI"_cstr,
             0,
             1,
             {
                 {u8"YFI"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Mina,
         DisplayDefinition(
             u8"MINA"_cstr,
             0,
             1,
             {
                 {u8"MINA"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_perp,
         DisplayDefinition(
             u8"PERP"_cstr,
             0,
             1,
             {
                 {u8"PERP"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Xdc,
         DisplayDefinition(
             u8"XDC"_cstr,
             0,
             1,
             {
                 {u8"XDC"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_tel,
         DisplayDefinition(
             u8"TEL"_cstr,
             0,
             1,
             {
                 {u8"TEL"_cstr, DisplayScale("", "", {{10, 2}}, 0, 2)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::erc20_eth_snx,
         DisplayDefinition(
             u8"SNX"_cstr,
             0,
             1,
             {
                 {u8"SNX"_cstr, DisplayScale("", "", {{10, 18}}, 0, 18)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
             })},
        {UnitType::Btg,
         DisplayDefinition(
             u8"BTG"_cstr,
             0,
             1,
             {
                 {u8"BTG"_cstr, DisplayScale("", "", {{10, 8}}, 0, 8)},
                 {u8"atomic"_cstr, DisplayScale("", "", {{10, 0}}, 0, 0)},
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
        {UnitType::Bsc,
         DisplayDefinition(
             u8"BSC"_cstr,
             0,
             1,
             {
                 {u8"BSC"_cstr,
                  DisplayScale("", u8"BSC"_cstr, {{10, 8}}, 0, 8)},
                 {u8"jager"_cstr,
                  DisplayScale("", u8"jager"_cstr, {{10, 0}}, 0, 0)},
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
