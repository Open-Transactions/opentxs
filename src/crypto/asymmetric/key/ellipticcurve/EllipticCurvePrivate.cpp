// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::EcdsaProvider

#include "crypto/asymmetric/key/ellipticcurve/EllipticCurvePrivate.hpp"  // IWYU pragma: associated

#include "internal/crypto/asymmetric/key/Ed25519.hpp"
#include "internal/crypto/asymmetric/key/HD.hpp"
#include "internal/crypto/asymmetric/key/Secp256k1.hpp"
#include "internal/crypto/library/EcdsaProvider.hpp"
#include "internal/crypto/library/Null.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/crypto/asymmetric/key/Ed25519.hpp"
#include "opentxs/crypto/asymmetric/key/HD.hpp"
#include "opentxs/crypto/asymmetric/key/Secp256k1.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::asymmetric::internal::key
{
auto EllipticCurve::asEd25519() const noexcept -> const internal::key::Ed25519&
{
    return internal::key::Ed25519::Blank();
}

auto EllipticCurve::asEd25519() noexcept -> internal::key::Ed25519&
{
    return internal::key::Ed25519::Blank();
}

auto EllipticCurve::asHD() const noexcept -> const internal::key::HD&
{
    return internal::key::HD::Blank();
}

auto EllipticCurve::asHD() noexcept -> internal::key::HD&
{
    return internal::key::HD::Blank();
}

auto EllipticCurve::asSecp256k1() const noexcept
    -> const internal::key::Secp256k1&
{
    return internal::key::Secp256k1::Blank();
}

auto EllipticCurve::asSecp256k1() noexcept -> internal::key::Secp256k1&
{
    return internal::key::Secp256k1::Blank();
}

auto EllipticCurve::Blank() noexcept -> EllipticCurve&
{
    static auto blank = EllipticCurve{};

    return blank;
}

auto EllipticCurve::ECDSA() const noexcept -> const crypto::EcdsaProvider&
{
    static const auto provider = crypto::blank::EcdsaProvider{};

    return provider;
}
}  // namespace opentxs::crypto::asymmetric::internal::key

namespace opentxs::crypto::asymmetric::key
{
EllipticCurvePrivate::EllipticCurvePrivate(allocator_type alloc) noexcept
    : KeyPrivate(alloc)
{
}

EllipticCurvePrivate::EllipticCurvePrivate(
    const EllipticCurvePrivate& rhs,
    allocator_type alloc) noexcept
    : KeyPrivate(rhs, alloc)
{
}

auto EllipticCurvePrivate::asEd25519Public() const noexcept
    -> const asymmetric::key::Ed25519&
{
    return asymmetric::key::Ed25519::Blank();
}

auto EllipticCurvePrivate::asEd25519Public() noexcept
    -> asymmetric::key::Ed25519&
{
    return asymmetric::key::Ed25519::Blank();
}

auto EllipticCurvePrivate::asHDPublic() const noexcept
    -> const asymmetric::key::HD&
{
    return asymmetric::key::HD::Blank();
}

auto EllipticCurvePrivate::asHDPublic() noexcept -> asymmetric::key::HD&
{
    return asymmetric::key::HD::Blank();
}

auto EllipticCurvePrivate::asSecp256k1Public() const noexcept
    -> const asymmetric::key::Secp256k1&
{
    return asymmetric::key::Secp256k1::Blank();
}

auto EllipticCurvePrivate::asSecp256k1Public() noexcept
    -> asymmetric::key::Secp256k1&
{
    return asymmetric::key::Secp256k1::Blank();
}

auto EllipticCurvePrivate::Blank(allocator_type alloc) noexcept
    -> EllipticCurvePrivate*
{
    auto pmr = alloc::PMR<EllipticCurvePrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out);

    return out;
}

auto EllipticCurvePrivate::clone(allocator_type alloc) const noexcept
    -> EllipticCurvePrivate*
{
    auto pmr = alloc::PMR<EllipticCurvePrivate>{alloc};
    auto* out = pmr.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    pmr.construct(out, *this);

    return out;
}

auto EllipticCurvePrivate::get_deleter() const noexcept
    -> std::function<void(KeyPrivate*)>
{
    return [alloc = alloc::PMR<EllipticCurvePrivate>{get_allocator()}](
               KeyPrivate* in) mutable {
        auto* p = dynamic_cast<EllipticCurvePrivate*>(in);

        OT_ASSERT(nullptr != p);

        alloc.destroy(p);
        alloc.deallocate(p, 1_uz);
    };
}

auto EllipticCurvePrivate::IncrementPrivate(
    const Secret&,
    const PasswordPrompt&,
    allocator_type alloc) const noexcept -> asymmetric::key::EllipticCurve
{
    return {alloc};
}

auto EllipticCurvePrivate::IncrementPublic(const Secret&, allocator_type alloc)
    const noexcept -> asymmetric::key::EllipticCurve
{
    return {alloc};
}

auto EllipticCurvePrivate::SignDER(
    const ReadView,
    const crypto::HashType,
    Writer&&,
    const PasswordPrompt&) const noexcept -> bool
{
    return {};
}

EllipticCurvePrivate::~EllipticCurvePrivate() = default;
}  // namespace opentxs::crypto::asymmetric::key
