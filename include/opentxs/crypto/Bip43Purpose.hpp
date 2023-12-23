// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/crypto/Types.hpp"  // IWYU pragma: keep

namespace opentxs::crypto
{
enum class Bip43Purpose : std::underlying_type_t<Bip43Purpose> {
    HDWALLET = 44,     // BIP-44
    PAYCODE = 47,      // BIP-47
    P2SH_P2WPKH = 49,  // BIP-49
    P2WPKH = 84,       // BIP-84
    FS = 0x4f544653,   // OTFS
    NYM = 0x4f544e4d   // OTNM
};
}  // namespace opentxs::crypto
