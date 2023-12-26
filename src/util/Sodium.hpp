// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include <cstddef>

#include "opentxs/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class WriteBuffer;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::sodium
{
using SiphashKey = std::array<unsigned char, 16>;

auto ExpandSeed(
    const ReadView seed,
    Writer&& privateKey,
    Writer&& publicKey) noexcept -> bool;
auto MakeSiphashKey(const ReadView data) noexcept -> SiphashKey;
auto Randomize(WriteBuffer buffer) noexcept -> bool;
auto Siphash(const SiphashKey& key, const ReadView data) noexcept
    -> std::size_t;
auto ToCurveKeypair(
    const ReadView edPrivate,
    const ReadView edPublic,
    Writer&& curvePrivate,
    Writer&& curvePublic) noexcept -> bool;
}  // namespace opentxs::crypto::sodium
