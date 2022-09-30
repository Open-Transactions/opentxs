// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "internal/crypto/key/Factory.hpp"  // IWYU pragma: associated

#include "internal/crypto/key/Null.hpp"

namespace opentxs::factory
{
auto Secp256k1Key(
    const api::Session&,
    const crypto::EcdsaProvider&,
    const proto::AsymmetricKey&) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>
{
    using ReturnType = crypto::key::blank::Secp256k1;

    return std::make_unique<ReturnType>();
}

auto Secp256k1Key(
    const api::Session&,
    const crypto::EcdsaProvider&,
    const crypto::key::asymmetric::Role,
    const VersionNumber,
    const opentxs::PasswordPrompt&) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>
{
    using ReturnType = crypto::key::blank::Secp256k1;

    return std::make_unique<ReturnType>();
}

auto Secp256k1Key(
    const api::Session&,
    const crypto::EcdsaProvider&,
    const opentxs::Secret&,
    const Data&,
    const crypto::key::asymmetric::Role,
    const VersionNumber,
    const opentxs::PasswordPrompt&) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>
{
    using ReturnType = crypto::key::blank::Secp256k1;

    return std::make_unique<ReturnType>();
}

auto Secp256k1Key(
    const api::Session&,
    const crypto::EcdsaProvider&,
    const opentxs::Secret&,
    const opentxs::Secret&,
    const Data&,
    const proto::HDPath&,
    const Bip32Fingerprint,
    const crypto::key::asymmetric::Role,
    const VersionNumber,
    const opentxs::PasswordPrompt&) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>
{
    using ReturnType = crypto::key::blank::Secp256k1;

    return std::make_unique<ReturnType>();
}

auto Secp256k1Key(
    const api::Session&,
    const crypto::EcdsaProvider&,
    const opentxs::Secret&,
    const opentxs::Secret&,
    const Data&,
    const proto::HDPath&,
    const Bip32Fingerprint,
    const crypto::key::asymmetric::Role,
    const VersionNumber) noexcept -> std::unique_ptr<crypto::key::Secp256k1>
{
    using ReturnType = crypto::key::blank::Secp256k1;

    return std::make_unique<ReturnType>();
}

auto Secp256k1Key(const api::Session&, const ReadView, const ReadView) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>
{
    using ReturnType = crypto::key::blank::Secp256k1;

    return std::make_unique<ReturnType>();
}
}  // namespace opentxs::factory
