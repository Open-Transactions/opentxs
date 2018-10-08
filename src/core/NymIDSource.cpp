// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/NymIDSource.hpp"

#include "opentxs/api/Factory.hpp"
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/MasterCredential.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
#include "opentxs/core/crypto/PaymentCode.hpp"
#endif
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <memory>
#include <ostream>

namespace opentxs
{

NymIDSource::NymIDSource(
    const api::Factory& factory,
    const proto::NymIDSource& serializedSource)
    : factory_{factory}
    , version_(serializedSource.version())
    , type_(serializedSource.type())
    , pubkey_(crypto::key::Asymmetric::Factory())
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    , payment_code_(factory_.PaymentCode(""))
#endif
{
    switch (type_) {
        case proto::SOURCETYPE_PUBKEY: {
            pubkey_ = crypto::key::Asymmetric::Factory(serializedSource.key());

            break;
        }
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case proto::SOURCETYPE_BIP47: {
            payment_code_ =
                factory_.PaymentCode(serializedSource.paymentcode());

            break;
        }
#endif
        default: {
        }
    }
}

NymIDSource::NymIDSource(const api::Factory& factory, const String& source)
    : NymIDSource(factory, *ExtractArmoredSource(Armored::Factory(source)))
{
}

NymIDSource::NymIDSource(
    const api::Factory& factory,
    const NymParameters& nymParameters,
    proto::AsymmetricKey& pubkey)
    : factory_{factory}
    , version_(1)
    , type_(nymParameters.SourceType())
    , pubkey_(crypto::key::Asymmetric::Factory(pubkey))
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    , payment_code_(factory_.PaymentCode(""))
#endif

{
    OT_ASSERT(pubkey_.get());
}

#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
NymIDSource::NymIDSource(const api::Factory& factory, const PaymentCode& source)
    : factory_{factory}
    , version_(1)
    , type_(proto::SOURCETYPE_BIP47)
    , pubkey_(crypto::key::Asymmetric::Factory())
    , payment_code_{source}
{
}
#endif

NymIDSource::NymIDSource(const NymIDSource& rhs)
    : NymIDSource(rhs.factory_, *rhs.Serialize())
{
}

OTData NymIDSource::asData() const
{
    serializedNymIDSource serializedSource = Serialize();

    return proto::ProtoAsData(*serializedSource);
}

std::unique_ptr<proto::AsymmetricKey> NymIDSource::ExtractKey(
    const proto::Credential& credential,
    const proto::KeyRole role)
{
    std::unique_ptr<proto::AsymmetricKey> output;

    const bool master = (proto::CREDROLE_MASTERKEY == credential.role());
    const bool child = (proto::CREDROLE_CHILDKEY == credential.role());
    const bool keyCredential = master || child;

    if (!keyCredential) { return output; }

    const auto& publicCred = credential.publiccredential();

    for (auto& key : publicCred.key()) {
        if (role == key.role()) {
            output.reset(new proto::AsymmetricKey(key));

            break;
        }
    }

    return output;
}

OTIdentifier NymIDSource::NymID() const
{
    auto nymID = Identifier::Factory();
    auto dataVersion = Data::Factory();

    switch (type_) {
        case proto::SOURCETYPE_PUBKEY:
            dataVersion = asData();
            nymID->CalculateDigest(dataVersion);

            break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case proto::SOURCETYPE_BIP47:
            nymID = payment_code_->ID();

            break;
#endif
        default:
            break;
    }

    return nymID;
}

serializedNymIDSource NymIDSource::Serialize() const
{
    serializedNymIDSource source = std::make_shared<proto::NymIDSource>();
    source->set_version(version_);
    source->set_type(type_);

    std::shared_ptr<proto::AsymmetricKey> key;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    SerializedPaymentCode paycode;
#endif

    switch (type_) {
        case proto::SOURCETYPE_PUBKEY:
            OT_ASSERT(pubkey_.get())

            key = pubkey_->Serialize();
            key->set_role(proto::KEYROLE_SIGN);
            *(source->mutable_key()) = *key;

            break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case proto::SOURCETYPE_BIP47:
            paycode = payment_code_->Serialize();
            *(source->mutable_paymentcode()) = *paycode;

            break;
#endif
        default:
            break;
    }

    return source;
}

// This function assumes that all internal verification checks are complete
// except for the source proof
bool NymIDSource::Verify(
    const proto::Credential& master,
    __attribute__((unused)) const proto::Signature& sourceSignature) const
{
    serializedCredential serializedMaster;
    bool isSelfSigned, sameSource;
    std::unique_ptr<proto::AsymmetricKey> signingKey;
    std::shared_ptr<proto::AsymmetricKey> sourceKey;

    switch (type_) {
        case proto::SOURCETYPE_PUBKEY:
            if (!pubkey_.get()) { return false; }

            isSelfSigned =
                (proto::SOURCEPROOFTYPE_SELF_SIGNATURE ==
                 serializedMaster->masterdata().sourceproof().type());

            if (!isSelfSigned) {
                OT_ASSERT_MSG(false, "Not yet implemented");

                return false;
            }

            signingKey = ExtractKey(*serializedMaster, proto::KEYROLE_SIGN);

            if (!signingKey) {
                otErr << __FUNCTION__ << ": Failed to extract signing key"
                      << std::endl;

                return false;
            }

            sourceKey = pubkey_->Serialize();
            sameSource = (sourceKey->key() == signingKey->key());

            if (!sameSource) {
                otErr << __FUNCTION__ << ": Master credential was not"
                      << " derived from this source." << std::endl;

                return false;
            }

            break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case proto::SOURCETYPE_BIP47:
            if (!payment_code_->Verify(master, sourceSignature)) {
                otErr << __FUNCTION__ << ": Invalid source signature."
                      << std::endl;

                return false;
            }

            break;
#endif
        default:
            break;
    }

    return true;
}

bool NymIDSource::Sign(
    __attribute__((unused)) const MasterCredential& credential,
    __attribute__((unused)) proto::Signature& sig,
    __attribute__((unused)) const OTPasswordData* pPWData) const
{
    bool goodsig = false;

    switch (type_) {
        case (proto::SOURCETYPE_PUBKEY):
            OT_ASSERT_MSG(false, "This is not implemented yet.");

            break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case (proto::SOURCETYPE_BIP47):
            goodsig = payment_code_->Sign(credential, sig, pPWData);

            break;
#endif
        default:
            break;
    }

    return goodsig;
}

OTString NymIDSource::asString() const
{
    auto dataSource = asData();
    auto armoredSource = Armored::Factory(dataSource);

    return String::Factory(armoredSource->Get());
}

// static
serializedNymIDSource NymIDSource::ExtractArmoredSource(
    const Armored& armoredSource)
{
    auto dataSource = Data::Factory(armoredSource);

    OT_ASSERT(dataSource->size() > 0);

    auto protoSource = std::make_shared<proto::NymIDSource>();
    protoSource->ParseFromArray(dataSource->data(), dataSource->size());

    return protoSource;
}

OTString NymIDSource::Description() const
{
    auto description = String::Factory();
    auto keyID = Identifier::Factory();

    switch (type_) {
        case (proto::SOURCETYPE_PUBKEY):
            if (pubkey_.get()) {
                pubkey_->CalculateID(keyID);
                description = String::Factory(keyID);
            }

            break;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
        case (proto::SOURCETYPE_BIP47):
            description = String::Factory(payment_code_->asBase58());

            break;
#endif
        default:
            break;
    }

    return description;
}

proto::SourceType NymIDSource::Type() const { return type_; }
}  // namespace opentxs
