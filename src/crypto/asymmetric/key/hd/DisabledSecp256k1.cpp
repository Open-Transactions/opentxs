// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/crypto/asymmetric/Factory.hpp"  // IWYU pragma: associated

#include "opentxs/crypto/asymmetric/key/Secp256k1.hpp"

namespace opentxs::factory
{
auto Secp256k1Key(
    const api::Session&,
    const crypto::EcdsaProvider&,
    const opentxs::Secret&,
    const opentxs::Secret& chainCode,
    const Data&,
    const proto::HDPath&,
    const Bip32Fingerprint parent,
    const crypto::asymmetric::Role,
    const VersionNumber version,
    const opentxs::PasswordPrompt&,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::Secp256k1
{
    return {alloc};
}

auto Secp256k1Key(
    const api::Session&,
    const crypto::EcdsaProvider&,
    const opentxs::Secret&,
    const opentxs::Secret& chainCode,
    const Data&,
    const proto::HDPath&,
    const Bip32Fingerprint parent,
    const crypto::asymmetric::Role,
    const VersionNumber,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::Secp256k1
{
    return {alloc};
}

auto Secp256k1Key(
    const api::Session&,
    const ReadView,
    const ReadView,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::Secp256k1
{
    return {alloc};
}
}  // namespace opentxs::factory
