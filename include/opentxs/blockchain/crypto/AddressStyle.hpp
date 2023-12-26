// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/blockchain/crypto/Types.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::crypto
{
enum class AddressStyle : std::underlying_type_t<AddressStyle> {
    unknown_address_style = 0,
    p2pkh = 1,
    p2sh = 2,
    p2wpkh = 3,
    p2wsh = 4,
    p2tr = 5,
    ethereum_account = 6,
};
}  // namespace opentxs::blockchain::crypto
