// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/crypto/asymmetric/Factory.hpp"  // IWYU pragma: associated

#include "opentxs/crypto/asymmetric/key/RSA.hpp"

namespace opentxs::factory
{
auto RSAKey(
    const api::Session&,
    const crypto::AsymmetricProvider&,
    const proto::AsymmetricKey&,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::RSA
{
    return {alloc};
}

auto RSAKey(
    const api::Session&,
    const crypto::AsymmetricProvider&,
    const crypto::asymmetric::Role,
    const VersionNumber,
    const crypto::Parameters&,
    const opentxs::PasswordPrompt&,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::RSA
{
    return {alloc};
}
}  // namespace opentxs::factory
