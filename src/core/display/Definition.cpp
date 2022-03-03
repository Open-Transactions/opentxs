// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated

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

namespace opentxs::display
{
using DefinitionMap = robin_hood::unordered_flat_map<UnitType, Definition>;

Definition::Definition(CString&& shortname, Scales&& scales) noexcept
    : imp_(std::make_unique<Imp>(std::move(shortname), std::move(scales))
               .release())
{
    OT_ASSERT(imp_);
}

Definition::Definition() noexcept
    : Definition({}, Scales{})
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
         {u8"BTC",
          {
              {u8"BTC", {"", u8"₿", {{10, 8}}, 0, 8}},
              {u8"mBTC", {"", u8"mBTC", {{10, 5}}, 0, 5}},
              {u8"bits", {"", u8"bits", {{10, 2}}, 0, 2}},
              {u8"μBTC", {"", u8"μBTC", {{10, 2}}, 0, 2}},
              {u8"satoshi", {"", u8"satoshis", {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Tnbtc,
         {u8"tnBTC",
          {
              {u8"BTC", {"", u8"tBTC", {{10, 8}}, 0, 8}},
              {u8"mBTC", {"", u8"mBTC", {{10, 5}}, 0, 5}},
              {u8"bits", {"", u8"bits", {{10, 2}}, 0, 2}},
              {u8"μBTC", {"", u8"μBTC", {{10, 2}}, 0, 2}},
              {u8"satoshi", {"", u8"satoshis", {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Bch,
         {u8"BCH",
          {
              {u8"BCH", {"", u8"BCH", {{10, 8}}, 0, 8}},
              {u8"mBCH", {"", u8"mBCH", {{10, 5}}, 0, 5}},
              {u8"bits", {"", u8"bits", {{10, 2}}, 0, 2}},
              {u8"μBCH", {"", u8"μBCH", {{10, 2}}, 0, 2}},
              {u8"satoshi", {"", u8"satoshis", {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Tnbch,
         {u8"tnBCH",
          {
              {u8"BCH", {"", u8"tBCH", {{10, 8}}, 0, 8}},
              {u8"mBCH", {"", u8"mBCH", {{10, 5}}, 0, 5}},
              {u8"bits", {"", u8"bits", {{10, 2}}, 0, 2}},
              {u8"μBCH", {"", u8"μBCH", {{10, 2}}, 0, 2}},
              {u8"satoshi", {"", u8"satoshis", {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Eth, {}},               // TODO
        {UnitType::Ethereum_ropsten, {}},  // TODO
        {UnitType::Ltc,
         {u8"LTC",
          {
              {u8"LTC", {"", u8"Ł", {{10, 8}}, 0, 6}},
              {u8"mLTC", {"", u8"mŁ", {{10, 5}}, 0, 3}},
              {u8"μLTC", {"", u8"μŁ", {{10, 2}}, 0, 0}},
              {u8"photons", {"", u8"photons", {{10, 2}}, 0, 0}},
          }}},
        {UnitType::Tnltx,
         {u8"tnLTC",
          {
              {u8"LTC", {"", u8"Ł", {{10, 8}}, 0, 6}},
              {u8"mLTC", {"", u8"mŁ", {{10, 5}}, 0, 3}},
              {u8"μLTC", {"", u8"μŁ", {{10, 2}}, 0, 0}},
              {u8"photons", {"", u8"photons", {{10, 2}}, 0, 0}},
          }}},
        {UnitType::Pkt,
         {u8"PKT",
          {
              {u8"PKT", {"", u8"PKT", {{2, 30}}, 0, 11}},
              {u8"mPKT", {"", u8"mPKT", {{2, 30}, {10, -3}}, 0, 8}},
              {u8"μPKT", {"", u8"μPKT", {{2, 30}, {10, -6}}, 0, 5}},
              {u8"nPKT", {"", u8"nPKT", {{2, 30}, {10, -9}}, 0, 2}},
              {u8"pack", {"", u8"pack", {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Tnpkt,
         {u8"tnPKT",
          {
              {u8"PKT", {"", u8"PKT", {{2, 30}}, 0, 11}},
              {u8"mPKT", {"", u8"mPKT", {{2, 30}, {10, -3}}, 0, 8}},
              {u8"μPKT", {"", u8"μPKT", {{2, 30}, {10, -6}}, 0, 5}},
              {u8"nPKT", {"", u8"nPKT", {{2, 30}, {10, -9}}, 0, 2}},
              {u8"pack", {"", u8"pack", {{10, 0}}, 0, 0}},
          }}},
        {UnitType::Regtest,
         {u8"UNITTEST",
          {
              {u8"Unit", {"", u8"units", {{10, 8}}, 0, 8}},
          }}},
        {UnitType::Usd,
         {u8"USD",
          {
              {u8"dollars", {u8"$", u8"", {{10, 0}}, 2, 3}},
              {u8"cents", {u8"", u8"¢", {{10, -2}}, 0, 1}},
              {u8"millions", {u8"$", u8"MM", {{10, 6}}, 0, 9}},
              {u8"mills", {u8"", u8"₥", {{10, -3}}, 0, 0}},
          }}},
    };

    try {
        return map.at(in);
    } catch (...) {
        return defaultDefinition;
    }
}
}  // namespace opentxs::display
