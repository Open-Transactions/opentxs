// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::KeyRole

#pragma once

#include <opentxs/protobuf/Enums.pb.h>
#include <cstdint>
#include <memory>
#include <optional>

#include "identity/credential/Base.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "internal/identity/credential/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Types.internal.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/Types.internal.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/Types.internal.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace crypto
{
class Parameters;
}  // namespace crypto

namespace identity
{
namespace internal
{
class Authority;
}  // namespace internal

class Source;
}  // namespace identity

namespace protobuf
{
class Credential;
class KeyCredential;
class Signature;
}  // namespace protobuf

class Data;
class PasswordPrompt;
class Signature;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential::implementation
{
class Key : virtual public credential::internal::Key,
            public credential::implementation::Base
{
public:
    auto asKey() const noexcept -> const internal::Key& final { return *this; }
    auto GetKeypair(const opentxs::crypto::asymmetric::Role role) const
        -> const crypto::key::Keypair& final;
    auto GetKeypair(
        const crypto::asymmetric::Algorithm type,
        const opentxs::crypto::asymmetric::Role role) const
        -> const crypto::key::Keypair& final;
    auto GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const opentxs::Signature& theSignature,
        char cKeyType = '0') const -> std::int32_t final;
    auto hasCapability(const NymCapability& capability) const -> bool override;
    using Base::Verify;
    auto Verify(
        const Data& plaintext,
        const protobuf::Signature& sig,
        const opentxs::crypto::asymmetric::Role key) const -> bool final;
    auto Sign(
        const crypto::GetPreimage input,
        const crypto::SignatureRole role,
        protobuf::Signature& signature,
        const PasswordPrompt& reason,
        opentxs::crypto::asymmetric::Role key,
        const crypto::HashType hash) const -> bool final;
    auto TransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const -> bool final;

    auto asKey() noexcept -> internal::Key& final { return *this; }

    Key() = delete;
    Key(const Key&) = delete;
    Key(Key&&) = delete;
    auto operator=(const Key&) -> Key& = delete;
    auto operator=(Key&&) -> Key& = delete;

    ~Key() override;

protected:
    const VersionNumber subversion_;
    const OTKeypair signing_key_;
    const OTKeypair authentication_key_;
    const OTKeypair encryption_key_;

    auto id_form() const -> std::shared_ptr<SerializedType> override;
    auto serialize(
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const
        -> std::shared_ptr<Base::SerializedType> override;
    auto verify_internally() const -> bool override;

    auto sign(
        const identity::credential::internal::Primary& master,
        const PasswordPrompt& reason,
        Signatures& out) noexcept(false) -> void override;

    Key(const api::Session& api,
        const identity::internal::Authority& owner,
        const identity::Source& source,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const identity::CredentialRole role,
        const PasswordPrompt& reason,
        const identifier_type& masterID,
        const bool useProvidedSigningKey = false) noexcept(false);
    Key(const api::Session& api,
        const identity::internal::Authority& owner,
        const identity::Source& source,
        const protobuf::Credential& serializedCred,
        const identifier_type& masterID) noexcept(false);

private:
    static const VersionConversionMap credential_subversion_;
    static const VersionConversionMap subversion_to_key_version_;

    static auto deserialize_key(
        const api::Session& api,
        const int index,
        const protobuf::Credential& credential) -> OTKeypair;
    static auto new_key(
        const api::Session& api,
        const protobuf::KeyRole role,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const PasswordPrompt& reason,
        const ReadView dh = {}) noexcept(false) -> OTKeypair;
    static auto signing_key(
        const api::Session& api,
        const crypto::Parameters& params,
        const VersionNumber subversion,
        const bool useProvided,
        const PasswordPrompt& reason) noexcept(false) -> OTKeypair;

    auto addKeytoSerializedKeyCredential(
        protobuf::KeyCredential& credential,
        const bool getPrivate,
        const protobuf::KeyRole role) const -> bool;
    auto addKeyCredentialtoSerializedCredential(
        std::shared_ptr<Base::SerializedType> credential,
        const bool addPrivate) const -> bool;
    auto SelfSign(
        const PasswordPrompt& reason,
        Signatures& out,
        const std::optional<Secret> exportPassword = {},
        const bool onlyPrivate = false) const noexcept(false) -> bool;
    auto VerifySig(
        const protobuf::Signature& sig,
        const CredentialModeFlag asPrivate = PRIVATE_VERSION) const -> bool;
    auto VerifySignedBySelf() const -> bool;
};
}  // namespace opentxs::identity::credential::implementation
