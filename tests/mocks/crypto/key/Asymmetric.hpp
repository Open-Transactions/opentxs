// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "opentxs/crypto/key/Asymmetric.hpp"

namespace opentxs::crypto::key
{

class AsymmetricMock : public Asymmetric
{
public:
    MOCK_METHOD(
        std::unique_ptr<Asymmetric>,
        asPublic,
        (),
        (const, noexcept, override));
    MOCK_METHOD(
        OTData,
        CalculateHash,
        (const crypto::HashType hashType, const PasswordPrompt& reason),
        (const, noexcept, override));

    MOCK_METHOD(
        bool,
        CalculateID,
        (Identifier & theOutput),
        (const, noexcept, override));

    MOCK_METHOD(
        bool,
        CalculateTag,
        (const identity::Authority& nym,
         const crypto::key::asymmetric::Algorithm type,
         const PasswordPrompt& reason,
         std::uint32_t& tag,
         Secret& password),
        (const, noexcept, override));

    MOCK_METHOD(
        bool,
        CalculateTag,
        (const Asymmetric& dhKey,
         const Identifier& credential,
         const PasswordPrompt& reason,
         std::uint32_t& tag),
        (const, noexcept, override));

    MOCK_METHOD(
        bool,
        CalculateSessionPassword,
        (const Asymmetric& dhKey,
         const PasswordPrompt& reason,
         Secret& password),
        (const, noexcept, override));

    MOCK_METHOD(
        const opentxs::crypto::AsymmetricProvider&,
        engine,
        (),
        (const, noexcept, override));

    MOCK_METHOD(
        const OTSignatureMetadata*,
        GetMetadata,
        (),
        (const, noexcept, override));
    MOCK_METHOD(
        bool,
        hasCapability,
        (const NymCapability& capability),
        (const, noexcept, override));
    MOCK_METHOD(bool, HasPrivate, (), (const, noexcept, override));
    MOCK_METHOD(bool, HasPublic, (), (const, noexcept, override));
    MOCK_METHOD(
        crypto::key::asymmetric::Algorithm,
        keyType,
        (),
        (const, noexcept, override));
    MOCK_METHOD(ReadView, Params, (), (const, noexcept, override));
    MOCK_METHOD(
        const UnallocatedCString,
        Path,
        (),
        (const, noexcept, override));

    MOCK_METHOD(
        bool,
        Path,
        (proto::HDPath & output),
        (const, noexcept, override));
    MOCK_METHOD(
        ReadView,
        PrivateKey,
        (const PasswordPrompt& reason),
        (const, noexcept, override));
    MOCK_METHOD(ReadView, PublicKey, (), (const, noexcept, override));
    MOCK_METHOD(
        opentxs::crypto::key::asymmetric::Role,
        Role,
        (),
        (const, noexcept, override));

    MOCK_METHOD(
        bool,
        Serialize,
        (Serialized & serialized),
        (const, noexcept, override));
    MOCK_METHOD(crypto::HashType, SigHashType, (), (const, noexcept, override));

    MOCK_METHOD(
        bool,
        Sign,
        (const GetPreimage input,
         const crypto::SignatureRole role,
         proto::Signature& signature,
         const Identifier& credential,
         const PasswordPrompt& reason,
         const crypto::HashType hash),
        (const, noexcept, override));

    MOCK_METHOD(
        bool,
        Sign,
        (const ReadView preimage,
         const crypto::HashType hash,
         const AllocateOutput output,
         const PasswordPrompt& reason),
        (const, noexcept, override));

    MOCK_METHOD(
        bool,
        TransportKey,
        (Data & publicKey, Secret& privateKey, const PasswordPrompt& reason),
        (const, noexcept, override));

    MOCK_METHOD(
        bool,
        Verify,
        (const Data& plaintext, const proto::Signature& sig),
        (const, noexcept, override));

    MOCK_METHOD(VersionNumber, Version, (), (const, noexcept, override));

    MOCK_METHOD(bool, BracketOperator, (), (const, noexcept));
    virtual operator bool() const noexcept { return true; }

    MOCK_METHOD(bool, EqualOperator, (), (const, noexcept));
    virtual bool operator==(const Serialized&) const noexcept { return true; }

    MOCK_METHOD(Asymmetric*, clone, (), (const, noexcept, override));
};
}  // namespace opentxs::crypto::key