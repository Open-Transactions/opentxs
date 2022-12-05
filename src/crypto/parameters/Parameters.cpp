// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::Language
// IWYU pragma: no_forward_declare opentxs::crypto::ParameterType
// IWYU pragma: no_forward_declare opentxs::crypto::SeedStrength
// IWYU pragma: no_forward_declare opentxs::crypto::SeedStyle
// IWYU pragma: no_forward_declare opentxs::crypto::symmetric::Algorithm
// IWYU pragma: no_forward_declare opentxs::identity::CredentialType
// IWYU pragma: no_forward_declare opentxs::identity::SourceProofType
// IWYU pragma: no_forward_declare opentxs::identity::SourceType

#include "opentxs/crypto/Parameters.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

#include "crypto/parameters/Imp.hpp"
#include "internal/crypto/Parameters.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/asymmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Container.hpp"

namespace opentxs::crypto
{
const UnallocatedMap<crypto::asymmetric::Algorithm, ParameterType> key_to_nym_{
    {crypto::asymmetric::Algorithm::Legacy, ParameterType::rsa},
    {crypto::asymmetric::Algorithm::Secp256k1, ParameterType::secp256k1},
    {crypto::asymmetric::Algorithm::ED25519, ParameterType::ed25519},
};
const auto nym_to_key_{reverse_map(key_to_nym_)};

auto swap(Parameters& lhs, Parameters& rhs) noexcept -> void { lhs.swap(rhs); }

auto operator<(const Parameters& lhs, const Parameters& rhs) noexcept -> bool
{
    return lhs.Internal() < rhs.Internal();
}

auto operator==(const Parameters& lhs, const Parameters& rhs) noexcept -> bool
{
    return lhs.Internal() == rhs.Internal();
}

Parameters::Parameters(
    const ParameterType type,
    const identity::CredentialType credential,
    const identity::SourceType source,
    const std::uint8_t pcVersion) noexcept
    : imp_(std::make_unique<Imp>(type, credential, source, pcVersion).release())
{
    OT_ASSERT(imp_);
}

Parameters::Parameters(
    crypto::asymmetric::Algorithm key,
    identity::CredentialType credential,
    const identity::SourceType source,
    const std::uint8_t pcVersion) noexcept
    : Parameters(key_to_nym_.at(key), credential, source, pcVersion)
{
}

Parameters::Parameters(const std::int32_t keySize) noexcept
    : Parameters(ParameterType::rsa, identity::CredentialType::Legacy)
{
    imp_->n_bits_ = keySize;
}

Parameters::Parameters(
    const UnallocatedCString& seedID,
    const int index,
    const std::uint8_t pcVersion) noexcept
    : Parameters()
{
    if (0 < seedID.size()) { SetSeed(seedID); }

    if (index >= 0) { SetNym(static_cast<Bip32Index>(index)); }

    if (0 != pcVersion) { SetPaymentCodeVersion(pcVersion); }
}

Parameters::Parameters(const Parameters& rhs) noexcept
    : imp_(rhs.imp_->clone())
{
}

Parameters::Parameters(Parameters&& rhs) noexcept
    : imp_(std::make_unique<Imp>().release())
{
    swap(rhs);
}

auto Parameters::operator=(const Parameters& rhs) noexcept -> Parameters&
{
    auto temp = std::unique_ptr<Imp>(imp_);

    OT_ASSERT(temp);

    imp_ = rhs.imp_->clone();

    return *this;
}

auto Parameters::operator=(Parameters&& rhs) noexcept -> Parameters&
{
    swap(rhs);

    return *this;
}

auto Parameters::Algorithm() const noexcept -> crypto::asymmetric::Algorithm
{
    try {
        return nym_to_key_.at(imp_->nym_type_);
    } catch (...) {
        return crypto::asymmetric::Algorithm::Error;
    }
}

auto Parameters::ChangeType(const ParameterType type) const noexcept
    -> Parameters
{
    auto output{*this};
    const_cast<ParameterType&>(output.imp_->nym_type_) = type;

    if (ParameterType::rsa == output.imp_->nym_type_) {
        const_cast<identity::CredentialType&>(output.imp_->credential_type_) =
            identity::CredentialType::Legacy;
        const_cast<identity::SourceType&>(output.imp_->source_type_) =
            identity::SourceType::PubKey;
        const_cast<identity::SourceProofType&>(
            output.imp_->source_proof_type_) =
            identity::SourceProofType::SelfSignature;
    }

    return output;
}

auto Parameters::credentialType() const noexcept -> identity::CredentialType
{
    return imp_->credential_type_;
}

auto Parameters::CredIndex() const noexcept -> Bip32Index
{
    return imp_->cred_index_;
}

auto Parameters::Credset() const noexcept -> Bip32Index
{
    return imp_->credset_;
}

auto Parameters::Default() const noexcept -> bool { return imp_->default_; }

auto Parameters::DHParams() const noexcept -> ReadView
{
    return reader(imp_->params_);
}

auto Parameters::Entropy() const noexcept -> const Secret&
{
    return imp_->entropy_;
}

auto Parameters::Internal() const noexcept -> const internal::Parameters&
{
    return *imp_;
}

auto Parameters::Internal() noexcept -> internal::Parameters& { return *imp_; }

auto Parameters::keySize() const noexcept -> std::int32_t
{
    return imp_->n_bits_;
}

auto Parameters::Nym() const noexcept -> Bip32Index { return imp_->nym_; }

auto Parameters::nymParameterType() const noexcept -> ParameterType
{
    return imp_->nym_type_;
}

auto Parameters::PaymentCodeVersion() const noexcept -> std::uint8_t
{
    return imp_->payment_code_version_;
}

auto Parameters::Seed() const noexcept -> UnallocatedCString
{
    return imp_->seed_;
}

auto Parameters::SeedLanguage() const noexcept -> crypto::Language
{
    return imp_->seed_language_;
}
auto Parameters::SeedStrength() const noexcept -> crypto::SeedStrength
{
    return imp_->seed_strength_;
}
auto Parameters::SeedStyle() const noexcept -> crypto::SeedStyle
{
    return imp_->seed_style_;
}

auto Parameters::SourceProofType() const noexcept -> identity::SourceProofType
{
    return imp_->source_proof_type_;
}
auto Parameters::SourceType() const noexcept -> identity::SourceType
{
    return imp_->source_type_;
}

auto Parameters::UseAutoIndex() const noexcept -> bool
{
    return imp_->use_auto_index_;
}

auto Parameters::SetCredIndex(const Bip32Index path) noexcept -> void
{
    imp_->cred_index_ = path;
}

auto Parameters::SetCredset(const Bip32Index path) noexcept -> void
{
    imp_->credset_ = path;
}

auto Parameters::SetDefault(const bool in) noexcept -> void
{
    imp_->default_ = in;
}

auto Parameters::SetDHParams(const ReadView bytes) noexcept -> void
{
    const auto* start = reinterpret_cast<const std::byte*>(bytes.data());
    const auto* end = start + bytes.size();

    imp_->params_.assign(start, end);
}

auto Parameters::SetEntropy(const Secret& entropy) noexcept -> void
{
    imp_->entropy_ = entropy;
}

auto Parameters::setKeySize(std::int32_t keySize) noexcept -> void
{
    imp_->n_bits_ = keySize;
}

auto Parameters::SetNym(const Bip32Index path) noexcept -> void
{
    imp_->nym_ = path;
    imp_->use_auto_index_ = false;
}

auto Parameters::SetPaymentCodeVersion(const std::uint8_t version) noexcept
    -> void
{
    imp_->payment_code_version_ = version;
}

auto Parameters::SetSeed(const UnallocatedCString& seed) noexcept -> void
{
    imp_->seed_ = seed;
}

auto Parameters::SetSeedLanguage(const crypto::Language lang) noexcept -> void
{
    imp_->seed_language_ = lang;
}

auto Parameters::SetSeedStrength(const crypto::SeedStrength type) noexcept
    -> void
{
    imp_->seed_strength_ = type;
}

auto Parameters::SetSeedStyle(const crypto::SeedStyle type) noexcept -> void
{
    imp_->seed_style_ = type;
}

auto Parameters::SetUseAutoIndex(const bool use) noexcept -> void
{
    imp_->use_auto_index_ = use;
}

auto Parameters::swap(Parameters& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
}

Parameters::~Parameters()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::crypto
