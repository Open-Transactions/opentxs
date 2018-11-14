// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTCachedKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/crypto/Bip32.hpp"
#endif
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Ed25519.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/crypto/key/RSA.hpp"
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/key/Secp256k1.hpp"
#endif
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"

#include "Null.hpp"

#include <cstdint>
#include <cstring>
#include <memory>
#include <ostream>
#include <string>

#include "Asymmetric.hpp"

template class opentxs::Pimpl<opentxs::crypto::key::Asymmetric>;

#define OT_METHOD "opentxs::crypto::key::implementation::Asymmetric::"

namespace opentxs::crypto::key
{
OTAsymmetricKey Asymmetric::Factory()
{
    return OTAsymmetricKey(new implementation::Null);
}

OTAsymmetricKey Asymmetric::Factory(
    const proto::AsymmetricKeyType keyType,
    const String& pubkey)  // Caller IS responsible to
                           // delete!
{
    switch (keyType) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case (proto::AKEYTYPE_ED25519): {
            return OTAsymmetricKey(Factory::Ed25519Key(pubkey));
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            return OTAsymmetricKey(Factory::Secp256k1Key(pubkey));
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (proto::AKEYTYPE_LEGACY): {
            return OTAsymmetricKey(Factory::RSAKey(pubkey));
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Open-Transactions isn't built with "
                "support for this key type.")
                .Flush();
        }
    }

    return OTAsymmetricKey(new implementation::Null);
}

OTAsymmetricKey Asymmetric::Factory(
    const NymParameters& nymParameters,
    const proto::KeyRole role)  // Caller IS responsible to delete!
{
    const auto keyType = nymParameters.AsymmetricKeyType();

    return OTAsymmetricKey(
        implementation::Asymmetric::KeyFactory(keyType, role));
}

OTAsymmetricKey Asymmetric::Factory(
    const proto::AsymmetricKey& serializedKey)  // Caller IS responsible to
                                                // delete!
{
    const auto keyType = serializedKey.type();

    switch (keyType) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case (proto::AKEYTYPE_ED25519): {
            return OTAsymmetricKey(Factory::Ed25519Key(serializedKey));
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            return OTAsymmetricKey(Factory::Secp256k1Key(serializedKey));
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (proto::AKEYTYPE_LEGACY): {
            return OTAsymmetricKey(Factory::RSAKey(serializedKey));
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Open-Transactions isn't built with "
                "support for this key type.")
                .Flush();
        }
    }

    return OTAsymmetricKey(new implementation::Null);
}

OTString Asymmetric::KeyTypeToString(const proto::AsymmetricKeyType keyType)

{
    auto keytypeString = String::Factory();

    switch (keyType) {
        case proto::AKEYTYPE_LEGACY:
            keytypeString = String::Factory("legacy");
            break;
        case proto::AKEYTYPE_SECP256K1:
            keytypeString = String::Factory("secp256k1");
            break;
        case proto::AKEYTYPE_ED25519:
            keytypeString = String::Factory("ed25519");
            break;
        default:
            keytypeString = String::Factory("error");
    }
    return keytypeString;
}

proto::AsymmetricKeyType Asymmetric::StringToKeyType(const String& keyType)
{
    if (keyType.Compare("legacy")) return proto::AKEYTYPE_LEGACY;
    if (keyType.Compare("secp256k1")) return proto::AKEYTYPE_SECP256K1;
    if (keyType.Compare("ed25519")) return proto::AKEYTYPE_ED25519;

    return proto::AKEYTYPE_ERROR;
}
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
Asymmetric::Asymmetric(
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role,
    const bool publicKey,
    const bool privateKey)
    : m_keyType{keyType}
    , role_{role}
    , m_bIsPublicKey{publicKey}
    , m_bIsPrivateKey{privateKey}
    , m_timer{}
    , m_pMetadata{new OTSignatureMetadata}
{
    OT_ASSERT(nullptr != m_pMetadata);
}

Asymmetric::Asymmetric()
    : Asymmetric(proto::AKEYTYPE_ERROR, proto::KEYROLE_ERROR, false, false)
{
}

Asymmetric::Asymmetric(
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role)
    : Asymmetric(keyType, role, false, false)
{
}

Asymmetric::Asymmetric(const proto::AsymmetricKey& key)
    : Asymmetric(
          key.type(),
          key.role(),
          proto::KEYMODE_PUBLIC == key.mode(),
          proto::KEYMODE_PRIVATE == key.mode())
{
}

Asymmetric::operator bool() const { return m_bIsPublicKey || m_bIsPrivateKey; }

bool Asymmetric::operator==(const proto::AsymmetricKey& rhs) const
{
    std::shared_ptr<proto::AsymmetricKey> tempKey = Serialize();
    auto LHData = SerializeKeyToData(*tempKey);
    auto RHData = SerializeKeyToData(rhs);

    return (LHData == RHData);
}

