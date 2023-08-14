// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/Json.hpp"  // IWYU pragma: associated

namespace opentxs::blockchain::params
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreserved-identifier"
#include <BCH.json.h>
#include <BSV.json.h>
#include <BTC.json.h>
#include <CSPR.json.h>
#include <DASH.json.h>
#include <ETH.json.h>
#include <ETHgoerli.json.h>
#include <ETHholesovice.json.h>
#include <ETHropsten.json.h>
#include <ETHsepolia.json.h>
#include <LTC.json.h>
#include <PKT.json.h>
#include <UNITTEST.json.h>
#include <XEC.json.h>
#include <tn4BCH.json.h>
#include <tnBCH.json.h>
#include <tnBSV.json.h>
#include <tnBTC.json.h>
#include <tnCSPR.json.h>
#include <tnDASH.json.h>
#include <tnLTC.json.h>
#include <tnPKT.json.h>
#include <tnXEC.json.h>

#pragma GCC diagnostic pop
}  // namespace opentxs::blockchain::params

namespace opentxs::blockchain::params
{
auto bch_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__BCH_json), __BCH_json_len};
}

auto bsv_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__BSV_json), __BSV_json_len};
}

auto btc_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__BTC_json), __BTC_json_len};
}

auto cspr_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__CSPR_json), __CSPR_json_len};
}

auto dash_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__DASH_json), __DASH_json_len};
}

auto eth_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__ETH_json), __ETH_json_len};
}

auto ethgoerli_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__ETHgoerli_json), __ETHgoerli_json_len};
}

auto ethholesovice_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__ETHholesovice_json),
        __ETHholesovice_json_len};
}

auto ethropsten_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__ETHropsten_json),
        __ETHropsten_json_len};
}

auto ethsepolia_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__ETHsepolia_json),
        __ETHsepolia_json_len};
}

auto ltc_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__LTC_json), __LTC_json_len};
}

auto pkt_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__PKT_json), __PKT_json_len};
}

auto tn4bch_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tn4BCH_json), __tn4BCH_json_len};
}

auto tnbch_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnBCH_json), __tnBCH_json_len};
}

auto tnbsv_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnBSV_json), __tnBSV_json_len};
}

auto tnbtc_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnBTC_json), __tnBTC_json_len};
}

auto tncspr_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnCSPR_json), __tnCSPR_json_len};
}

auto tndash_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnDASH_json), __tnDASH_json_len};
}

auto tnltc_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnLTC_json), __tnLTC_json_len};
}

auto tnpkt_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnPKT_json), __tnPKT_json_len};
}

auto tnxec_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__tnXEC_json), __tnXEC_json_len};
}

auto unittest_json() noexcept -> std::string_view
{
    return {
        reinterpret_cast<const char*>(__UNITTEST_json), __UNITTEST_json_len};
}

auto xec_json() noexcept -> std::string_view
{
    return {reinterpret_cast<const char*>(__XEC_json), __XEC_json_len};
}
}  // namespace opentxs::blockchain::params
