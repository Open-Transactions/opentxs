// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/crypto/asymmetric/Factory.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "crypto/asymmetric/key/ed25519/Ed25519Private.hpp"
#include "crypto/asymmetric/key/ed25519/Imp.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/crypto/asymmetric/key/Ed25519.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto Ed25519Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::Ed25519
{
    using ReturnType = crypto::asymmetric::key::implementation::Ed25519;
    using BlankType = crypto::asymmetric::key::Ed25519Private;

    try {

        return pmr::construct<ReturnType>(alloc, api, ecdsa, serializedKey);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc);
    }
}

auto Ed25519Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::Ed25519
{
    using ReturnType = crypto::asymmetric::key::implementation::Ed25519;
    using BlankType = crypto::asymmetric::key::Ed25519Private;

    try {
        // TODO use alloc::Strategy::work_
        std::byte b[512_uz];  // NOLINT(modernize-avoid-c-arrays)
        auto mono = alloc::MonotonicUnsync{std::addressof(b), sizeof(b)};
        auto sessionKey =
            api.Crypto().Symmetric().Key(reason, {std::addressof(mono)});

        return pmr::construct<ReturnType>(
            alloc, api, ecdsa, role, version, sessionKey, reason);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc);
    }
}
}  // namespace opentxs::factory
