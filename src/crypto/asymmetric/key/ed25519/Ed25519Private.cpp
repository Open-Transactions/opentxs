// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/asymmetric/key/ed25519/Ed25519Private.hpp"  // IWYU pragma: associated

#include "crypto/asymmetric/base/KeyPrivate.hpp"
#include "crypto/asymmetric/key/ellipticcurve/EllipticCurvePrivate.hpp"

namespace opentxs::crypto::asymmetric::internal::key
{
auto Ed25519::Blank() noexcept -> Ed25519&
{
    static auto blank = Ed25519{};

    return blank;
}
}  // namespace opentxs::crypto::asymmetric::internal::key

namespace opentxs::crypto::asymmetric::key
{
Ed25519Private::Ed25519Private(allocator_type alloc) noexcept
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(alloc)
    , HDPrivate(alloc)
{
}

Ed25519Private::Ed25519Private(
    const Ed25519Private& rhs,
    allocator_type alloc) noexcept
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(alloc)
    , HDPrivate(rhs, alloc)
{
}

Ed25519Private::~Ed25519Private() = default;
}  // namespace opentxs::crypto::asymmetric::key
