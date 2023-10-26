// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/license/License.hpp"  // IWYU pragma: associated

#include "opentxs/util/Container.hpp"

namespace opentxs
{
auto license_keccak_tiny(LicenseMap& out) noexcept -> void
{
    using namespace std::literals;
    static constexpr auto header =
        "A single-file implementation of SHA-3 and SHAKE.\n\nImplementor: David Leon Gil\nLicense: CC0, attribution kindly requested. Blame taken too, but not liability.\n"sv;

    out.emplace(
        "libkeccak-tiny",
        UnallocatedCString{header} + UnallocatedCString{text_cc0()});
}
}  // namespace opentxs
