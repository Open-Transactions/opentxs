// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "crypto/library/openssl/OpenSSL.hpp"  // IWYU pragma: associated

namespace opentxs::crypto::implementation
{
auto OpenSSL::generate_dh(const Parameters&, ::EVP_PKEY*) const noexcept -> bool
{
    return false;
}

auto OpenSSL::get_params(Writer&&, const Parameters&, ::EVP_PKEY*)
    const noexcept -> bool
{
    return false;
}

auto OpenSSL::import_dh(const ReadView, ::EVP_PKEY*) const noexcept -> bool
{
    return false;
}

auto OpenSSL::make_dh_key(Writer&&, Writer&&, Writer&&, const Parameters&)
    const noexcept -> bool
{
    return false;
}

auto OpenSSL::make_signing_key(Writer&&, Writer&&, const Parameters&)
    const noexcept -> bool
{
    return false;
}

auto OpenSSL::primes(const int) -> int { return {}; }

auto OpenSSL::RandomKeypair(
    Writer&&,
    Writer&&,
    const crypto::asymmetric::Role,
    const Parameters&,
    Writer&&) const noexcept -> bool
{
    return false;
}

auto OpenSSL::SharedSecret(
    const ReadView,
    const ReadView,
    const SecretStyle,
    Secret&) const noexcept -> bool
{
    return false;
}

auto OpenSSL::Sign(
    const ReadView,
    const ReadView,
    const crypto::HashType,
    Writer&&) const -> bool
{
    return false;
}

auto OpenSSL::Verify(
    const ReadView,
    const ReadView,
    const ReadView,
    const crypto::HashType) const -> bool
{
    return false;
}

auto OpenSSL::write_dh(Writer&&, ::EVP_PKEY*) const noexcept -> bool
{
    return false;
}

auto OpenSSL::write_keypair(Writer&&, Writer&&, ::EVP_PKEY*) const noexcept
    -> bool
{
    return false;
}
}  // namespace opentxs::crypto::implementation
