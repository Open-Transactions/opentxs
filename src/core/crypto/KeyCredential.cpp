/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

// A nym contains a list of credential sets.
// The whole purpose of a Nym is to be an identity, which can have
// master credentials.
//
// Each CredentialSet contains list of Credentials. One of the
// Credentials is a MasterCredential, and the rest are ChildCredentials
// signed by the MasterCredential.
//
// A Credential may contain keys, in which case it is a KeyCredential.
//
// Credentials without keys might be an interface to a hardware device
// or other kind of external encryption and authentication system.
//
// Non-key Credentials are not yet implemented.
//
// Each KeyCredential has 3 OTKeypairs: encryption, signing, and authentication.
// Each OTKeypair has 2 OTAsymmetricKeys (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials

#include "opentxs/core/crypto/KeyCredential.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/core/OTData.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/app/App.hpp"
#include "opentxs/core/contract/Signable.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_HD
#include "opentxs/core/crypto/Bip32.hpp"
#include "opentxs/core/crypto/Bip39.hpp"
#endif
#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/Ecdsa.hpp"
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/core/crypto/Libsecp256k1.hpp"
#endif
#include "opentxs/core/crypto/Libsodium.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTKeypair.hpp"
#include "opentxs/core/crypto/OTSignature.hpp"
#include "opentxs/core/crypto/OTSignatureMetadata.hpp"
#include "opentxs/core/util/Assert.hpp"

#include <stdint.h>
#include <cstdint>
#include <memory>
#include <ostream>

