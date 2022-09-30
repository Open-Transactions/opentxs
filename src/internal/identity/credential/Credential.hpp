// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Enums.pb.h>
#include <optional>

#include "internal/identity/Types.hpp"
#include "internal/identity/credential/Types.hpp"
#include "opentxs/core/Secret.hpp"  // IWYU pragma: keep
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/credential/Contact.hpp"
#include "opentxs/identity/credential/Key.hpp"
#include "opentxs/identity/credential/Primary.hpp"
#include "opentxs/identity/credential/Secondary.hpp"
#include "opentxs/identity/credential/Verification.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace proto
{
class Credential;
class ContactData;
class VerificationSet;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::credential::internal
{
class Base : virtual public identity::credential::Base
{
public:
    using SerializedType = proto::Credential;

    virtual auto GetContactData(proto::ContactData& contactData) const
        -> bool = 0;
    virtual auto GetVerificationSet(
        proto::VerificationSet& verificationSet) const -> bool = 0;
    auto Internal() const noexcept -> const internal::Base& final
    {
        return *this;
    }
    virtual void ReleaseSignatures(const bool onlyPrivate) = 0;
    virtual auto SelfSignature(
        CredentialModeFlag version = PUBLIC_VERSION) const -> Signature = 0;
    using Signable::Serialize;
    virtual auto Serialize(
        SerializedType& serialized,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const -> bool = 0;
    virtual auto Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const opentxs::crypto::key::asymmetric::Role key =
            opentxs::crypto::key::asymmetric::Role::Sign) const -> bool = 0;
    virtual auto Verify(
        const proto::Credential& credential,
        const identity::CredentialRole& role,
        const identifier::Generic& masterID,
        const proto::Signature& masterSig) const -> bool = 0;

    auto Internal() noexcept -> internal::Base& final { return *this; }

#ifdef _MSC_VER
    Base() {}
#endif  // _MSC_VER
    ~Base() override = default;
};
struct Contact : virtual public Base,
                 virtual public identity::credential::Contact {
#ifdef _MSC_VER
    Contact() {}
#endif  // _MSC_VER
    ~Contact() override = default;
};
struct Key : virtual public Base, virtual public identity::credential::Key {
    virtual auto SelfSign(
        const PasswordPrompt& reason,
        const std::optional<OTSecret> exportPassword = {},
        const bool onlyPrivate = false) -> bool = 0;

#ifdef _MSC_VER
    Key() {}
#endif  // _MSC_VER
    ~Key() override = default;
};
struct Primary : virtual public Key,
                 virtual public identity::credential::Primary {
#ifdef _MSC_VER
    Primary() {}
#endif  // _MSC_VER
    ~Primary() override = default;
};
struct Secondary : virtual public Key,
                   virtual public identity::credential::Secondary {
#ifdef _MSC_VER
    Secondary() {}
#endif  // _MSC_VER
    ~Secondary() override = default;
};
struct Verification : virtual public Base,
                      virtual public identity::credential::Verification {
#ifdef _MSC_VER
    Verification() {}
#endif  // _MSC_VER
    ~Verification() override = default;
};
}  // namespace opentxs::identity::credential::internal

namespace opentxs
{
auto translate(const identity::CredentialRole in) noexcept
    -> proto::CredentialRole;
auto translate(const identity::CredentialType in) noexcept
    -> proto::CredentialType;
auto translate(const proto::CredentialRole in) noexcept
    -> identity::CredentialRole;
auto translate(const proto::CredentialType in) noexcept
    -> identity::CredentialType;
}  // namespace opentxs
