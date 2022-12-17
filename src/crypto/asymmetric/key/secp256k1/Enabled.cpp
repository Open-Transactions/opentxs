// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/crypto/asymmetric/Factory.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <stdexcept>
#include <utility>

#include "crypto/asymmetric/key/secp256k1/Imp.hpp"
#include "crypto/asymmetric/key/secp256k1/Secp256k1Private.hpp"
#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/crypto/asymmetric/key/Secp256k1.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::Secp256k1
{
    using ReturnType = crypto::asymmetric::key::implementation::Secp256k1;
    using BlankType = crypto::asymmetric::key::Secp256k1Private;
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);

        if (nullptr == out) {
            throw std::runtime_error{"failed to allocate key"};
        }

        pmr.construct(out, api, ecdsa, serializedKey);

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}

auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::Secp256k1
{
    using ReturnType = crypto::asymmetric::key::implementation::Secp256k1;
    using BlankType = crypto::asymmetric::key::Secp256k1Private;
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);

        if (nullptr == out) {
            throw std::runtime_error{"failed to allocate key"};
        }

        {
            std::byte b[512_uz];  // NOLINT(modernize-avoid-c-arrays)
            auto mono = alloc::BoostMonotonic{std::addressof(b), sizeof(b)};
            auto sessionKey =
                api.Crypto().Symmetric().Key(reason, {std::addressof(mono)});
            pmr.construct(out, api, ecdsa, role, version, sessionKey, reason);
        }

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}

auto Secp256k1Key(
    const api::Session& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const Data& publicKey,
    const crypto::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason,
    alloc::Default alloc) noexcept -> crypto::asymmetric::key::Secp256k1
{
    using ReturnType = crypto::asymmetric::key::implementation::Secp256k1;
    using BlankType = crypto::asymmetric::key::Secp256k1Private;
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);

        if (nullptr == out) {
            throw std::runtime_error{"failed to allocate key"};
        }

        {
            std::byte b[512_uz];  // NOLINT(modernize-avoid-c-arrays)
            auto mono = alloc::BoostMonotonic{std::addressof(b), sizeof(b)};
            auto sessionKey =
                api.Crypto().Symmetric().Key(reason, {std::addressof(mono)});
            pmr.construct(
                out,
                api,
                ecdsa,
                privateKey,
                publicKey,
                role,
                version,
                sessionKey,
                reason);
        }

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}
}  // namespace opentxs::factory
