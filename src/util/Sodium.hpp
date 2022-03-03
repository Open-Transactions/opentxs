// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include <cstddef>

#include "opentxs/util/Bytes.hpp"

namespace opentxs::crypto::sodium
{
using SiphashKey = std::array<unsigned char, 16>;

auto ExpandSeed(
    const ReadView seed,
    const AllocateOutput privateKey,
    const AllocateOutput publicKey) noexcept -> bool;
auto MakeSiphashKey(const ReadView data) noexcept -> SiphashKey;
auto Siphash(const SiphashKey& key, const ReadView data) noexcept
    -> std::size_t;
auto ToCurveKeypair(
    const ReadView edPrivate,
    const ReadView edPublic,
    const AllocateOutput curvePrivate,
    const AllocateOutput curvePublic) noexcept -> bool;
}  // namespace opentxs::crypto::sodium
