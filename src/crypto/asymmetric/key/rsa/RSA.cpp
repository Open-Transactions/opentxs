// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "opentxs/crypto/asymmetric/key/RSA.hpp"  // IWYU pragma: associated

#include <utility>

#include "crypto/asymmetric/key/rsa/RSAPrivate.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::crypto::asymmetric::key
{
RSA::RSA(KeyPrivate* imp) noexcept
    : Key(imp)
{
}

RSA::RSA(allocator_type alloc) noexcept
    : RSA(RSAPrivate::Blank(alloc))
{
}

RSA::RSA(const RSA& rhs, allocator_type alloc) noexcept
    : Key(rhs, alloc)
{
}

RSA::RSA(RSA&& rhs) noexcept
    : Key(std::move(rhs))
{
}

RSA::RSA(RSA&& rhs, allocator_type alloc) noexcept
    : Key(std::move(rhs), alloc)
{
}

auto RSA::Blank() noexcept -> RSA&
{
    static auto blank = RSA{allocator_type{alloc::Default()}};

    return blank;
}

// NOLINTBEGIN(modernize-use-equals-default)
auto RSA::operator=(const RSA& rhs) noexcept -> RSA&
{
    Key::operator=(rhs);

    return *this;
}

auto RSA::operator=(RSA&& rhs) noexcept -> RSA&
{
    Key::operator=(std::move(rhs));

    return *this;
}
// NOLINTEND(modernize-use-equals-default)

RSA::~RSA() = default;
}  // namespace opentxs::crypto::asymmetric::key
