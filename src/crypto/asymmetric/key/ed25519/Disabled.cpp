// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "internal/crypto/asymmetric/Factory.hpp"  // IWYU pragma: associated

#include "opentxs/crypto/asymmetric/key/Ed25519.hpp"

namespace opentxs::factory
{
auto Ed25519Key(
    const api::Session&,
    const crypto::EcdsaProvider&,
    const proto::AsymmetricKey&,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::Ed25519
{
    return {alloc};
}

auto Ed25519Key(
    const api::Session&,
    const crypto::EcdsaProvider&,
    const crypto::asymmetric::Role,
    const VersionNumber,
    const opentxs::PasswordPrompt&,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::Ed25519
{
    return {alloc};
}
}  // namespace opentxs::factory
