// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "internal/crypto/key/Factory.hpp"  // IWYU pragma: associated

#include "internal/api/Crypto.hpp"
#include "internal/crypto/key/Null.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"

namespace opentxs::factory
{
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

auto Secp256k1Key(
    const api::Session& api,
    const ReadView key,
    const ReadView chaincode) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>
{
    using ReturnType = opentxs::crypto::key::Secp256k1;
    using Type = opentxs::crypto::key::asymmetric::Algorithm;
    auto serialized = ReturnType::Serialized{};
    serialized.set_version(ReturnType::DefaultVersion);
    serialized.set_type(proto::AKEYTYPE_SECP256K1);
    serialized.set_mode(proto::KEYMODE_PUBLIC);
    serialized.set_role(proto::KEYROLE_SIGN);
    serialized.set_key(key.data(), key.size());
    auto output = factory::Secp256k1Key(
        api,
        api.Crypto().Internal().EllipticProvider(Type::Secp256k1),
        serialized);

    if (false == bool(output)) {
        output = std::make_unique<opentxs::crypto::key::blank::Secp256k1>();
    }

    OT_ASSERT(output);

    return output;
}
}  // namespace opentxs::factory