bool Asymmetric::CalculateID(Identifier& theOutput) const  // Only works
                                                           // for public
                                                           // keys.
{
    theOutput.Release();

    if (!IsPublic()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: !IsPublic() (This function should only be "
            "called on a public key).")
            .Flush();
        return false;
    }

    auto strPublicKey = String::Factory();
    bool bGotPublicKey = GetPublicKey(strPublicKey);

    if (!bGotPublicKey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error getting public key.")
            .Flush();
        return false;
    }

    bool bSuccessCalculateDigest = theOutput.CalculateDigest(strPublicKey);

    if (!bSuccessCalculateDigest) {
        theOutput.Release();
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error calculating digest of public key.")
            .Flush();
        return false;
    }

    return true;
}

Asymmetric* Asymmetric::clone() const
{
    auto output = KeyFactory(m_keyType, role_);

    OT_ASSERT(nullptr != output)

    auto* key = dynamic_cast<Asymmetric*>(output);

    OT_ASSERT(nullptr != key)

    key->m_bIsPublicKey = m_bIsPublicKey;
    key->m_bIsPrivateKey = m_bIsPrivateKey;

    return key;
}

bool Asymmetric::hasCapability(const NymCapability& capability) const
{
    switch (capability) {
        case (NymCapability::SIGN_CHILDCRED): {
        }
        case (NymCapability::SIGN_MESSAGE): {
        }
        case (NymCapability::ENCRYPT_MESSAGE): {

            return true;
        }
        default: {
        }
    }

    return false;
}

// static
key::Asymmetric* Asymmetric::KeyFactory(
    const proto::AsymmetricKeyType keyType,
    const proto::KeyRole role)
{
    key::Asymmetric* pKey = nullptr;

    switch (keyType) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case (proto::AKEYTYPE_ED25519): {
            pKey = Factory::Ed25519Key(role);

            break;
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (proto::AKEYTYPE_SECP256K1): {
            pKey = Factory::Secp256k1Key(role);

            break;
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (proto::AKEYTYPE_LEGACY): {
            pKey = Factory::RSAKey(role);

            break;
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Open-Transactions isn't built with "
                "support for this key type.")
                .Flush();
        }
    }

    return pKey;
}

proto::AsymmetricKeyType Asymmetric::keyType() const { return m_keyType; }

const std::string Asymmetric::Path() const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key type.").Flush();

    return "";
}

bool Asymmetric::Path(proto::HDPath&) const
{
    LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key type.").Flush();

    return false;
}

void Asymmetric::Release()
{
    ReleaseKeyLowLevel_Hook();

    m_timer.clear();
}

std::shared_ptr<proto::AsymmetricKey> Asymmetric::Serialize() const

{
    std::shared_ptr<proto::AsymmetricKey> serializedKey =
        std::make_shared<proto::AsymmetricKey>();

    serializedKey->set_version(1);
    serializedKey->set_role(role_);
    serializedKey->set_type(static_cast<proto::AsymmetricKeyType>(m_keyType));

    return serializedKey;
}

OTData Asymmetric::SerializeKeyToData(
    const proto::AsymmetricKey& serializedKey) const
{
    return proto::ProtoAsData(serializedKey);
}

bool Asymmetric::Sign(
    const Data& plaintext,
    proto::Signature& sig,
    const OTPasswordData* pPWData,
    const OTPassword* exportPassword,
    const String& credID,
    const proto::SignatureRole role) const
{
    if (IsPublic()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": You must use private keys to create signatures.")
            .Flush();
        return false;
    }

    auto signature = Data::Factory();
    const auto hash = SigHashType();

    bool goodSig = engine().Sign(
        plaintext, *this, hash, signature, pPWData, exportPassword);

    if (goodSig) {
        sig.set_version(1);
        if (credID.Exists()) { sig.set_credentialid(credID.Get()); }
        if (proto::SIGROLE_ERROR != role) { sig.set_role(role); }
        sig.set_hashtype(hash);
        sig.set_signature(signature->data(), signature->size());
    }

    return goodSig;
}

bool Asymmetric::Verify(const Data& plaintext, const proto::Signature& sig)
    const
{
    if (IsPrivate()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": You must use public keys to verify signatures.")
            .Flush();
        return false;
    }

    auto signature = Data::Factory();
    signature->Assign(sig.signature().c_str(), sig.signature().size());

    return engine().Verify(
        plaintext, *this, signature, sig.hashtype(), nullptr);
}

Asymmetric::~Asymmetric()
{
    m_timer.clear();

    if (nullptr != m_pMetadata) { delete m_pMetadata; }

    m_pMetadata = nullptr;
}
}  // namespace opentxs::crypto::key::implementation
