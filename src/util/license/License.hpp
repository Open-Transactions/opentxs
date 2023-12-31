// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "opentxs/Context.hpp"

namespace opentxs
{
auto license_argon(LicenseMap& out) noexcept -> void;
auto license_base58(LicenseMap& out) noexcept -> void;
auto license_base64(LicenseMap& out) noexcept -> void;
auto license_bech32(LicenseMap& out) noexcept -> void;
auto license_chaiscript(LicenseMap& out) noexcept -> void;
auto license_dash(LicenseMap& out) noexcept -> void;
auto license_frozen(LicenseMap& out) noexcept -> void;
auto license_irrxml(LicenseMap& out) noexcept -> void;
auto license_keccak_tiny(LicenseMap& out) noexcept -> void;
auto license_libguarded(LicenseMap& out) noexcept -> void;
auto license_lucre(LicenseMap& out) noexcept -> void;
auto license_matterfi(LicenseMap& out) noexcept -> void;
auto license_opentxs(LicenseMap& out) noexcept -> void;
auto license_packetcrypt(LicenseMap& out) noexcept -> void;
auto license_protobuf(LicenseMap& out) noexcept -> void;
auto license_simpleini(LicenseMap& out) noexcept -> void;
auto license_tbb(LicenseMap& out) noexcept -> void;
auto text_apache_v2() noexcept -> std::string_view;
auto text_cc0() noexcept -> std::string_view;
auto text_lgpl_v2_1() noexcept -> std::string_view;
auto text_mpl_v2() noexcept -> std::string_view;
}  // namespace opentxs
