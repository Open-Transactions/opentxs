// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <cstddef>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>

#include "core/display/Definition.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Literals.hpp"

namespace opentxs::display
{
using DefinitionMap = robin_hood::unordered_flat_map<UnitType, Definition>;

Definition::Definition(std::string_view shortname, Scales&& scales) noexcept
    : imp_(std::make_unique<Imp>(CString{shortname}, std::move(scales))
               .release())
{
    OT_ASSERT(imp_);
}

Definition::Definition() noexcept
    : Definition({}, {})
{
}

Definition::Definition(const Definition& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_).release())
{
    OT_ASSERT(imp_);
}

Definition::Definition(Definition&& rhs) noexcept
    : Definition()
{
    swap(rhs);
}

auto Definition::operator=(const Definition& rhs) noexcept -> Definition&
{
    if (&rhs != this) { imp_ = std::make_unique<Imp>(*rhs.imp_).release(); }

    return *this;
}

auto Definition::operator=(Definition&& rhs) noexcept -> Definition&
{
    if (&rhs != this) { std::swap(imp_, rhs.imp_); }

    return *this;
}

auto Definition::DisplayScales() const noexcept -> const Scales&
{
    return imp_->scales_;
}

auto Definition::Format(
    const Amount amount,
    const Index index,
    const OptionalInt minDecimals,
    const OptionalInt maxDecimals) const noexcept(false) -> UnallocatedCString
{
    try {
        const auto& scale = imp_->scales_.at(static_cast<std::size_t>(index));

        return scale.second.Format(amount, minDecimals, maxDecimals);
    } catch (...) {
        throw std::out_of_range("Invalid scale index");
    }
}

auto Definition::GetScales() const noexcept -> const Map&
{
    imp_->Populate();

    return imp_->cached_.value();
}

auto Definition::Import(const std::string_view formatted, const Index scale)
    const noexcept(false) -> Amount
{
    return imp_->Import(formatted, scale);
}

auto Definition::ShortName() const noexcept -> std::string_view
{
    return imp_->short_name_;
}

auto Definition::swap(Definition& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
}

Definition::~Definition()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}

