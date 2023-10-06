// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/crypto/asymmetric/Factory.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "crypto/asymmetric/key/rsa/Imp.hpp"
#include "crypto/asymmetric/key/rsa/RSAPrivate.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/crypto/asymmetric/key/RSA.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto RSAKey(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& input,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::RSA
{
    using ReturnType = crypto::asymmetric::key::implementation::RSA;
    using BlankType = crypto::asymmetric::key::RSAPrivate;

    try {

        return pmr::construct<ReturnType>(alloc, api, engine, input);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc);
    }
}

auto RSAKey(
    const api::Session& api,
    const crypto::AsymmetricProvider& engine,
    const crypto::asymmetric::Role input,
    const VersionNumber version,
    const crypto::Parameters& options,
    const opentxs::PasswordPrompt& reason,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::RSA
{
    using ReturnType = crypto::asymmetric::key::implementation::RSA;
    using BlankType = crypto::asymmetric::key::RSAPrivate;

    try {
        // TODO use alloc::Strategy::work_
        std::byte b[512_uz];  // NOLINT(modernize-avoid-c-arrays)
        auto mono = alloc::MonotonicUnsync{std::addressof(b), sizeof(b)};
        auto sessionKey =
            api.Crypto().Symmetric().Key(reason, {std::addressof(mono)});
        auto params = Space{};

        return pmr::construct<ReturnType>(
            alloc,
            api,
            engine,
            input,
            version,
            options,
            params,
            sessionKey,
            reason);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc);
    }
}
}  // namespace opentxs::factory
