// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"  // IWYU pragma: associated

#include <type_traits>
#include <utility>

#include "crypto/asymmetric/base/KeyPrivate.hpp"
#include "crypto/asymmetric/key/ellipticcurve/EllipticCurvePrivate.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::asymmetric::key
{
EllipticCurve::EllipticCurve(KeyPrivate* imp) noexcept
    : Key(imp)
{
}

EllipticCurve::EllipticCurve(allocator_type alloc) noexcept
    : EllipticCurve(EllipticCurvePrivate::Blank(alloc))
{
}

EllipticCurve::EllipticCurve(
    const EllipticCurve& rhs,
    allocator_type alloc) noexcept
    : Key(rhs, alloc)
{
}

EllipticCurve::EllipticCurve(EllipticCurve&& rhs) noexcept
    : Key(std::move(rhs))
{
}

EllipticCurve::EllipticCurve(EllipticCurve&& rhs, allocator_type alloc) noexcept
    : Key(std::move(rhs), alloc)
{
}

auto EllipticCurve::asEd25519() const noexcept -> const key::Ed25519&
{
    return imp_->asEllipticCurvePrivate()->asEd25519Public();
}

auto EllipticCurve::asEd25519() noexcept -> key::Ed25519&
{
    return imp_->asEllipticCurvePrivate()->asEd25519Public();
}

auto EllipticCurve::asHD() const noexcept -> const key::HD&
{
    return imp_->asEllipticCurvePrivate()->asHDPublic();
}

auto EllipticCurve::asHD() noexcept -> key::HD&
{
    return imp_->asEllipticCurvePrivate()->asHDPublic();
}

auto EllipticCurve::asSecp256k1() const noexcept -> const key::Secp256k1&
{
    return imp_->asEllipticCurvePrivate()->asSecp256k1Public();
}

auto EllipticCurve::asSecp256k1() noexcept -> key::Secp256k1&
{
    return imp_->asEllipticCurvePrivate()->asSecp256k1Public();
}

auto EllipticCurve::Blank() noexcept -> EllipticCurve&
{
    static auto blank = EllipticCurve{allocator_type{alloc::Default()}};

    return blank;
}

auto EllipticCurve::DefaultVersion() noexcept -> VersionNumber { return 2; }

auto EllipticCurve::IncrementPrivate(
    const Secret& scalar,
    const PasswordPrompt& reason,
    allocator_type alloc) const noexcept -> EllipticCurve
{
    return imp_->asEllipticCurvePrivate()->IncrementPrivate(
        scalar, reason, alloc);
}

auto EllipticCurve::IncrementPublic(const Secret& scalar, allocator_type alloc)
    const noexcept -> EllipticCurve
{
    return imp_->asEllipticCurvePrivate()->IncrementPublic(scalar, alloc);
}

auto EllipticCurve::MaxVersion() noexcept -> VersionNumber { return 2; }

// NOLINTBEGIN(modernize-use-equals-default)
auto EllipticCurve::operator=(const EllipticCurve& rhs) noexcept
    -> EllipticCurve&
{
    Key::operator=(rhs);

    return *this;
}

auto EllipticCurve::operator=(EllipticCurve&& rhs) noexcept -> EllipticCurve&
{
    Key::operator=(std::move(rhs));

    return *this;
}
// NOLINTEND(modernize-use-equals-default)

auto EllipticCurve::SignDER(
    const ReadView preimage,
    const crypto::HashType hash,
    Writer&& output,
    const PasswordPrompt& reason) const noexcept -> bool
{
    return imp_->asEllipticCurvePrivate()->SignDER(
        preimage, hash, std::move(output), reason);
}

EllipticCurve::~EllipticCurve() = default;
}  // namespace opentxs::crypto::asymmetric::key
