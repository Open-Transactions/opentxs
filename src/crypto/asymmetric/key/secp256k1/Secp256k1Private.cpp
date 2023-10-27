// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/asymmetric/key/secp256k1/Secp256k1Private.hpp"  // IWYU pragma: associated

#include "crypto/asymmetric/base/KeyPrivate.hpp"
#include "crypto/asymmetric/key/ellipticcurve/EllipticCurvePrivate.hpp"

namespace opentxs::crypto::asymmetric::internal::key
{
auto Secp256k1::Blank() noexcept -> Secp256k1&
{
    static auto blank = Secp256k1{};

    return blank;
}
}  // namespace opentxs::crypto::asymmetric::internal::key

namespace opentxs::crypto::asymmetric::key
{
Secp256k1Private::Secp256k1Private(allocator_type alloc) noexcept
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(alloc)
    , HDPrivate(alloc)
{
}

Secp256k1Private::Secp256k1Private(
    const Secp256k1Private& rhs,
    allocator_type alloc) noexcept
    : KeyPrivate(alloc)
    , EllipticCurvePrivate(alloc)
    , HDPrivate(rhs, alloc)
{
}

auto Secp256k1Private::UncompressedPubkey() const noexcept -> ReadView
{
    return {};
}

Secp256k1Private::~Secp256k1Private() = default;
}  // namespace opentxs::crypto::asymmetric::key
