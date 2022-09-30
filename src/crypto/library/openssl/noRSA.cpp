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

auto OpenSSL::get_params(const AllocateOutput, const Parameters&, ::EVP_PKEY*)
    const noexcept -> bool
{
    return false;
}

auto OpenSSL::import_dh(const ReadView, ::EVP_PKEY*) const noexcept -> bool
{
    return false;
}

auto OpenSSL::make_dh_key(
    const AllocateOutput,
    const AllocateOutput,
    const AllocateOutput,
    const Parameters&) const noexcept -> bool
{
    return false;
}

auto OpenSSL::make_signing_key(
    const AllocateOutput,
    const AllocateOutput,
    const Parameters&) const noexcept -> bool
{
    return false;
}

auto OpenSSL::primes(const int) -> int { return {}; }

auto OpenSSL::RandomKeypair(
    const AllocateOutput,
    const AllocateOutput,
    const crypto::key::asymmetric::Role,
    const Parameters&,
    const AllocateOutput) const noexcept -> bool
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
    const AllocateOutput) const -> bool
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

auto OpenSSL::write_dh(const AllocateOutput, ::EVP_PKEY*) const noexcept -> bool
{
    return false;
}

auto OpenSSL::write_keypair(
    const AllocateOutput,
    const AllocateOutput,
    ::EVP_PKEY*) const noexcept -> bool
{
    return false;
}
}  // namespace opentxs::crypto::implementation
