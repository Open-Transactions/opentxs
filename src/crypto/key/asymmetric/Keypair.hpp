// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/crypto/key/Keypair.hpp"
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identity/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace proto
{
class AsymmetricKey;
}  // namespace proto

class Data;
class PasswordPrompt;
class Secret;
class Signature;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto::key::implementation
{
class Keypair final : virtual public key::Keypair
{
public:
    operator bool() const noexcept final { return true; }

    auto CheckCapability(
        const identity::NymCapability& capability) const noexcept -> bool final;
    auto GetPrivateKey() const noexcept(false) -> const asymmetric::Key& final;
    auto GetPublicKey() const noexcept(false) -> const asymmetric::Key& final;
    auto GetPublicKeyBySignature(
        Keys& listOutput,
        const Signature& theSignature,
        bool bInclusive = false) const noexcept -> std::int32_t final;
    auto Serialize(proto::AsymmetricKey& serialized, bool privateKey = false)
        const noexcept -> bool final;
    auto GetTransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const noexcept -> bool final;

    Keypair(
        const api::Session& api,
        const opentxs::crypto::asymmetric::Role role,
        crypto::asymmetric::Key publicKey,
        crypto::asymmetric::Key privateKey) noexcept;
    Keypair() = delete;
    Keypair(const Keypair&) noexcept;
    Keypair(Keypair&&) = delete;
    auto operator=(const Keypair&) -> Keypair& = delete;
    auto operator=(Keypair&&) -> Keypair& = delete;

    ~Keypair() final = default;

private:
    friend key::Keypair;

    const api::Session& api_;
    asymmetric::Key key_private_;
    asymmetric::Key key_public_;
    const opentxs::crypto::asymmetric::Role role_;

    auto clone() const -> Keypair* final { return new Keypair(*this); }
};
}  // namespace opentxs::crypto::key::implementation
