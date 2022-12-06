// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/crypto/asymmetric/key/Secp256k1.hpp"  // IWYU pragma: associated

#include <utility>

#include "crypto/asymmetric/key/secp256k1/Secp256k1Private.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::crypto::asymmetric::key
{
Secp256k1::Secp256k1(KeyPrivate* imp) noexcept
    : HD(imp)
{
}

Secp256k1::Secp256k1(allocator_type alloc) noexcept
    : Secp256k1(Secp256k1Private::Blank(alloc))
{
}

Secp256k1::Secp256k1(const Secp256k1& rhs, allocator_type alloc) noexcept
    : HD(rhs, alloc)
{
}

Secp256k1::Secp256k1(Secp256k1&& rhs) noexcept
    : HD(std::move(rhs))
{
}

Secp256k1::Secp256k1(Secp256k1&& rhs, allocator_type alloc) noexcept
    : HD(std::move(rhs), alloc)
{
}

auto Secp256k1::Blank() noexcept -> Secp256k1&
{
    static auto blank = Secp256k1{allocator_type{alloc::Default()}};

    return blank;
}

// NOLINTBEGIN(modernize-use-equals-default)
auto Secp256k1::operator=(const Secp256k1& rhs) noexcept -> Secp256k1&
{
    HD::operator=(rhs);

    return *this;
}

auto Secp256k1::operator=(Secp256k1&& rhs) noexcept -> Secp256k1&
{
    HD::operator=(std::move(rhs));

    return *this;
}
// NOLINTEND(modernize-use-equals-default)

Secp256k1::~Secp256k1() = default;
}  // namespace opentxs::crypto::asymmetric::key
