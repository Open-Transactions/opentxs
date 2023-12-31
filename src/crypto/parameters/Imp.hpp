// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "internal/crypto/Parameters.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/HDSeed.hpp"
#include "opentxs/identity/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Factory;
}  // namespace api

namespace protobuf
{
class ContactData;
class VerificationSet;
}  // namespace protobuf
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
class Parameters::Imp final : public internal::Parameters
{
public:
    const api::Factory& factory_;
    const ParameterType nym_type_;
    const identity::CredentialType credential_type_;
    const identity::SourceType source_type_;
    const identity::SourceProofType source_proof_type_;
    std::uint8_t payment_code_version_;
    crypto::SeedStyle seed_style_;
    crypto::Language seed_language_;
    crypto::SeedStrength seed_strength_;
    Secret entropy_;
    SeedID seed_;
    Bip32Index nym_;
    Bip32Index credset_;
    Bip32Index cred_index_;
    bool default_;
    bool use_auto_index_;
    std::int32_t n_bits_;
    Space params_;
    OTKeypair source_keypair_;
    std::shared_ptr<protobuf::ContactData> contact_data_;
    std::shared_ptr<protobuf::VerificationSet> verification_set_;
    mutable std::optional<ByteArray> hashed_;

    auto operator<(const Parameters& rhs) const noexcept -> bool final;
    auto operator==(const Parameters& rhs) const noexcept -> bool final;

    auto clone() const noexcept -> Imp*;
    auto GetContactData(protobuf::ContactData& serialized) const noexcept
        -> bool final;
    auto GetVerificationSet(
        protobuf::VerificationSet& serialized) const noexcept -> bool final;
    auto Hash() const noexcept -> ByteArray final;
    auto Keypair() const noexcept -> const key::Keypair& final
    {
        return source_keypair_;
    }

    auto Keypair() noexcept -> OTKeypair& final { return source_keypair_; }
    auto SetContactData(const protobuf::ContactData& contactData) noexcept
        -> void final;
    auto SetVerificationSet(
        const protobuf::VerificationSet& verificationSet) noexcept
        -> void final;

    Imp(const api::Factory& factory,
        const ParameterType type,
        const identity::CredentialType credential,
        const identity::SourceType source,
        const std::uint8_t pcVersion) noexcept;
    Imp(const api::Factory& factory) noexcept;
    Imp(const Imp& rhs) noexcept;
};
}  // namespace opentxs::crypto
