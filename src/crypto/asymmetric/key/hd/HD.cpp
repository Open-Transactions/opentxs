// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "opentxs/crypto/asymmetric/key/HD.hpp"  // IWYU pragma: associated

#include <utility>

#include "crypto/asymmetric/base/KeyPrivate.hpp"
#include "crypto/asymmetric/key/hd/HDPrivate.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::asymmetric::key
{
HD::HD(KeyPrivate* imp) noexcept
    : EllipticCurve(imp)
{
}

HD::HD(allocator_type alloc) noexcept
    : HD(HDPrivate::Blank(alloc))
{
}

HD::HD(const HD& rhs, allocator_type alloc) noexcept
    : EllipticCurve(rhs, alloc)
{
}

HD::HD(HD&& rhs) noexcept
    : EllipticCurve(std::move(rhs))
{
}

HD::HD(HD&& rhs, allocator_type alloc) noexcept
    : EllipticCurve(std::move(rhs), alloc)
{
}

auto HD::Blank() noexcept -> HD&
{
    static auto blank = HD{allocator_type{alloc::Default()}};

    return blank;
}

auto HD::Chaincode(const PasswordPrompt& reason) const noexcept -> ReadView
{
    return imp_->asHDPrivate()->Chaincode(reason);
}

auto HD::ChildKey(
    const Bip32Index index,
    const PasswordPrompt& reason,
    allocator_type alloc) const noexcept -> HD
{
    return imp_->asHDPrivate()->ChildKey(index, reason, alloc);
}

auto HD::Depth() const noexcept -> int { return imp_->asHDPrivate()->Depth(); }

auto HD::Fingerprint() const noexcept -> Bip32Fingerprint
{
    return imp_->asHDPrivate()->Fingerprint();
}

// NOLINTBEGIN(modernize-use-equals-default)
auto HD::operator=(const HD& rhs) noexcept -> HD&
{
    EllipticCurve::operator=(rhs);

    return *this;
}

auto HD::operator=(HD&& rhs) noexcept -> HD&
{
    EllipticCurve::operator=(std::move(rhs));

    return *this;
}
// NOLINTEND(modernize-use-equals-default)

auto HD::Parent() const noexcept -> Bip32Fingerprint
{
    return imp_->asHDPrivate()->Parent();
}

auto HD::Xprv(const PasswordPrompt& reason, Writer&& out) const noexcept -> bool
{
    return imp_->asHDPrivate()->Xprv(reason, std::move(out));
}

auto HD::Xpub(const PasswordPrompt& reason, Writer&& out) const noexcept -> bool
{
    return imp_->asHDPrivate()->Xpub(reason, std::move(out));
}

HD::~HD() = default;
}  // namespace opentxs::crypto::asymmetric::key
