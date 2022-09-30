// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/identity/Authority.hpp"

#include "internal/identity/Types.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identity
{
namespace credential
{
class Primary;
}  // namespace credential
}  // namespace identity

namespace proto
{
class Authority;
class ContactData;
class HDPath;
class Signature;
class Verification;
class VerificationSet;
}  // namespace proto
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::internal
{
class Authority : virtual public identity::Authority
{
public:
    using Serialized = proto::Authority;

    static auto NymToContactCredential(const VersionNumber nym) noexcept(false)
        -> VersionNumber;

    virtual auto GetContactData(proto::ContactData& contactData) const
        -> bool = 0;
    virtual auto GetMasterCredential() const -> const credential::Primary& = 0;
    auto Internal() const noexcept -> const internal::Authority& final
    {
        return *this;
    }
    virtual auto GetVerificationSet(
        proto::VerificationSet& verificationSet) const -> bool = 0;
    virtual auto Path(proto::HDPath& output) const -> bool = 0;
    virtual auto Serialize(
        Serialized& serialized,
        const CredentialIndexModeFlag mode) const -> bool = 0;
    virtual auto Sign(
        const GetPreimage input,
        const crypto::SignatureRole role,
        proto::Signature& signature,
        const PasswordPrompt& reason,
        opentxs::crypto::key::asymmetric::Role key =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const crypto::HashType hash = crypto::HashType::Error) const
        -> bool = 0;
    virtual auto Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const opentxs::crypto::key::asymmetric::Role key =
            opentxs::crypto::key::asymmetric::Role::Sign) const -> bool = 0;
    virtual auto Verify(const proto::Verification& item) const -> bool = 0;
    virtual auto WriteCredentials() const -> bool = 0;

    virtual auto AddVerificationCredential(
        const proto::VerificationSet& verificationSet,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto AddContactCredential(
        const proto::ContactData& contactData,
        const PasswordPrompt& reason) -> bool = 0;
    auto Internal() noexcept -> internal::Authority& final { return *this; }

    ~Authority() override = default;
};
}  // namespace opentxs::identity::internal
