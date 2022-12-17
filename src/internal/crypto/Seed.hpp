// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/crypto/Types.hpp"

namespace opentxs::crypto::internal
{
class Seed
{
public:
    static auto Translate(const int proto) noexcept -> SeedStyle;

    virtual auto IncrementIndex(const Bip32Index index) noexcept -> bool = 0;

    virtual ~Seed() = default;
};
}  // namespace opentxs::crypto::internal
