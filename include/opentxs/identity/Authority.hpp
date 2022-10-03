// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>

#include "opentxs/Export.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identity
{
namespace credential
{
class Key;
}  // namespace credential

namespace internal
{
class Authority;
}  // namespace internal
}  // namespace identity

class Secret;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity
{
class OPENTXS_EXPORT Authority
{
public:
    using AuthorityKeys = Nym::AuthorityKeys;

    virtual auto ContactCredentialVersion() const -> VersionNumber = 0;
    virtual auto EncryptionTargets() const noexcept -> AuthorityKeys = 0;
    virtual auto GetMasterCredID() const -> identifier::Generic = 0;
    virtual auto GetPublicAuthKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPublicEncrKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType = '0') const -> std::int32_t = 0;
    virtual auto GetPublicSignKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPrivateSignKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPrivateEncrKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPrivateAuthKey(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetAuthKeypair(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Keypair& = 0;
    virtual auto GetEncrKeypair(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Keypair& = 0;
    virtual auto GetSignKeypair(
        crypto::key::asymmetric::Algorithm keytype,
        const String::List* plistRevokedIDs = nullptr) const
        -> const crypto::key::Keypair& = 0;
    virtual auto GetTagCredential(crypto::key::asymmetric::Algorithm keytype)
        const noexcept(false) -> const credential::Key& = 0;
    virtual auto hasCapability(const NymCapability& capability) const
        -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Authority& = 0;
    virtual auto Params(const crypto::key::asymmetric::Algorithm type)
        const noexcept -> ReadView = 0;
    virtual auto Source() const -> const identity::Source& = 0;
    virtual auto TransportKey(
        Data& publicKey,
        Secret& privateKey,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto Unlock(
        const crypto::key::Asymmetric& dhKey,
        const std::uint32_t tag,
        const crypto::key::asymmetric::Algorithm type,
        const crypto::key::Symmetric& key,
        PasswordPrompt& reason) const noexcept -> bool = 0;
    virtual auto VerificationCredentialVersion() const -> VersionNumber = 0;
    virtual auto VerifyInternally() const -> bool = 0;

    virtual auto AddChildKeyCredential(
        const crypto::Parameters& nymParameters,
        const PasswordPrompt& reason) -> UnallocatedCString = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Authority& = 0;
    virtual void RevokeContactCredentials(
        UnallocatedList<UnallocatedCString>& contactCredentialIDs) = 0;
    virtual void RevokeVerificationCredentials(
        UnallocatedList<UnallocatedCString>& verificationCredentialIDs) = 0;

    Authority(const Authority&) = delete;
    Authority(Authority&&) = delete;
    auto operator=(const Authority&) -> Authority& = delete;
    auto operator=(Authority&&) -> Authority& = delete;

    virtual ~Authority() = default;

protected:
    Authority() noexcept = default;
};
}  // namespace opentxs::identity