namespace opentxs
{

bool KeyCredential::VerifySignedBySelf() const
{
    OT_ASSERT(m_SigningKey);

    auto publicSig = SelfSignature(PUBLIC_VERSION);

    if (!publicSig) {
        otErr << __FUNCTION__ << ": Could not find public self signature."
              << std::endl;

        return false;
    }

    bool goodPublic = VerifySig(*publicSig, PUBLIC_VERSION);

    if (!goodPublic) {
        otErr << __FUNCTION__ << ": Could not verify public self signature."
              << std::endl;

        return false;
    }

    if (Private()) {
        auto privateSig = SelfSignature(PRIVATE_VERSION);

        if (!privateSig) {
            otErr << __FUNCTION__
                  << ": Could not find private self signature." << std::endl;

            return false;
        }

        bool goodPrivate = VerifySig(*privateSig, PRIVATE_VERSION);

        if (!goodPrivate) {
            otErr << __FUNCTION__
                  << ": Could not verify private self signature." << std::endl;

            return false;
        }
    }

    return true;
}

// NOTE: You might ask, if we are using theSignature's metadata to narrow down
// the key type,
// then why are we still passing the key type as a separate parameter? Good
// question. Because
// often, theSignature will have no metadata at all! In that case, normally we
// would just NOT
// return any keys, period. Because we assume, if a key credential signed it,
// then it WILL have
// metadata, and if it doesn't have metadata, then a key credential did NOT sign
// it, and therefore
// we know from the get-go that none of the keys from the key credentials will
// work to verify it,
// either. That's why, normally, we don't return any keys if theSignature has no
// metadata.
// BUT...Let's say you know this, that the signature has no metadata, yet you
// also still believe
// it may be signed with one of these keys. Further, while you don't know
// exactly which key it
// actually is, let's say you DO know by context that it's a signing key, or an
// authentication key,
// or an encryption key. So you specify that. In which case, OT should return
// all possible matching
// pubkeys based on that 1-letter criteria, instead of its normal behavior,
// which is to return all
// possible matching pubkeys based on a full match of the metadata.
//
int32_t KeyCredential::GetPublicKeysBySignature(
    listOfAsymmetricKeys& listOutput,
    const OTSignature& theSignature,
    char cKeyType) const  // 'S' (signing key) or 'E' (encryption key)
                          // or 'A' (authentication key)
{
    // Key type was not specified, because we only want keys that match the
    // metadata on theSignature.
    // And if theSignature has no metadata, then we want to return 0 keys.
    if (('0' == cKeyType) && !theSignature.getMetaData().HasMetadata())
        return 0;

    // By this point, we know that EITHER exact metadata matches must occur, and
    // the signature DOES have metadata, ('0')
    // OR the search is only for 'A', 'E', or 'S' candidates, based on cKeyType,
    // and that the signature's metadata
    // can additionally narrow the search down, if it's present, which in this
    // case it's not guaranteed to be.
    int32_t nCount = 0;

    OT_ASSERT(m_AuthentKey);
    OT_ASSERT(m_EncryptKey);
    OT_ASSERT(m_SigningKey);

    switch (cKeyType) {
        // Specific search only for signatures with metadata.
        // FYI, theSignature.getMetaData().HasMetadata() is true, in this case.
        case '0': {
            // That's why I can just assume theSignature has a key type here:
            switch (theSignature.getMetaData().GetKeyType()) {
                case 'A':
                    nCount = m_AuthentKey->GetPublicKeyBySignature(
                        listOutput, theSignature);
                    break;  // bInclusive=false by default
                case 'E':
                    nCount = m_EncryptKey->GetPublicKeyBySignature(
                        listOutput, theSignature);
                    break;  // bInclusive=false by default
                case 'S':
                    nCount = m_SigningKey->GetPublicKeyBySignature(
                        listOutput, theSignature);
                    break;  // bInclusive=false by default
                default:
                    otErr
                        << __FUNCTION__
                        << ": Unexpected keytype value in signature metadata: "
                        << theSignature.getMetaData().GetKeyType()
                        << " (failure)\n";
                    return 0;
            }
            break;
        }
        // Generalized search which specifies key type and returns keys
        // even for signatures with no metadata. (When metadata is present,
        // it's still used to eliminate keys.)
        case 'A':
            nCount = m_AuthentKey->GetPublicKeyBySignature(
                listOutput, theSignature, true);
            break;  // bInclusive=true
        case 'E':
            nCount = m_EncryptKey->GetPublicKeyBySignature(
                listOutput, theSignature, true);
            break;  // bInclusive=true
        case 'S':
            nCount = m_SigningKey->GetPublicKeyBySignature(
                listOutput, theSignature, true);
            break;  // bInclusive=true
        default:
            otErr
                << __FUNCTION__
                << ": Unexpected value for cKeyType (should be 0, A, E, or S): "
                << cKeyType << "\n";
            return 0;
    }
    return nCount;
}

bool KeyCredential::VerifyInternally() const
{
    // Perform common Credential verifications
    if (!ot_super::VerifyInternally()) {
        return false;
    }

    // All KeyCredentials must sign themselves
    if (!VerifySignedBySelf()) {
        otOut << __FUNCTION__ << ": Failed verifying key credential: it's not "
                                 "signed by itself (its own signing key.)\n";
        return false;
    }

    return true;
}

KeyCredential::KeyCredential(
    CredentialSet& theOwner,
    const proto::Credential& serializedCred)
    : ot_super(theOwner, serializedCred)
{
    const bool hasPrivate =
        (proto::KEYMODE_PRIVATE == serializedCred.mode()) ? true : false;

    // Auth key
    proto::AsymmetricKey publicAuth =
        serializedCred.publiccredential().key(proto::KEYROLE_AUTH - 1);

    if (hasPrivate) {
        proto::AsymmetricKey privateAuth =
            serializedCred.privatecredential().key(proto::KEYROLE_AUTH - 1);

        m_AuthentKey = std::make_shared<OTKeypair>(publicAuth, privateAuth);
    } else {
        m_AuthentKey = std::make_shared<OTKeypair>(publicAuth);
    }

    // Encrypt key
    proto::AsymmetricKey publicEncrypt =
        serializedCred.publiccredential().key(proto::KEYROLE_ENCRYPT - 1);

    if (hasPrivate) {
        proto::AsymmetricKey privateEncrypt =
            serializedCred.privatecredential().key(proto::KEYROLE_ENCRYPT - 1);

        m_EncryptKey =
            std::make_shared<OTKeypair>(publicEncrypt, privateEncrypt);
    } else {
        m_EncryptKey = std::make_shared<OTKeypair>(publicEncrypt);
    }

    // Sign key
    proto::AsymmetricKey publicSign =
        serializedCred.publiccredential().key(proto::KEYROLE_SIGN - 1);

    if (hasPrivate) {
        proto::AsymmetricKey privateSign =
            serializedCred.privatecredential().key(proto::KEYROLE_SIGN - 1);

        m_SigningKey = std::make_shared<OTKeypair>(publicSign, privateSign);
    } else {
        m_SigningKey = std::make_shared<OTKeypair>(publicSign);
    }
}

KeyCredential::KeyCredential(
    CredentialSet& theOwner,
    const NymParameters& nymParameters)
        : ot_super(theOwner, nymParameters)
{
    Log::vOutput(0, "KeyCredential::KeyCredential: credential set, nym params");

    
    if (proto::CREDTYPE_HD != nymParameters.credentialType()) {
        
        Log::vOutput(0, "KeyCredential::KeyCredential: 1");

        
        m_AuthentKey =
            std::make_shared<OTKeypair>(nymParameters, proto::KEYROLE_AUTH);
        m_EncryptKey =
            std::make_shared<OTKeypair>(nymParameters, proto::KEYROLE_ENCRYPT);
        m_SigningKey =
            std::make_shared<OTKeypair>(nymParameters, proto::KEYROLE_SIGN);
    } else {
        
        Log::vOutput(0, "KeyCredential::KeyCredential: 2");

        
#if OT_CRYPTO_SUPPORTED_KEY_HD
        
        Log::vOutput(0, "KeyCredential::KeyCredential: 3");

        const auto keyType = nymParameters.AsymmetricKeyType();
        const auto curve = CryptoAsymmetric::KeyTypeToCurve(keyType);

        if (EcdsaCurve::ERROR != curve) {
            
            Log::vOutput(0, "KeyCredential::KeyCredential: 4");

            
            m_AuthentKey = DeriveHDKeypair(
                nymParameters.Seed(),
                nymParameters.Nym(),
                nymParameters.Credset(),
                nymParameters.CredIndex(),
                curve,
                proto::KEYROLE_AUTH);
            
            Log::vOutput(0, "KeyCredential::KeyCredential: 5");

            
            m_EncryptKey = DeriveHDKeypair(
                nymParameters.Seed(),
                nymParameters.Nym(),
                nymParameters.Credset(),
                nymParameters.CredIndex(),
                curve,
                proto::KEYROLE_ENCRYPT);
            
            Log::vOutput(0, "KeyCredential::KeyCredential: 6");

            m_SigningKey = DeriveHDKeypair(
                nymParameters.Seed(),
                nymParameters.Nym(),
                nymParameters.Credset(),
                nymParameters.CredIndex(),
                curve,
                proto::KEYROLE_SIGN);
            
            
            Log::vOutput(0, "KeyCredential::KeyCredential: 7");

        }
        
        Log::vOutput(0, "KeyCredential::KeyCredential: 8");

#endif
        
        Log::vOutput(0, "KeyCredential::KeyCredential: 9");

    }
    
    Log::vOutput(0, "KeyCredential::KeyCredential: 10");

}

bool KeyCredential::New(
    const NymParameters& nymParameters)
{
    Log::vOutput(0, "KeyCredential::New: 1");

    CalculateID();

    Log::vOutput(0, "KeyCredential::New: 2");

    if (SelfSign()) {
        
        Log::vOutput(0, "KeyCredential::New: 3");

        return ot_super::New(nymParameters);
    }
    Log::vOutput(0, "KeyCredential::New: 4");

    return false;
}

#if OT_CRYPTO_SUPPORTED_KEY_HD
std::shared_ptr<OTKeypair> KeyCredential::DeriveHDKeypair(
    const std::string& fingerprint,
    const uint32_t nym,
    const uint32_t credset,
    const uint32_t credindex,
    const EcdsaCurve& curve,
    const proto::KeyRole role)
{
    
    Log::vOutput(0, "KeyCredential::DeriveHDKeypair: 1");

    
    proto::HDPath keyPath;
    keyPath.set_version(1);

    if (!fingerprint.empty()) {
        
        Log::vOutput(0, "KeyCredential::DeriveHDKeypair: 2");

        // Check to see if specified seed exists
        //TODO: make fingerprint non-const
        std::string input (fingerprint);
        auto seed = App::Me().Crypto().BIP39().Seed(input);

        if (seed) {
            
            Log::vOutput(0, "KeyCredential::DeriveHDKeypair: 3");

            
            keyPath.set_root(fingerprint.c_str(), fingerprint.size());
            otErr << __FUNCTION__ << ": Using seed " << fingerprint <<  " for "
                  << "key derivation." << std::endl;
        } else {
            otLog5 << __FUNCTION__ << ": Using default seed for key derivation."
                  << std::endl;
        }
        
        Log::vOutput(0, "KeyCredential::DeriveHDKeypair: 4");

    } else {
        otLog5 << __FUNCTION__ << ": Using default seed for key derivation."
                << std::endl;
    }

    Log::vOutput(0, "KeyCredential::DeriveHDKeypair: 5");

    keyPath.add_child(
        static_cast<std::uint32_t>(Bip43Purpose::NYM) |
        static_cast<std::uint32_t>(Bip32Child::HARDENED));
    keyPath.add_child(
        nym |
        static_cast<std::uint32_t>(Bip32Child::HARDENED));
    keyPath.add_child(
        credset |
        static_cast<std::uint32_t>(Bip32Child::HARDENED));
    keyPath.add_child(
        credindex |
        static_cast<std::uint32_t>(Bip32Child::HARDENED));

    switch (role) {
        case proto::KEYROLE_AUTH:
            keyPath.add_child(
                static_cast<std::uint32_t>(Bip32Child::AUTH_KEY) |
                static_cast<std::uint32_t>(Bip32Child::HARDENED));
            break;
        case proto::KEYROLE_ENCRYPT:
            keyPath.add_child(
                static_cast<std::uint32_t>(Bip32Child::ENCRYPT_KEY) |
                static_cast<std::uint32_t>(Bip32Child::HARDENED));
            break;
        case proto::KEYROLE_SIGN:
            keyPath.add_child(
                static_cast<std::uint32_t>(Bip32Child::SIGN_KEY) |
                static_cast<std::uint32_t>(Bip32Child::HARDENED));
            break;
        default:
            break;
    }

    std::shared_ptr<OTKeypair> newKeypair;
    auto privateKey = App::Me().Crypto().BIP32().GetHDKey(curve, keyPath);

    if (!privateKey) { return newKeypair; }

    privateKey->set_role(role);
    Ecdsa* engine = nullptr;

    switch (curve) {
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (EcdsaCurve::SECP256K1) : {
            engine =
                static_cast<Libsecp256k1*>(&App::Me().Crypto().SECP256K1());
            break;
        }
#endif
        case (EcdsaCurve::ED25519) : {
            engine = static_cast<Libsodium*>(&App::Me().Crypto().ED25519());
            break;
        }
        default : {}
    }

    if (nullptr == engine) { return newKeypair; }

    proto::AsymmetricKey publicKey;
    const bool haveKey = engine->PrivateToPublic(*privateKey, publicKey);

    if (haveKey) {
        newKeypair = std::make_shared<OTKeypair>(publicKey, *privateKey);
    }

    return newKeypair;
}
#endif

KeyCredential::~KeyCredential() {}

bool KeyCredential::ReEncryptKeys(
    const OTPassword& theExportPassword,
    bool bImporting)
{
    OT_ASSERT(m_AuthentKey);
    OT_ASSERT(m_EncryptKey);
    OT_ASSERT(m_SigningKey);

    const bool bSign = m_SigningKey->ReEncrypt(theExportPassword, bImporting);
    const bool bAuth = m_AuthentKey->ReEncrypt(theExportPassword, bImporting);
    const bool bEncr = m_EncryptKey->ReEncrypt(theExportPassword, bImporting);

    const bool bSuccessReEncrypting = (bSign && bAuth && bEncr);
    OT_ASSERT(bSuccessReEncrypting);

    return bSuccessReEncrypting;  // Note: Caller must re-sign credential after
                                  // doing this,
                                  // to keep these changes.
}

serializedCredential KeyCredential::asSerialized(
    SerializationModeFlag asPrivate,
    SerializationSignatureFlag asSigned) const
{
    serializedCredential serializedCredential =
        this->ot_super::asSerialized(asPrivate, asSigned);

    addKeyCredentialtoSerializedCredential(serializedCredential, false);

    if (asPrivate) {
        addKeyCredentialtoSerializedCredential(serializedCredential, true);
    }

    return serializedCredential;
}

bool KeyCredential::addKeytoSerializedKeyCredential(
    proto::KeyCredential& credential,
    const bool getPrivate,
    const proto::KeyRole role) const
{
    serializedAsymmetricKey key;
    std::shared_ptr<OTKeypair> pKey;

    switch (role) {
        case proto::KEYROLE_AUTH:
            pKey = m_AuthentKey;
            break;
        case proto::KEYROLE_ENCRYPT:
            pKey = m_EncryptKey;
            break;
        case proto::KEYROLE_SIGN:
            pKey = m_SigningKey;
            break;
        default:
            return false;
    }

    if (!pKey) {
        return false;
    }

    key = pKey->Serialize(getPrivate);

    if (!key) {
        return false;
    }

    key->set_role(role);

    auto newKey = credential.add_key();
    *newKey = *key;

    return true;
}

bool KeyCredential::addKeyCredentialtoSerializedCredential(
    serializedCredential credential,
    const bool addPrivate) const
{
    std::unique_ptr<proto::KeyCredential> keyCredential(
        new proto::KeyCredential);

    if (!keyCredential) {
        otErr << __FUNCTION__ << ": failed to allocate keyCredential protobuf."
              << std::endl;

        return false;
    }

    keyCredential->set_version(1);

    // These must be serialized in this order
    bool auth = addKeytoSerializedKeyCredential(
        *keyCredential, addPrivate, proto::KEYROLE_AUTH);
    bool encrypt = addKeytoSerializedKeyCredential(
        *keyCredential, addPrivate, proto::KEYROLE_ENCRYPT);
    bool sign = addKeytoSerializedKeyCredential(
        *keyCredential, addPrivate, proto::KEYROLE_SIGN);

    if (auth && encrypt && sign) {
        if (addPrivate) {
            keyCredential->set_mode(proto::KEYMODE_PRIVATE);
            credential->set_allocated_privatecredential(
                keyCredential.release());

            return true;
        } else {
            keyCredential->set_mode(proto::KEYMODE_PUBLIC);
            credential->set_allocated_publiccredential(keyCredential.release());

            return true;
        }
    }

    return false;
}

bool KeyCredential::Verify(
    const OTData& plaintext,
    const proto::Signature& sig,
    const proto::KeyRole key) const
{
    const OTKeypair* keyToUse = nullptr;

    switch (key) {
        case (proto::KEYROLE_AUTH):
            keyToUse = m_AuthentKey.get();
            break;
        case (proto::KEYROLE_SIGN):
            keyToUse = m_SigningKey.get();
            break;
        default:
            otErr << __FUNCTION__ << ": Can not verify signatures with the "
                  << "specified key.\n";
            return false;
    }

    OT_ASSERT(nullptr != keyToUse);

    return keyToUse->Verify(plaintext, sig);
}

bool KeyCredential::SelfSign(
    __attribute__((unused)) const OTPassword* exportPassword,
    const OTPasswordData* pPWData,
    const bool onlyPrivate)
{
    
    Log::vOutput(0, "KeyCredential::SelfSign: 1");
    

    
    CalculateID();
    
    Log::vOutput(0, "KeyCredential::SelfSign: 2");
    

    SerializedSignature publicSignature = std::make_shared<proto::Signature>();
    SerializedSignature privateSignature = std::make_shared<proto::Signature>();

    
    Log::vOutput(0, "KeyCredential::SelfSign: 3");
    

    bool havePublicSig = false;
    if (!onlyPrivate) {
        
        Log::vOutput(0, "KeyCredential::SelfSign: 4");
        

        const serializedCredential publicVersion =
            asSerialized(AS_PUBLIC, WITHOUT_SIGNATURES);
        
        Log::vOutput(0, "KeyCredential::SelfSign: 5");
        

        auto& signature = *publicVersion->add_signature();
        
        Log::vOutput(0, "KeyCredential::SelfSign: 6");
        

        signature.set_role(proto::SIGROLE_PUBCREDENTIAL);
        
        Log::vOutput(0, "KeyCredential::SelfSign: 7");
        

        havePublicSig = SignProto(
            *publicVersion,
            signature,
            proto::KEYROLE_SIGN,
            pPWData);

        
        Log::vOutput(0, "KeyCredential::SelfSign: 8");
        

        if (havePublicSig) {
            Log::vOutput(0, "KeyCredential::SelfSign: 9");
            

            publicSignature->CopyFrom(signature);
            Log::vOutput(0, "KeyCredential::SelfSign: 10");
            

            signatures_.push_back(publicSignature);
        }
    }

    Log::vOutput(0, "KeyCredential::SelfSign: 11");
    

    serializedCredential privateVersion =
        asSerialized(AS_PRIVATE, WITHOUT_SIGNATURES);
    auto& signature = *privateVersion->add_signature();
    signature.set_role(proto::SIGROLE_PRIVCREDENTIAL);
    
    Log::vOutput(0, "KeyCredential::SelfSign: 12");
    

    bool havePrivateSig = SignProto(
        *privateVersion,
        signature,
        proto::KEYROLE_SIGN,
        pPWData);

    Log::vOutput(0, "KeyCredential::SelfSign: 13");
    

    if (havePrivateSig) {
        privateSignature->CopyFrom(signature);
        signatures_.push_back(privateSignature);
    }

    Log::vOutput(0, "KeyCredential::SelfSign: 14");
    

    return ((havePublicSig | onlyPrivate) && havePrivateSig);
}

bool KeyCredential::VerifySig(
    const proto::Signature& sig,
    const CredentialModeFlag asPrivate) const
{
    serializedCredential serialized;

    if ((proto::KEYMODE_PRIVATE != mode_) && asPrivate) {
        otErr << __FUNCTION__ << ": Can not serialize a public credential "
              << "as a private credential.\n";
        return false;
    }

    if (asPrivate) {
        serialized = asSerialized(
            AS_PRIVATE, WITHOUT_SIGNATURES);
    } else {
        serialized =
            asSerialized(AS_PUBLIC, WITHOUT_SIGNATURES);
    }

    auto& signature = *serialized->add_signature();
    signature.CopyFrom(sig);
    signature.clear_signature();

    OTData plaintext = proto::ProtoAsData<proto::Credential>(*serialized);

    return Verify(plaintext, sig);
}

bool KeyCredential::TransportKey(
    OTData& publicKey,
    OTPassword& privateKey) const
{
    OT_ASSERT(m_AuthentKey);

    return m_AuthentKey->TransportKey(publicKey, privateKey);
}

bool KeyCredential::hasCapability(const NymCapability& capability) const
{
    switch (capability) {
        case (NymCapability::SIGN_MESSAGE) : {
            if (m_SigningKey) {
                return m_SigningKey->hasCapability(capability);
            }

            break;
        }
        case (NymCapability::ENCRYPT_MESSAGE) : {
            if (m_EncryptKey) {
                return m_EncryptKey->hasCapability(capability);
            }

            break;
        }
        case (NymCapability::AUTHENTICATE_CONNECTION) : {
            if (m_AuthentKey) {
                return m_AuthentKey->hasCapability(capability);
            }

            break;
        }
        default : {}
    }

    return false;
}
}  // namespace opentxs
