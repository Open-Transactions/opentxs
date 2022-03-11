// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "opentxs/crypto/key/Keypair.hpp"

namespace opentxs::crypto::key
{

class KeypairMock : public Keypair
{
public:
    MOCK_METHOD(bool, BracketOperator, (), (const, noexcept));
    virtual operator bool() const noexcept { return true; }

    MOCK_METHOD(
        bool,
        CheckCapability,
        (const NymCapability& capability),
        (const, noexcept, override));
    MOCK_METHOD(const Asymmetric&, GetPrivateKey, (), (const, override));
    MOCK_METHOD(const Asymmetric&, GetPublicKey, (), (const, override));
    MOCK_METHOD(
        std::int32_t,
        GetPublicKeyBySignature,
        (Keys & listOutput, const Signature& theSignature, bool bInclusive),
        (const, noexcept, override));
    MOCK_METHOD(
        bool,
        Serialize,
        (proto::AsymmetricKey & serialized, bool privateKey),
        (const, noexcept, override));
    MOCK_METHOD(
        bool,
        GetTransportKey,
        (Data & publicKey, Secret& privateKey, const PasswordPrompt& reason),
        (const, noexcept, override));
    MOCK_METHOD(Keypair*, clone, (), (const, override));
};
}  // namespace opentxs::crypto::key