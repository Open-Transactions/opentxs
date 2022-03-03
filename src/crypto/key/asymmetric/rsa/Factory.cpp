// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "internal/crypto/key/Factory.hpp"  // IWYU pragma: associated

#include <exception>

#include "crypto/key/asymmetric/rsa/RSA.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto RSAKey(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& input) noexcept
    -> std::unique_ptr<crypto::key::RSA>
{
    using ReturnType = crypto::key::implementation::RSA;

    try {
        return std::make_unique<ReturnType>(api, engine, input);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto RSAKey(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const crypto::Parameters& options,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::RSA>
{
    using ReturnType = crypto::key::implementation::RSA;

    try {
        auto params = Space{};

        return std::make_unique<ReturnType>(
            api, engine, role, version, options, params, reason);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory
