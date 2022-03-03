// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "internal/crypto/key/Factory.hpp"  // IWYU pragma: associated

#include <exception>

#include "crypto/key/asymmetric/ed25519/Ed25519.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto Ed25519Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& input) noexcept
    -> std::unique_ptr<crypto::key::Ed25519>
{
    using ReturnType = crypto::key::implementation::Ed25519;

    try {

        return std::make_unique<ReturnType>(api, ecdsa, input);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(
            ": Failed to generate key: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Ed25519Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Role input,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Ed25519>
{
    using ReturnType = crypto::key::implementation::Ed25519;

    try {

        return std::make_unique<ReturnType>(api, ecdsa, input, version, reason);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(
            ": Failed to generate key: ")(e.what())
            .Flush();

        return nullptr;
    }
}
}  // namespace opentxs::factory
