// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identity::CredentialRole
// IWYU pragma: no_forward_declare opentxs::identity::CredentialType
// IWYU pragma: no_forward_declare opentxs::identity::NymCapability

#pragma once

#include <Credential.pb.h>
#include <memory>

#include "core/contract/Signable.hpp"
#include "internal/identity/Types.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "internal/identity/credential/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/asymmetric/Mode.hpp"
#include "opentxs/crypto/asymmetric/Role.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identity/CredentialRole.hpp"
#include "opentxs/identity/CredentialType.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
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

namespace proto
{
class ContactData;
class Signature;
class VerificationSet;
}  // namespace proto

class ByteArray;
class Data;
class PasswordPrompt;
class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential::implementation
{
class Base : virtual public credential::internal::Base,
             public opentxs::contract::implementation::Signable
{
public:
    using SerializedType = proto::Credential;

    auto asString(const bool asPrivate = false) const
        -> UnallocatedCString final;
    auto CredentialID() const -> const identifier::Generic& final
    {
        return id_;
    }
    auto GetContactData(proto::ContactData& output) const -> bool override
    {
        return false;
    }
    auto GetVerificationSet(proto::VerificationSet& output) const
        -> bool override
    {
        return false;
    }
    auto hasCapability(const NymCapability& capability) const -> bool override
    {
        return false;
    }
    auto MasterSignature() const -> Signature final;
    auto Mode() const -> crypto::asymmetric::Mode final { return mode_; }
    auto Role() const -> identity::CredentialRole final { return role_; }
    auto Private() const -> bool final
    {
        return (crypto::asymmetric::Mode::Private == mode_);
    }
    auto Save() const -> bool final;
    auto SelfSignature(CredentialModeFlag version = PUBLIC_VERSION) const
        -> Signature final;
    using Signable::Serialize;
    auto Serialize() const noexcept -> ByteArray final;
    auto Serialize(
        SerializedType& serialized,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const -> bool final;
    auto SourceSignature() const -> Signature final;
    auto TransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const -> bool override;
    auto Type() const -> identity::CredentialType final { return type_; }
    auto Validate() const noexcept -> bool final;
    auto Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const opentxs::crypto::asymmetric::Role key =
            opentxs::crypto::asymmetric::Role::Sign) const -> bool override
    {
        return false;
    }
    auto Verify(
        const proto::Credential& credential,
        const identity::CredentialRole& role,
        const identifier::Generic& masterID,
        const proto::Signature& masterSig) const -> bool override;

    void ReleaseSignatures(const bool onlyPrivate) final;

    Base() = delete;
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    auto operator=(const Base&) -> Base& = delete;
    auto operator=(Base&&) -> Base& = delete;

    ~Base() override = default;

protected:
    const identity::internal::Authority& parent_;
    const identity::Source& source_;
    const UnallocatedCString nym_id_;
    const UnallocatedCString master_id_;
    const identity::CredentialType type_;
    const identity::CredentialRole role_;
    const crypto::asymmetric::Mode mode_;

    static auto get_master_id(
        const api::Session& api,
        const internal::Primary& master) noexcept -> UnallocatedCString;
    static auto get_master_id(
        const api::Session& api,
        const proto::Credential& serialized,
        const internal::Primary& master) noexcept(false) -> UnallocatedCString;

    virtual auto serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const
        -> std::shared_ptr<SerializedType>;
    auto validate(const Lock& lock) const -> bool final;
    virtual auto verify_internally(const Lock& lock) const -> bool;

    void init(
        const identity::credential::internal::Primary& master,
        const PasswordPrompt& reason) noexcept(false);
    virtual void sign(
        const identity::credential::internal::Primary& master,
        const PasswordPrompt& reason) noexcept(false);

    Base(
        const api::Session& api,
        const identity::internal::Authority& owner,
        const identity::Source& source,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const identity::CredentialRole role,
        const crypto::asymmetric::Mode mode,
        const UnallocatedCString& masterID) noexcept;
    Base(
        const api::Session& api,
        const identity::internal::Authority& owner,
        const identity::Source& source,
        const proto::Credential& serialized,
        const UnallocatedCString& masterID) noexcept(false);

private:
    static auto extract_signatures(const SerializedType& serialized)
        -> Signatures;

    auto clone() const noexcept -> Base* final { return nullptr; }
    auto GetID(const Lock& lock) const -> identifier::Generic final;
    // Syntax (non cryptographic) validation
    auto isValid(const Lock& lock) const -> bool;
    // Returns the serialized form to prevent unnecessary serializations
    auto isValid(const Lock& lock, std::shared_ptr<SerializedType>& credential)
        const -> bool;
    auto Name() const noexcept -> UnallocatedCString final
    {
        return id_.asBase58(api_.Crypto());
    }
    auto verify_master_signature(const Lock& lock) const -> bool;

    void add_master_signature(
        const Lock& lock,
        const identity::credential::internal::Primary& master,
        const PasswordPrompt& reason) noexcept(false);
};
}  // namespace opentxs::identity::credential::implementation
