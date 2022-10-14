// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "2_Factory.hpp"        // IWYU pragma: associated
#include "identity/Source.hpp"  // IWYU pragma: associated

#include <AsymmetricKey.pb.h>
#include <Credential.pb.h>
#include <Enums.pb.h>
#include <KeyCredential.pb.h>
#include <MasterCredentialParameters.pb.h>
#include <NymIDSource.pb.h>
#include <SourceProof.pb.h>
#include <memory>
#include <stdexcept>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/core/Armored.hpp"
#include "internal/core/PaymentCode.hpp"
#include "internal/core/String.hpp"
#include "internal/crypto/Parameters.hpp"
#include "internal/crypto/key/Keypair.hpp"
#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/identity/CredentialType.hpp"
#include "opentxs/identity/SourceType.hpp"
#include "opentxs/identity/credential/Primary.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/Container.hpp"

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
                              crypto::key::asymmetric::Algorithm::Secp256k1))) {
                throw std::runtime_error("Missing BIP-47 support");
            }

            const auto paymentCode = api.Factory().PaymentCode(
                params.Seed(),
                params.Nym(),
                params.PaymentCodeVersion(),
                reason);

            return new ReturnType{api.Factory(), paymentCode};
        }
        case identity::SourceType::PubKey:
            switch (params.credentialType()) {
                case identity::CredentialType::Legacy: {
                    params.Internal().Keypair() =
                        api.Factory().InternalSession().Keypair(
                            params,
                            crypto::key::Asymmetric::DefaultVersion,
                            opentxs::crypto::key::asymmetric::Role::Sign,
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
                        api.Factory().InternalSession().Keypair(
                            params.Seed(),
                            params.Nym(),
                            params.Credset(),
                            params.CredIndex(),
                            curve,
                            opentxs::crypto::key::asymmetric::Role::Sign,
                            reason);
                } break;
                case identity::CredentialType::Error:
                default: {
                    throw std::runtime_error("Unsupported credential type");
                }
            }

            if (false == bool(params.Internal().Keypair().get())) {
                LogError()("opentxs::Factory::")(__func__)(
                    ": Failed to generate signing keypair")
                    .Flush();

                return nullptr;
            }

            return new ReturnType{api.Factory(), params};
        case identity::SourceType::Error:
        default: {
            LogError()("opentxs::Factory::")(__func__)(
                ": Unsupported source type.")
                .Flush();

            return nullptr;
        }
    }
}

auto Factory::NymIDSource(
    const api::Session& api,
    const proto::NymIDSource& serialized) -> identity::Source*
{
    using ReturnType = identity::implementation::Source;

    return new ReturnType{api.Factory(), serialized};
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
    const api::session::Factory& factory,
    const proto::NymIDSource& serialized) noexcept
    : factory_(factory)
    , type_(translate(serialized.type()))
    , pubkey_(deserialize_pubkey(factory, type_, serialized))
    , payment_code_(deserialize_paymentcode(factory, type_, serialized))
    , version_(serialized.version())
{
}

Source::Source(
    const api::session::Factory& factory,
    const crypto::Parameters& nymParameters) noexcept(false)
    : factory_{factory}
    , type_(nymParameters.SourceType())
    , pubkey_(nymParameters.Internal().Keypair().GetPublicKey())
    , payment_code_(factory_.PaymentCode(UnallocatedCString{}))
    , version_(key_to_source_version_.at(pubkey_->Version()))

{
    if (false == bool(pubkey_.get())) {
        throw std::runtime_error("Invalid pubkey");
    }
}

Source::Source(
    const api::session::Factory& factory,
    const PaymentCode& source) noexcept
    : factory_{factory}
    , type_(identity::SourceType::Bip47)
    , pubkey_(crypto::key::Asymmetric::Factory())
    , payment_code_{source}
    , version_(key_to_source_version_.at(payment_code_.Version()))
{
}

Source::Source(const Source& rhs) noexcept
    : Source(rhs.factory_, [&](const Source& rhs) -> proto::NymIDSource {
        auto serialized = proto::NymIDSource{};
        rhs.Serialize(serialized);
        return serialized;
    }(rhs))
{
}

auto Source::asData() const -> ByteArray
{
    auto serialized = proto::NymIDSource{};
    if (false == Serialize(serialized)) { return ByteArray{nullptr}; }

    return factory_.InternalSession().Data(serialized);
}

auto Source::deserialize_paymentcode(
    const api::session::Factory& factory,
    const identity::SourceType type,
    const proto::NymIDSource& serialized) -> PaymentCode
{
    if (identity::SourceType::Bip47 == type) {

        return factory.InternalSession().PaymentCode(serialized.paymentcode());
    } else {

        return factory.PaymentCode(UnallocatedCString{});
    }
}

