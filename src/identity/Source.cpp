// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "identity/Source.hpp"           // IWYU pragma: associated
#include "opentxs/internal.factory.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <opentxs/protobuf/AsymmetricKey.pb.h>
#include <opentxs/protobuf/Credential.pb.h>
#include <opentxs/protobuf/Enums.pb.h>
#include <opentxs/protobuf/KeyCredential.pb.h>
#include <opentxs/protobuf/MasterCredentialParameters.pb.h>
#include <opentxs/protobuf/NymIDSource.pb.h>
#include <opentxs/protobuf/SourceProof.pb.h>
#include <functional>
#include <memory>
#include <stdexcept>

#include "internal/core/Armored.hpp"
#include "internal/core/PaymentCode.hpp"
#include "internal/core/String.hpp"
#include "internal/crypto/Parameters.hpp"
#include "internal/crypto/asymmetric/Key.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/crypto/EcdsaCurve.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/Algorithm.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Key.hpp"
#include "opentxs/crypto/asymmetric/Role.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identifier/HDSeed.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/CredentialType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/SourceType.hpp"
#include "opentxs/identity/credential/Primary.hpp"
#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto Factory::NymIDSource(
    const api::Session& api,
    crypto::Parameters& params,
    const opentxs::PasswordPrompt& reason) -> identity::Source*
{
    using ReturnType = identity::implementation::Source;

    switch (params.SourceType()) {
        case identity::SourceType::Bip47: {
            if ((false == api::crypto::HaveHDKeys()) ||
                (false == api::crypto::HaveSupport(
                              crypto::asymmetric::Algorithm::Secp256k1))) {
                throw std::runtime_error("Missing BIP-47 support");
            }

            const auto paymentCode = api.Factory().PaymentCode(
                params.Seed(),
                params.Nym(),
                params.PaymentCodeVersion(),
                reason);

            return new ReturnType{api.Crypto(), api.Factory(), paymentCode};
        }
        case identity::SourceType::PubKey:
            switch (params.credentialType()) {
                case identity::CredentialType::Legacy: {
                    params.Internal().Keypair() =
                        api.Factory().Internal().Session().Keypair(
                            params,
                            crypto::asymmetric::Key::DefaultVersion(),
                            opentxs::crypto::asymmetric::Role::Sign,
                            reason);
                } break;
                case identity::CredentialType::HD: {
                    if (false == api::crypto::HaveHDKeys()) {
                        throw std::runtime_error("Missing HD key support");
                    }

                    const auto curve =
                        crypto::AsymmetricProvider::KeyTypeToCurve(
                            params.Algorithm());

                    if (crypto::EcdsaCurve::invalid == curve) {
                        throw std::runtime_error("Invalid curve type");
                    }

                    params.Internal().Keypair() =
                        api.Factory().Internal().Session().Keypair(
                            params.Seed(),
                            params.Nym(),
                            params.Credset(),
                            params.CredIndex(),
                            curve,
                            opentxs::crypto::asymmetric::Role::Sign,
                            reason);
                } break;
                case identity::CredentialType::Error:
                default: {
                    throw std::runtime_error("Unsupported credential type");
                }
            }

            if (false == bool(params.Internal().Keypair().get())) {
                LogError()()("Failed to generate signing keypair").Flush();

                return nullptr;
            }

            return new ReturnType{api.Crypto(), api.Factory(), params};
        case identity::SourceType::Error:
        default: {
            LogError()()("Unsupported source type.").Flush();

            return nullptr;
        }
    }
}

auto Factory::NymIDSource(
    const api::Session& api,
    const protobuf::NymIDSource& serialized) -> identity::Source*
{
    using ReturnType = identity::implementation::Source;

    return new ReturnType{api.Crypto(), api.Factory(), serialized};
}
}  // namespace opentxs

