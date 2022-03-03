// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace proto
{
class Credential;
class ContactData;
class VerificationSet;
}  // namespace proto

class PasswordPrompt;
class Secret;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential
{
class OPENTXS_EXPORT Base : virtual public opentxs::contract::Signable
{
public:
    using SerializedType = proto::Credential;

    virtual auto asString(const bool asPrivate = false) const
        -> UnallocatedCString = 0;
    virtual auto CredentialID() const -> const Identifier& = 0;
    virtual auto GetContactData(proto::ContactData& contactData) const
        -> bool = 0;
    virtual auto GetVerificationSet(
        proto::VerificationSet& verificationSet) const -> bool = 0;
    virtual auto hasCapability(const NymCapability& capability) const
        -> bool = 0;
    virtual auto MasterSignature() const -> Signature = 0;
    virtual auto Mode() const -> crypto::key::asymmetric::Mode = 0;
    virtual auto Role() const -> identity::CredentialRole = 0;
    virtual auto Private() const -> bool = 0;
    virtual auto Save() const -> bool = 0;
    virtual auto SelfSignature(
        CredentialModeFlag version = PUBLIC_VERSION) const -> Signature = 0;
    using Signable::Serialize;
    OPENTXS_NO_EXPORT virtual auto Serialize(
        SerializedType& serialized,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const -> bool = 0;
    virtual auto SourceSignature() const -> Signature = 0;
    virtual auto TransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto Type() const -> identity::CredentialType = 0;
    virtual auto Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const opentxs::crypto::key::asymmetric::Role key =
            opentxs::crypto::key::asymmetric::Role::Sign) const -> bool = 0;
    virtual auto Verify(
        const proto::Credential& credential,
        const identity::CredentialRole& role,
        const Identifier& masterID,
        const proto::Signature& masterSig) const -> bool = 0;

    ~Base() override = default;

protected:
    Base() noexcept {}  // TODO Signable

private:
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    auto operator=(const Base&) -> Base& = delete;
    auto operator=(Base&&) -> Base& = delete;
};
}  // namespace opentxs::identity::credential
