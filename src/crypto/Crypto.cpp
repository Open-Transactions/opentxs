// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "opentxs/crypto/Types.hpp"  // IWYU pragma: associated

#include <robin_hood.h>

#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
auto print(crypto::SeedStyle type) noexcept -> UnallocatedCString
{
    using Type = crypto::SeedStyle;
    static const auto map =
        robin_hood::unordered_flat_map<Type, UnallocatedCString>{
            {Type::Error, "invalid"},
            {Type::BIP32, "BIP-32"},
            {Type::BIP39, "BIP-39"},
            {Type::PKT, "pktwallet"},
        };

    try {

        return map.at(type);
    } catch (...) {

        return map.at(Type::Error);
    }
}
}  // namespace opentxs
