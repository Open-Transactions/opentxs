// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <cstdint>

namespace ot = opentxs;

namespace ottest
{
struct OPENTXS_EXPORT PathElement {
    using Index = std::uint32_t;

    bool hardened_{};
    Index index_{};
};

struct OPENTXS_EXPORT Child {
    using Path = ot::UnallocatedVector<PathElement>;
    using Base58 = ot::UnallocatedCString;

    Path path_{};
    Base58 xpub_{};
    Base58 xprv_{};
};

struct OPENTXS_EXPORT Bip32TestCase {
    using Hex = ot::UnallocatedCString;
    using Children = ot::UnallocatedVector<Child>;

    Hex seed_{};
    Children children_{};
};

OPENTXS_EXPORT auto Bip32TestCases() noexcept
    -> const ot::UnallocatedVector<Bip32TestCase>&;
}  // namespace ottest
