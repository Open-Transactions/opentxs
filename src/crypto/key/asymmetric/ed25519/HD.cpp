// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "internal/crypto/key/Factory.hpp"  // IWYU pragma: associated

#include "crypto/key/asymmetric/ed25519/Ed25519.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Secret.hpp"

namespace opentxs::factory
{
auto Ed25519Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Ed25519>
{
    using ReturnType = crypto::key::implementation::Ed25519;
    auto sessionKey = api.Crypto().Symmetric().Key(reason);

    return std::make_unique<ReturnType>(
        api,
        ecdsa,
        privateKey,
        chainCode,
        publicKey,
        path,
        parent,
        role,
        version,
        sessionKey,
        reason);
}
}  // namespace opentxs::factory
