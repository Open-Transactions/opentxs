// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

namespace opentxs::blockchain::params
{
auto bch_json() noexcept -> std::string_view;
auto bsv_json() noexcept -> std::string_view;
auto btc_json() noexcept -> std::string_view;
auto cspr_json() noexcept -> std::string_view;
auto dash_json() noexcept -> std::string_view;
auto eth_json() noexcept -> std::string_view;
auto ethropsten_json() noexcept -> std::string_view;
auto ltc_json() noexcept -> std::string_view;
auto pkt_json() noexcept -> std::string_view;
auto tn4bch_json() noexcept -> std::string_view;
auto tnbch_json() noexcept -> std::string_view;
auto tnbsv_json() noexcept -> std::string_view;
auto tnbtc_json() noexcept -> std::string_view;
auto tncspr_json() noexcept -> std::string_view;
auto tndash_json() noexcept -> std::string_view;
auto tnltc_json() noexcept -> std::string_view;
auto tnpkt_json() noexcept -> std::string_view;
auto tnxec_json() noexcept -> std::string_view;
auto unittest_json() noexcept -> std::string_view;
auto xec_json() noexcept -> std::string_view;
}  // namespace opentxs::blockchain::params