auto Source::deserialize_pubkey(
    const api::session::Factory& factory,
    const identity::SourceType type,
    const proto::NymIDSource& serialized) -> OTAsymmetricKey
{
    if (identity::SourceType::PubKey == type) {

        return factory.InternalSession().AsymmetricKey(serialized.key());
    } else {

        return crypto::key::Asymmetric::Factory();
    }
}

auto Source::extract_key(
    const proto::Credential& credential,
    const proto::KeyRole role) -> std::unique_ptr<proto::AsymmetricKey>
{
    std::unique_ptr<proto::AsymmetricKey> output;

    const bool master = (proto::CREDROLE_MASTERKEY == credential.role());
    const bool child = (proto::CREDROLE_CHILDKEY == credential.role());
    const bool keyCredential = master || child;

    if (!keyCredential) { return output; }

    const auto& publicCred = credential.publiccredential();

    for (const auto& key : publicCred.key()) {
        if (role == key.role()) {
            output = std::make_unique<proto::AsymmetricKey>(key);

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

auto Source::Serialize(proto::NymIDSource& source) const noexcept -> bool
{
    source.set_version(version_);
    source.set_type(translate(type_));

    switch (type_) {
        case identity::SourceType::PubKey: {
            OT_ASSERT(pubkey_.get());

            auto key = proto::AsymmetricKey{};
            if (false == pubkey_->Serialize(key)) { return false; }
            key.set_role(proto::KEYROLE_SIGN);
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
    static const auto map = SourceTypeMap{
        {identity::SourceType::Error, proto::SOURCETYPE_ERROR},
        {identity::SourceType::PubKey, proto::SOURCETYPE_PUBKEY},
        {identity::SourceType::Bip47, proto::SOURCETYPE_BIP47},
    };

    return map;
}
auto Source::translate(const identity::SourceType in) noexcept
    -> proto::SourceType
{
    try {
        return sourcetype_map().at(in);
    } catch (...) {
        return proto::SOURCETYPE_ERROR;
    }
}

auto Source::translate(const proto::SourceType in) noexcept
    -> identity::SourceType
{
    static const auto map = reverse_arbitrary_map<
        identity::SourceType,
        proto::SourceType,
        SourceTypeReverseMap>(sourcetype_map());

    try {
        return map.at(in);
    } catch (...) {
        return identity::SourceType::Error;
    }
}

// This function assumes that all internal verification checks are complete
// except for the source proof
auto Source::Verify(
    const proto::Credential& master,
    [[maybe_unused]] const proto::Signature& sourceSignature) const noexcept
    -> bool
{
    bool isSelfSigned, sameSource;
    std::unique_ptr<proto::AsymmetricKey> signingKey;

    switch (type_) {
        case identity::SourceType::PubKey: {
            if (!pubkey_.get()) { return false; }

            isSelfSigned =
                (proto::SOURCEPROOFTYPE_SELF_SIGNATURE ==
                 master.masterdata().sourceproof().type());

            if (!isSelfSigned) {
                OT_ASSERT_MSG(false, "Not yet implemented");

                return false;
            }

            signingKey = extract_key(master, proto::KEYROLE_SIGN);

            if (!signingKey) {
                LogError()(OT_PRETTY_CLASS())("Failed to extract signing key.")
                    .Flush();

                return false;
            }

            auto sourceKey = proto::AsymmetricKey{};
            if (false == pubkey_->Serialize(sourceKey)) {
                LogError()(OT_PRETTY_CLASS())("Failed to serialize key")
                    .Flush();

                return false;
            }
            sameSource = (sourceKey.key() == signingKey->key());

            if (!sameSource) {
                LogError()(OT_PRETTY_CLASS())("Master credential was not"
                                              " derived from this source.")
                    .Flush();

                return false;
            }
        } break;
        case identity::SourceType::Bip47: {
            if (!payment_code_.Internal().Verify(master, sourceSignature)) {
                LogError()(OT_PRETTY_CLASS())("Invalid source signature.")
                    .Flush();

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
    [[maybe_unused]] proto::Signature& sig,
    [[maybe_unused]] const PasswordPrompt& reason) const noexcept -> bool
{
    bool goodsig = false;

    switch (type_) {
        case identity::SourceType::PubKey: {
            OT_ASSERT_MSG(false, "This is not implemented yet.");

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
    return {factory_.InternalSession().Armored(asData())};
}

auto Source::Description() const noexcept -> OTString
{
    auto description = String::Factory();
    auto keyID = identifier::Generic{};

    switch (type_) {
        case identity::SourceType::PubKey: {
            if (pubkey_.get()) {
                pubkey_->CalculateID(keyID);
                description = String::Factory(keyID);
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
