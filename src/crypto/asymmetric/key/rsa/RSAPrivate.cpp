// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/asymmetric/key/rsa/RSAPrivate.hpp"  // IWYU pragma: associated

namespace opentxs::crypto::asymmetric::internal::key
{
auto RSA::Blank() noexcept -> RSA&
{
    static auto blank = RSA{};

    return blank;
}
}  // namespace opentxs::crypto::asymmetric::internal::key

namespace opentxs::crypto::asymmetric::key
{
RSAPrivate::RSAPrivate(allocator_type alloc) noexcept
    : KeyPrivate(alloc)
{
}

RSAPrivate::RSAPrivate(const RSAPrivate& rhs, allocator_type alloc) noexcept
    : KeyPrivate(rhs, alloc)
{
}

RSAPrivate::~RSAPrivate() = default;
}  // namespace opentxs::crypto::asymmetric::key