auto GetDefinition(UnitType in) noexcept -> const Definition&
{
    static const auto defaultDefinition = Definition{};
    static const auto map = DefinitionMap{
        {UnitType::Btc,
         {u8"BTC"_cstr,
          {
              {u8"BTC"_cstr, {"", u8"₿"_cstr, {{10, 8}}, 0, 8}},
              {u8"mBTC"_cstr, {"", u8"mBTC"_cstr, {{10, 5}}, 0, 5}},
              {u8"bits"_cstr, {"", u8"bits"_cstr, {{10, 2}}, 0, 2}},
              {u8"μBTC"_cstr, {"", u8"μBTC"_cstr, {{10, 2}}, 0, 2}},
              {u8"satoshi"_cstr, {"", u8"satoshis"_cstr, {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Tnbtc,
         {u8"tnBTC"_cstr,
          {
              {u8"BTC"_cstr, {"", u8"tBTC"_cstr, {{10, 8}}, 0, 8}},
              {u8"mBTC"_cstr, {"", u8"mBTC"_cstr, {{10, 5}}, 0, 5}},
              {u8"bits"_cstr, {"", u8"bits"_cstr, {{10, 2}}, 0, 2}},
              {u8"μBTC"_cstr, {"", u8"μBTC"_cstr, {{10, 2}}, 0, 2}},
              {u8"satoshi"_cstr, {"", u8"satoshis"_cstr, {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Bch,
         {u8"BCH"_cstr,
          {
              {u8"BCH"_cstr, {"", u8"BCH"_cstr, {{10, 8}}, 0, 8}},
              {u8"mBCH"_cstr, {"", u8"mBCH"_cstr, {{10, 5}}, 0, 5}},
              {u8"bits"_cstr, {"", u8"bits"_cstr, {{10, 2}}, 0, 2}},
              {u8"μBCH"_cstr, {"", u8"μBCH"_cstr, {{10, 2}}, 0, 2}},
              {u8"satoshi"_cstr, {"", u8"satoshis"_cstr, {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Tnbch,
         {u8"tnBCH"_cstr,
          {
              {u8"BCH"_cstr, {"", u8"tBCH"_cstr, {{10, 8}}, 0, 8}},
              {u8"mBCH"_cstr, {"", u8"mBCH"_cstr, {{10, 5}}, 0, 5}},
              {u8"bits"_cstr, {"", u8"bits"_cstr, {{10, 2}}, 0, 2}},
              {u8"μBCH"_cstr, {"", u8"μBCH"_cstr, {{10, 2}}, 0, 2}},
              {u8"satoshi"_cstr, {"", u8"satoshis"_cstr, {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Eth, {}},               // TODO
        {UnitType::Ethereum_ropsten, {}},  // TODO
        {UnitType::Ltc,
         {u8"LTC"_cstr,
          {
              {u8"LTC"_cstr, {"", u8"Ł"_cstr, {{10, 8}}, 0, 6}},
              {u8"mLTC"_cstr, {"", u8"mŁ"_cstr, {{10, 5}}, 0, 3}},
              {u8"μLTC"_cstr, {"", u8"μŁ"_cstr, {{10, 2}}, 0, 0}},
              {u8"photons"_cstr, {"", u8"photons"_cstr, {{10, 2}}, 0, 0}},
          }}},
        {UnitType::Tnltx,
         {u8"tnLTC"_cstr,
          {
              {u8"LTC"_cstr, {"", u8"Ł"_cstr, {{10, 8}}, 0, 6}},
              {u8"mLTC"_cstr, {"", u8"mŁ"_cstr, {{10, 5}}, 0, 3}},
              {u8"μLTC"_cstr, {"", u8"μŁ"_cstr, {{10, 2}}, 0, 0}},
              {u8"photons"_cstr, {"", u8"photons"_cstr, {{10, 2}}, 0, 0}},
          }}},
        {UnitType::Pkt,
         {u8"PKT"_cstr,
          {
              {u8"PKT"_cstr, {"", u8"PKT"_cstr, {{2, 30}}, 0, 11}},
              {u8"mPKT"_cstr, {"", u8"mPKT"_cstr, {{2, 30}, {10, -3}}, 0, 8}},
              {u8"μPKT"_cstr, {"", u8"μPKT"_cstr, {{2, 30}, {10, -6}}, 0, 5}},
              {u8"nPKT"_cstr, {"", u8"nPKT"_cstr, {{2, 30}, {10, -9}}, 0, 2}},
              {u8"pack"_cstr, {"", u8"pack"_cstr, {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Tnpkt,
         {u8"tnPKT"_cstr,
          {
              {u8"PKT"_cstr, {"", u8"PKT"_cstr, {{2, 30}}, 0, 11}},
              {u8"mPKT"_cstr, {"", u8"mPKT"_cstr, {{2, 30}, {10, -3}}, 0, 8}},
              {u8"μPKT"_cstr, {"", u8"μPKT"_cstr, {{2, 30}, {10, -6}}, 0, 5}},
              {u8"nPKT"_cstr, {"", u8"nPKT"_cstr, {{2, 30}, {10, -9}}, 0, 2}},
              {u8"pack"_cstr, {"", u8"pack"_cstr, {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Bsv,
         {u8"BSV"_cstr,
          {
              {u8"BSV"_cstr, {"", u8"BSV"_cstr, {{10, 8}}, 0, 8}},
              {u8"mBSV"_cstr, {"", u8"mBSV"_cstr, {{10, 5}}, 0, 5}},
              {u8"bits"_cstr, {"", u8"bits"_cstr, {{10, 2}}, 0, 2}},
              {u8"μBSV"_cstr, {"", u8"μBSV"_cstr, {{10, 2}}, 0, 2}},
              {u8"satoshi"_cstr, {"", u8"satoshis"_cstr, {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Tnbsv,
         {u8"tnBSV"_cstr,
          {
              {u8"BSV"_cstr, {"", u8"tBSV"_cstr, {{10, 8}}, 0, 8}},
              {u8"mBSV"_cstr, {"", u8"mBSV"_cstr, {{10, 5}}, 0, 5}},
              {u8"bits"_cstr, {"", u8"bits"_cstr, {{10, 2}}, 0, 2}},
              {u8"μBSV"_cstr, {"", u8"μBSV"_cstr, {{10, 2}}, 0, 2}},
              {u8"satoshi"_cstr, {"", u8"satoshis"_cstr, {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Xec,
         {u8"XEC"_cstr,
          {
              {u8"MXEC"_cstr, {"", u8"MXEC"_cstr, {{10, 8}}, 0, 8}},
              {u8"kXEC"_cstr, {"", u8"kXEC"_cstr, {{10, 5}}, 0, 5}},
              {u8"XEC"_cstr, {"", u8"XEC"_cstr, {{10, 2}}, 0, 2}},
              {u8"mXEC"_cstr, {"", u8"mXEC"_cstr, {{10, 0}}, 0, 0}},
          }}},
        {UnitType::TnXec,
         {u8"tnXEC"_cstr,
          {
              {u8"MtXEC"_cstr, {"", u8"MtXEC"_cstr, {{10, 8}}, 0, 8}},
              {u8"ktXEC"_cstr, {"", u8"ktXEC"_cstr, {{10, 5}}, 0, 5}},
              {u8"tXEC"_cstr, {"", u8"tXEC"_cstr, {{10, 2}}, 0, 2}},
              {u8"mtXEC"_cstr, {"", u8"mtXEC"_cstr, {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Regtest,
         {u8"UNITTEST"_cstr,
          {
              {u8"Unit"_cstr, {"", u8"units"_cstr, {{10, 8}}, 0, 8}},
          }}},
        {UnitType::Usd,
         {u8"USD"_cstr,
          {
              {u8"dollars"_cstr, {u8"$"_cstr, u8""_cstr, {{10, 0}}, 2, 3}},
              {u8"cents"_cstr, {u8""_cstr, u8"¢"_cstr, {{10, -2}}, 0, 1}},
              {u8"millions"_cstr, {u8"$"_cstr, u8"MM"_cstr, {{10, 6}}, 0, 9}},
              {u8"mills"_cstr, {u8""_cstr, u8"₥"_cstr, {{10, -3}}, 0, 0}},
          }}},
    };

    try {
        return map.at(in);
    } catch (...) {
        return defaultDefinition;
    }
}
}  // namespace opentxs::display