namespace opentxs::identity::implementation
{
const VersionConversionMap Source::key_to_source_version_{
    {1, 1},
    {2, 2},
    {3, 2},
};

Source::Source(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const protobuf::NymIDSource& serialized) noexcept
    : crypto_(crypto)
    , factory_(factory)
    , type_(translate(serialized.type()))
    , pubkey_(deserialize_pubkey(factory, type_, serialized))
    , payment_code_(deserialize_paymentcode(factory, type_, serialized))
    , version_(serialized.version())
{
}

Source::Source(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const crypto::Parameters& nymParameters) noexcept(false)
    : crypto_(crypto)
    , factory_{factory}
    , type_(nymParameters.SourceType())
    , pubkey_(nymParameters.Internal().Keypair().GetPublicKey())
    , payment_code_()
    , version_(key_to_source_version_.at(pubkey_.Version()))

{
    if (false == pubkey_.IsValid()) {
        throw std::runtime_error("Invalid pubkey");
    }
}

Source::Source(
    const api::Crypto& crypto,
    const api::session::Factory& factory,
    const PaymentCode& source) noexcept
    : crypto_(crypto)
    , factory_{factory}
    , type_(identity::SourceType::Bip47)
    , pubkey_()
    , payment_code_{source}
    , version_(key_to_source_version_.at(payment_code_.Version()))
{
}

Source::Source(const Source& rhs) noexcept
    : Source(
          rhs.crypto_,
          rhs.factory_,
          [&](const Source& source) -> protobuf::NymIDSource {
              auto serialized = protobuf::NymIDSource{};
              source.Serialize(serialized);
              return serialized;
          }(rhs))
{
}

auto Source::asData() const -> ByteArray
{
    auto serialized = protobuf::NymIDSource{};
    if (false == Serialize(serialized)) { return ByteArray{nullptr}; }

    return factory_.Internal().Data(serialized);
}

auto Source::deserialize_paymentcode(
    const api::session::Factory& factory,
    const identity::SourceType type,
    const protobuf::NymIDSource& serialized) -> PaymentCode
{
    if (identity::SourceType::Bip47 == type) {

        return factory.Internal().Session().PaymentCode(
            serialized.paymentcode());
    } else {

        return {};
    }
}

auto Source::deserialize_pubkey(
    const api::session::Factory& factory,
    const identity::SourceType type,
    const protobuf::NymIDSource& serialized) -> crypto::asymmetric::Key
{
    if (identity::SourceType::PubKey == type) {

        return factory.Internal().Session().AsymmetricKey(serialized.key());
    } else {

        return {};
    }
}

auto Source::extract_key(
    const protobuf::Credential& credential,
    const protobuf::KeyRole role) -> std::unique_ptr<protobuf::AsymmetricKey>
{
    std::unique_ptr<protobuf::AsymmetricKey> output;

    const bool master = (protobuf::CREDROLE_MASTERKEY == credential.role());
    const bool child = (protobuf::CREDROLE_CHILDKEY == credential.role());
    const bool keyCredential = master || child;

    if (!keyCredential) { return output; }

    const auto& publicCred = credential.publiccredential();

    for (const auto& key : publicCred.key()) {
        if (role == key.role()) {
            output = std::make_unique<protobuf::AsymmetricKey>(key);

            break;
        }
    }

    return output;
}

auto Source::NymID() const noexcept -> identifier::Nym
{
    switch (type_) {
        case identity::SourceType::PubKey: {

            return factory_.NymIDFromPreimage(asData().Bytes());
        }
        case identity::SourceType::Bip47: {

            return payment_code_.ID();
        }
        case identity::SourceType::Error:
        default: {

            return {};
        }
    }
}

auto Source::Serialize(protobuf::NymIDSource& source) const noexcept -> bool
{
    source.set_version(version_);
    source.set_type(translate(type_));

    switch (type_) {
        case identity::SourceType::PubKey: {
            assert_true(pubkey_.IsValid());

            auto key = protobuf::AsymmetricKey{};
            if (false == pubkey_.Internal().Serialize(key)) { return false; }
            key.set_role(protobuf::KEYROLE_SIGN);
            *(source.mutable_key()) = key;

        } break;
        case identity::SourceType::Bip47: {
            if (false == payment_code_.Internal().Serialize(
                             *(source.mutable_paymentcode()))) {
                return false;
            }

        } break;
        case identity::SourceType::Error:
        default: {
        }
    }

    return true;
}

auto Source::sourcetype_map() noexcept -> const SourceTypeMap&
{
    using enum identity::SourceType;
    using enum protobuf::SourceType;
    static constexpr auto map = SourceTypeMap{
        {Error, SOURCETYPE_ERROR},
        {PubKey, SOURCETYPE_PUBKEY},
        {Bip47, SOURCETYPE_BIP47},
    };

    return map;
}
auto Source::translate(const identity::SourceType in) noexcept
    -> protobuf::SourceType
{
    try {
        return sourcetype_map().at(in);
    } catch (...) {
        return protobuf::SOURCETYPE_ERROR;
    }
}

auto Source::translate(const protobuf::SourceType in) noexcept
    -> identity::SourceType
{
    static const auto map = frozen::invert_unordered_map(sourcetype_map());

    try {
        return map.at(in);
    } catch (...) {
        return identity::SourceType::Error;
    }
}

// This function assumes that all internal verification checks are complete
// except for the source proof
auto Source::Verify(
    const protobuf::Credential& master,
    [[maybe_unused]] const protobuf::Signature& sourceSignature) const noexcept
    -> bool
{
    bool isSelfSigned, sameSource;
    std::unique_ptr<protobuf::AsymmetricKey> signingKey;

    switch (type_) {
        case identity::SourceType::PubKey: {
            if (false == pubkey_.IsValid()) { return false; }

            isSelfSigned =
                (protobuf::SOURCEPROOFTYPE_SELF_SIGNATURE ==
                 master.masterdata().sourceproof().type());

            if (!isSelfSigned) {
                assert_true(false, "Not yet implemented");

                return false;
            }

            signingKey = extract_key(master, protobuf::KEYROLE_SIGN);

            if (!signingKey) {
                LogError()()("Failed to extract signing key.").Flush();

                return false;
            }

            auto sourceKey = protobuf::AsymmetricKey{};
            if (false == pubkey_.Internal().Serialize(sourceKey)) {
                LogError()()("Failed to serialize key").Flush();

                return false;
            }
            sameSource = (sourceKey.key() == signingKey->key());

            if (!sameSource) {
                LogError()()("Master credential was not"
                             " derived from this source.")
                    .Flush();

                return false;
            }
        } break;
        case identity::SourceType::Bip47: {
            if (!payment_code_.Internal().Verify(master, sourceSignature)) {
                LogError()()("Invalid source signature.").Flush();

                return false;
            }
        } break;
        case identity::SourceType::Error:
        default: {
            return false;
        }
    }

    return true;
}  // namespace opentxs::identity::implementation

auto Source::Sign(
    [[maybe_unused]] const identity::credential::Primary& credential,
    [[maybe_unused]] protobuf::Signature& sig,
    [[maybe_unused]] const PasswordPrompt& reason) const noexcept -> bool
{
    bool goodsig = false;

    switch (type_) {
        case identity::SourceType::PubKey: {
            assert_true(false, "This is not implemented yet.");

        } break;
        case identity::SourceType::Bip47: {
            goodsig = payment_code_.Internal().Sign(credential, sig, reason);
        } break;
        case identity::SourceType::Error:
        default: {
        }
    }

    return goodsig;
}

auto Source::asString() const noexcept -> OTString
{
    const auto armored = factory_.Internal().Armored(asData());
    auto out = String::Factory();
    out->Set(armored);

    return out;
}

auto Source::Description() const noexcept -> OTString
{
    auto description = String::Factory();
    auto keyID = identifier::Generic{};

    switch (type_) {
        case identity::SourceType::PubKey: {
            if (pubkey_.IsValid()) {
                pubkey_.Internal().CalculateID(keyID);
                description = String::Factory(keyID, crypto_);
            }
        } break;
        case identity::SourceType::Bip47: {
            description = String::Factory(payment_code_.asBase58());

        } break;
        case identity::SourceType::Error:
        default: {
        }
    }

    return description;
}
}  // namespace opentxs::identity::implementation
