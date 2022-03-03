// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "internal/crypto/key/Factory.hpp"  // IWYU pragma: associated

#include <exception>

#include "crypto/key/asymmetric/secp256k1/Secp256k1.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& input) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>
{
    using ReturnType = crypto::key::implementation::Secp256k1;

    try {

        return std::make_unique<ReturnType>(api, ecdsa, input);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to generate key: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Role input,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>
{
    using ReturnType = crypto::key::implementation::Secp256k1;

    try {

        return std::make_unique<ReturnType>(api, ecdsa, input, version, reason);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to generate key: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const Data& publicKey,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>
{
    using ReturnType = crypto::key::implementation::Secp256k1;

    try {
        auto sessionKey = api.Crypto().Symmetric().Key(reason);

        return std::make_unique<ReturnType>(
            api,
            ecdsa,
            privateKey,
            publicKey,
            role,
            version,
            sessionKey,
            reason);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            ": Failed to generate key: ")(e.what())
            .Flush();

        return nullptr;
    }
}
}  // namespace opentxs::factory
