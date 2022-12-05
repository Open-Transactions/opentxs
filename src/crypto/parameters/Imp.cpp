// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::Language
// IWYU pragma: no_forward_declare opentxs::crypto::ParameterType
// IWYU pragma: no_forward_declare opentxs::crypto::SeedStrength
// IWYU pragma: no_forward_declare opentxs::crypto::SeedStyle
// IWYU pragma: no_forward_declare opentxs::identity::CredentialType
// IWYU pragma: no_forward_declare opentxs::identity::SourceProofType
// IWYU pragma: no_forward_declare opentxs::identity::SourceType

#include "crypto/parameters/Imp.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <ContactData.pb.h>
#include <VerificationSet.pb.h>
#include <compare>
#include <cstdint>
#include <memory>

#include "internal/crypto/key/Factory.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto
{
Parameters::Imp::Imp(
    const ParameterType type,
    const identity::CredentialType credential,
    const identity::SourceType source,
    const std::uint8_t pcVersion) noexcept
    : nym_type_(type)
    , credential_type_(
          (ParameterType::rsa == nym_type_) ? identity::CredentialType::Legacy
                                            : credential)
    , source_type_(
          (ParameterType::rsa == nym_type_) ? identity::SourceType::PubKey
                                            : source)
    , source_proof_type_(
          (identity::SourceType::Bip47 == source_type_)
              ? identity::SourceProofType::Signature
              : identity::SourceProofType::SelfSignature)
    , payment_code_version_(
          (0 == pcVersion) ? PaymentCode::DefaultVersion() : pcVersion)
    , seed_style_(crypto::SeedStyle::BIP39)
    , seed_language_(crypto::Language::en)
    , seed_strength_(crypto::SeedStrength::TwentyFour)
    , entropy_(Context().Factory().Secret(0))
    , seed_("")
    , nym_(0)
    , credset_(0)
    , cred_index_(0)
    , default_(true)
    , use_auto_index_(true)
    , n_bits_(1024)
    , params_()
    , source_keypair_(factory::Keypair())
    , contact_data_(nullptr)
    , verification_set_(nullptr)
    , hashed_(std::nullopt)
{
}

Parameters::Imp::Imp() noexcept
    : Imp(crypto::Parameters::DefaultType(),
          crypto::Parameters::DefaultCredential(),
          crypto::Parameters::DefaultSource(),
          0)
{
}

Parameters::Imp::Imp(const Imp& rhs) noexcept
    : Imp(rhs.nym_type_,
          rhs.credential_type_,
          rhs.source_type_,
          rhs.payment_code_version_)
{
    entropy_ = rhs.entropy_;
    seed_ = rhs.seed_;
    nym_ = rhs.nym_;
    credset_ = rhs.credset_;
    cred_index_ = rhs.cred_index_;
    default_ = rhs.default_;
    use_auto_index_ = rhs.use_auto_index_;
    n_bits_ = rhs.n_bits_;
    params_ = rhs.params_;
    contact_data_ = rhs.contact_data_;
    verification_set_ = rhs.verification_set_;
    hashed_ = rhs.hashed_;
}

auto Parameters::Imp::clone() const noexcept -> Imp*
{
    return std::make_unique<Imp>(*this).release();
}

auto Parameters::Imp::GetContactData(
    proto::ContactData& serialized) const noexcept -> bool
{
    if (contact_data_) {
        serialized = *contact_data_;

        return true;
    }

    return false;
}

auto Parameters::Imp::GetVerificationSet(
    proto::VerificationSet& serialized) const noexcept -> bool
{
    if (verification_set_) {
        serialized = *verification_set_;

        return true;
    }

    return false;
}

auto Parameters::Imp::Hash() const noexcept -> ByteArray
{
    if (false == hashed_.has_value()) {
        auto& out = hashed_.emplace(ByteArray{});
        out.Concatenate(&nym_type_, sizeof(nym_type_));
        out.Concatenate(&credential_type_, sizeof(credential_type_));
        out.Concatenate(&source_type_, sizeof(source_type_));
        out.Concatenate(&source_proof_type_, sizeof(source_proof_type_));
        out.Concatenate(&payment_code_version_, sizeof(payment_code_version_));
        out.Concatenate(&seed_style_, sizeof(seed_style_));
        out.Concatenate(&seed_language_, sizeof(seed_language_));
        out.Concatenate(&seed_strength_, sizeof(seed_strength_));
        out.Concatenate(entropy_.data(), entropy_.size());  // TODO hash this
        out.Concatenate(seed_.data(), seed_.size());
        out.Concatenate(&nym_, sizeof(nym_));
        out.Concatenate(&credset_, sizeof(credset_));
        out.Concatenate(&cred_index_, sizeof(cred_index_));
        out.Concatenate(&default_, sizeof(default_));
        out.Concatenate(&use_auto_index_, sizeof(use_auto_index_));
        out.Concatenate(&n_bits_, sizeof(n_bits_));
        out.Concatenate(params_.data(), params_.size());
        const auto keypair = [this] {
            auto out = Space{};
            auto proto = proto::AsymmetricKey{};
            source_keypair_->Serialize(proto, false);
            proto::write(proto, writer(out));

            return out;
        }();
        out.Concatenate(keypair.data(), keypair.size());

        if (contact_data_) {
            const auto data = [this] {
                auto out = Space{};
                proto::write(*contact_data_, writer(out));

                return out;
            }();
            out.Concatenate(data.data(), data.size());
        }

        if (verification_set_) {
            const auto data = [this] {
                auto out = Space{};
                proto::write(*verification_set_, writer(out));

                return out;
            }();
            out.Concatenate(data.data(), data.size());
        }
    }

    return hashed_.value();
}

auto Parameters::Imp::operator<(const Parameters& rhs) const noexcept -> bool
{
    // TODO optimize by performing per-member comparison
    return Hash() < rhs.Hash();
}

auto Parameters::Imp::operator==(const Parameters& rhs) const noexcept -> bool
{
    // TODO optimize by performing per-member comparison
    return Hash() != rhs.Hash();
}

auto Parameters::Imp::SetContactData(const proto::ContactData& in) noexcept
    -> void
{
    contact_data_ = std::make_unique<proto::ContactData>(in);
}

auto Parameters::Imp::SetVerificationSet(
    const proto::VerificationSet& in) noexcept -> void
{
    verification_set_ = std::make_unique<proto::VerificationSet>(in);
}
}  // namespace opentxs::crypto
