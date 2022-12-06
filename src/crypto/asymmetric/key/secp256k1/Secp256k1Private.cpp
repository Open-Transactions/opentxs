// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/asymmetric/key/secp256k1/Secp256k1Private.hpp"  // IWYU pragma: associated

#include "crypto/asymmetric/base/KeyPrivate.hpp"
#include "crypto/asymmetric/key/ellipticcurve/EllipticCurvePrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocator.hpp"

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

auto Secp256k1Private::Blank(allocator_type alloc) noexcept -> Secp256k1Private*
{
    auto pmr = alloc::PMR<Secp256k1Private>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out);

    return out;
}

auto Secp256k1Private::clone(allocator_type alloc) const noexcept
    -> Secp256k1Private*
{
    auto pmr = alloc::PMR<Secp256k1Private>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this);

    return out;
}

auto Secp256k1Private::get_deleter() const noexcept
    -> std::function<void(KeyPrivate*)>
{
    return [alloc = alloc::PMR<Secp256k1Private>{get_allocator()}](
               KeyPrivate* in) mutable {
        auto* p = dynamic_cast<Secp256k1Private*>(in);

        OT_ASSERT(nullptr != p);

        alloc.destroy(p);
        alloc.deallocate(p, 1_uz);
    };
}

Secp256k1Private::~Secp256k1Private() = default;
}  // namespace opentxs::crypto::asymmetric::key
