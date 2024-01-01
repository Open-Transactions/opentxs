// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/Credential.pb.h>
#include <memory>

#include "core/contract/Signable.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "internal/identity/credential/Types.hpp"
#include "opentxs/contract/Types.internal.hpp"
#include "opentxs/crypto/asymmetric/Mode.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/Types.internal.hpp"
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

namespace protobuf
{
class ContactData;
class Signature;
class VerificationSet;
}  // namespace protobuf

class Data;
class PasswordPrompt;
class Secret;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential::implementation
{
class Base
    : virtual public credential::internal::Base,
      public opentxs::contract::implementation::Signable<identifier::Generic>
{
public:
    using SerializedType = protobuf::Credential;

    auto asString(const bool asPrivate = false) const
        -> UnallocatedCString final;
    auto CredentialID() const -> const identifier_type& final { return ID(); }
    auto GetContactData(protobuf::ContactData& output) const -> bool override
    {
        return false;
    }
    auto GetVerificationSet(protobuf::VerificationSet& output) const
        -> bool override
    {
        return false;
    }
    auto hasCapability(const NymCapability& capability) const -> bool override
    {
        return false;
    }
    auto MasterSignature() const -> contract::Signature final;
    auto Mode() const -> crypto::asymmetric::Mode final { return mode_; }
    auto Role() const -> identity::CredentialRole final { return role_; }
    auto Private() const -> bool final
    {
        return (crypto::asymmetric::Mode::Private == mode_);
    }
    auto Save() const -> bool final;
    auto SelfSignature(CredentialModeFlag version = PUBLIC_VERSION) const
        -> contract::Signature final;
    using Signable::Serialize;
    auto Serialize(Writer&& out) const noexcept -> bool final;
    auto Serialize(
        SerializedType& serialized,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const -> bool final;
    auto SourceSignature() const -> contract::Signature final;
    auto TransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const -> bool override;
    auto Type() const -> identity::CredentialType final { return type_; }
    auto Validate() const noexcept -> bool final;
    auto Verify(
        const Data& plaintext,
        const protobuf::Signature& sig,
        const opentxs::crypto::asymmetric::Role key =
            opentxs::crypto::asymmetric::Role::Sign) const -> bool override
    {
        return false;
    }
    auto Verify(
        const protobuf::Credential& credential,
        const identity::CredentialRole& role,
        const identifier_type& masterID,
        const protobuf::Signature& masterSig) const -> bool override;

    Base() = delete;
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    auto operator=(const Base&) -> Base& = delete;
    auto operator=(Base&&) -> Base& = delete;

    ~Base() override = default;

protected:
    const identity::internal::Authority& parent_;
    const identity::Source& source_;
    const identifier::Nym nym_id_;
    const identifier_type master_id_;
    const identity::CredentialType type_;
    const identity::CredentialRole role_;
    const crypto::asymmetric::Mode mode_;

    static auto get_master_id(
        const api::Session& api,
        const protobuf::Credential& serialized,
        const internal::Primary& master) noexcept(false)
        -> const identifier_type&;

    virtual auto id_form() const -> std::shared_ptr<SerializedType>;
    using Signable::serialize;
    virtual auto serialize(
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const
        -> std::shared_ptr<SerializedType>;
    auto validate() const -> bool final;
    virtual auto verify_internally() const -> bool;

    auto init(
        const identity::credential::internal::Primary& master,
        const PasswordPrompt& reason) noexcept(false) -> void;
    virtual auto sign(
        const identity::credential::internal::Primary& master,
        const PasswordPrompt& reason,
        Signatures& out) noexcept(false) -> void;

    Base(
        const api::Session& api,
        const identity::internal::Authority& owner,
        const identity::Source& source,
        const crypto::Parameters& nymParameters,
        const VersionNumber version,
        const identity::CredentialRole role,
        const crypto::asymmetric::Mode mode,
        const identifier_type& masterID) noexcept;
    Base(
        const api::Session& api,
        const identity::internal::Authority& owner,
        const identity::Source& source,
        const protobuf::Credential& serialized,
        const identifier_type& masterID) noexcept(false);

private:
    static auto extract_signatures(const SerializedType& serialized)
        -> Signatures;

    auto calculate_id() const -> identifier_type final;
    auto clone() const noexcept -> Base* final { return nullptr; }
    // Syntax (non cryptographic) validation
    auto isValid() const -> bool;
    // Returns the serialized form to prevent unnecessary serializations
    auto isValid(std::shared_ptr<SerializedType>& credential) const -> bool;
    auto verify_master_signature() const -> bool;

    auto add_master_signature(
        const identity::credential::internal::Primary& master,
        const PasswordPrompt& reason,
        Signatures& out) noexcept(false) -> void;
};
}  // namespace opentxs::identity::credential::implementation
