// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/api/Context.hpp"
#include "internal/crypto/Parameters.hpp"

namespace opentxs::crypto
{
class Parameters;
namespace internal
{
class Parameters;
}
class ParametersMock : public Parameters {
public:
    MOCK_METHOD(key::asymmetric::Algorithm, Algorithm, (), (const, noexcept, override));
    MOCK_METHOD(Parameters, ChangeType, (const ParameterType type), (const, noexcept, override));
    MOCK_METHOD(identity::CredentialType, credentialType, (), (const, noexcept, override));
    MOCK_METHOD(Bip32Index, CredIndex, (), (const, noexcept, override));
    MOCK_METHOD(Bip32Index, Credset, (), (const, noexcept, override));
    MOCK_METHOD(bool, Default, (), (const, noexcept, override));
    MOCK_METHOD(ReadView, DHParams, (), (const, noexcept, override));
    MOCK_METHOD(const Secret&, Entropy, (), (const, noexcept, override));
    MOCK_METHOD(const key::Keypair&, Keypair, (), (const, noexcept, override));
    MOCK_METHOD(std::int32_t, keySize, (), (const, noexcept, override));
    MOCK_METHOD(const internal::Parameters&, Internal, (), (const, noexcept, override));
    MOCK_METHOD(Bip32Index, Nym, (), (const, noexcept, override));
    MOCK_METHOD(ParameterType, nymParameterType, (), (const, noexcept, override));
    MOCK_METHOD(std::uint8_t, PaymentCodeVersion, (), (const, noexcept, override));
    MOCK_METHOD(UnallocatedCString, Seed, (), (const, noexcept, override));
    MOCK_METHOD(Language, SeedLanguage, (), (const, noexcept, override));
    MOCK_METHOD(crypto::SeedStrength, SeedStrength, (), (const, noexcept, override));
    MOCK_METHOD(crypto::SeedStyle, SeedStyle, (), (const, noexcept, override));
    MOCK_METHOD(identity::SourceProofType, SourceProofType, (), (const, noexcept, override));
    MOCK_METHOD(identity::SourceType, SourceType, (), (const, noexcept, override));
    MOCK_METHOD(bool, UseAutoIndex, (), (const, noexcept, override));

    MOCK_METHOD(internal::Parameters &, Internal, (), (noexcept, override));
    MOCK_METHOD(OTKeypair &, Keypair, (), (noexcept, override));
    MOCK_METHOD(void, SetCredIndex, (const Bip32Index path), (noexcept, override));
    MOCK_METHOD(void, SetCredset, (const Bip32Index path), (noexcept, override));
    MOCK_METHOD(void, SetDefault, (const bool in), (noexcept, override));
    MOCK_METHOD(void, SetEntropy, (const Secret& entropy), (noexcept, override));
    MOCK_METHOD(void, setKeySize, (std::int32_t keySize), (noexcept, override));
    MOCK_METHOD(void, SetNym, (const Bip32Index path), (noexcept, override));
    MOCK_METHOD(void, SetDHParams, (const ReadView bytes), (noexcept, override));
    MOCK_METHOD(void, SetPaymentCodeVersion, (const std::uint8_t version), (noexcept, override));
    MOCK_METHOD(void, SetSeed, (const UnallocatedCString& seed), (noexcept, override));
    MOCK_METHOD(void, SetSeedLanguage, (const Language lang), (noexcept, override));
    MOCK_METHOD(void, SetSeedStrength, (const crypto::SeedStrength value), (noexcept, override));
    MOCK_METHOD(void, SetSeedStyle, (const crypto::SeedStyle type), (noexcept, override));
    MOCK_METHOD(void, SetUseAutoIndex, (const bool use), (noexcept, override));
    MOCK_METHOD(void, swap, (Parameters & rhs), (noexcept, override));
};
}