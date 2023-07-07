// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/core/contract/Blank.hpp"

#include "internal/crypto/key/Null.hpp"
#include "internal/identity/credential/Credential.hpp"
#include "opentxs/identity/credential/Base.hpp"
#include "opentxs/identity/credential/Key.hpp"

namespace opentxs::identity::credential::blank
{
struct Base : virtual public contract::blank::Signable2<identifier::Generic>,
              virtual public credential::internal::Base {
    auto asString(const bool) const -> UnallocatedCString final { return {}; }
    auto CredentialID() const -> const identifier::Generic& final
    {
        static const auto blank = identifier::Generic{};

        return blank;
    }
    auto GetContactData(proto::ContactData&) const -> bool final { return {}; }
    auto GetVerificationSet(proto::VerificationSet&) const -> bool final
    {
        return {};
    }
    auto hasCapability(const NymCapability&) const -> bool final { return {}; }
    auto MasterSignature() const -> contract::Signature final { return {}; }
    auto Mode() const -> crypto::asymmetric::Mode final { return {}; }
    auto Private() const -> bool final { return {}; }
    auto Role() const -> identity::CredentialRole final { return {}; }
    auto Save() const -> bool final { return {}; }
    auto SelfSignature(CredentialModeFlag) const -> contract::Signature final
    {
        return {};
    }
    using Signable2::Serialize;
    auto Serialize(
        SerializedType&,
        const SerializationModeFlag,
        const SerializationSignatureFlag) const -> bool final
    {
        return {};
    }
    auto SourceSignature() const -> contract::Signature final { return {}; }
    auto TransportKey(Data&, Secret&, const PasswordPrompt&) const -> bool final
    {
        return {};
    }
    auto Type() const -> identity::CredentialType final { return {}; }
    auto Verify(
        const Data&,
        const proto::Signature&,
        const opentxs::crypto::asymmetric::Role) const -> bool final
    {
        return {};
    }
    auto Verify(
        const proto::Credential&,
        const identity::CredentialRole&,
        const identifier::Generic&,
        const proto::Signature&) const -> bool final
    {
        return {};
    }
};

struct Key : virtual public blank::Base,
             virtual public credential::internal::Key {
    auto GetKeypair(
        const crypto::asymmetric::Algorithm,
        const opentxs::crypto::asymmetric::Role) const
        -> const crypto::key::Keypair& final
    {
        static const auto blank = crypto::key::blank::Keypair{};

        return blank;
    }
    auto GetKeypair(const opentxs::crypto::asymmetric::Role) const
        -> const crypto::key::Keypair& final
    {
        static const auto blank = crypto::key::blank::Keypair{};

        return blank;
    }
    auto GetPublicKeysBySignature(
        crypto::key::Keypair::Keys&,
        const opentxs::Signature&,
        char cKeyType) const -> std::int32_t final
    {
        return {};
    }
    auto Sign(
        const GetPreimage,
        const crypto::SignatureRole,
        proto::Signature&,
        const PasswordPrompt&,
        opentxs::crypto::asymmetric::Role,
        const crypto::HashType hash) const -> bool final
    {
        return {};
    }
};
}  // namespace opentxs::identity::credential::blank
