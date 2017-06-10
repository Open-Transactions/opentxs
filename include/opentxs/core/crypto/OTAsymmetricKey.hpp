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

#ifndef OPENTXS_CORE_CRYPTO_OTASYMMETRICKEY_HPP
#define OPENTXS_CORE_CRYPTO_OTASYMMETRICKEY_HPP

#include "opentxs/core/crypto/CryptoAsymmetric.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"

#include <stdint.h>
#include <list>
#include <memory>
#include <string>

namespace opentxs
{

class CryptoAsymmetric;
class Identifier;
class NymParameters;
class OTAsymmetricKey;
class OTCaller;
class OTPassword;
class OTPasswordData;
class OTSignatureMetadata;
class String;

typedef std::list<OTAsymmetricKey*> listOfAsymmetricKeys;
typedef std::shared_ptr<proto::AsymmetricKey> serializedAsymmetricKey;

#ifndef OT_KEY_TIMER
// TODO:
// 1. Add this value to the config file so it becomes merely a default value
// here.
// 2. This timer solution isn't the full solution but only a stopgap measure.
// See notes in ReleaseKeyLowLevel for more -- ultimate solution will involve
// the callback itself, and some kind of encrypted storage of hashed passwords,
// using session keys, as well as an option to use ssh-agent and other standard
// APIs for protected memory.
//
// UPDATE: Am in the process now of adding the actual Master key. Therefore
// OT_MASTER_KEY_TIMEOUT was added for the actual mechanism, while OT_KEY_TIMER
// (a stopgap measure) was set to 0, which makes it of no effect. Probably
// OT_KEY_TIMER will be removed entirely (we'll see.)
#define OT_KEY_TIMER 30
// TODO: Next release, as users get converted to file format 2.0 (master key)
// then reduce this timer from 30 to 0. (30 is just to help them convert.)
//#define OT_KEY_TIMER 0
//#define OT_MASTER_KEY_TIMEOUT 300  // This is in OTEnvelope.h
// FYI: 1800 seconds is 30 minutes, 300 seconds is 5 mins.
#endif  // OT_KEY_TIMER

/** This is the only part of the API that actually accepts objects as
parameters,
since the above objects have SWIG C++ wrappers.
Caller must have Callback attached already. */
EXPORT bool OT_API_Set_PasswordCallback(OTCaller& theCaller);

/** For getting the password from the user, for using his private key. */
extern "C" {
typedef int32_t OT_OPENSSL_CALLBACK(
    char* buf,
    int32_t size,
    int32_t rwflag,
    void* userdata);  // <== Callback type, used for declaring.

EXPORT OT_OPENSSL_CALLBACK default_pass_cb;
EXPORT OT_OPENSSL_CALLBACK souped_up_pass_cb;
}

class OTAsymmetricKey
{
private:
    OTAsymmetricKey(const OTAsymmetricKey&);
    OTAsymmetricKey& operator=(const OTAsymmetricKey&);

public:
    static String KeyTypeToString(const proto::AsymmetricKeyType keyType);

    static proto::AsymmetricKeyType StringToKeyType(const String& keyType);

    proto::AsymmetricKeyType keyType() const;

    virtual CryptoAsymmetric& engine() const = 0;
    virtual const std::string Path() const;

private:
    static OTAsymmetricKey* KeyFactory(
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role);

protected:
    proto::AsymmetricKeyType m_keyType = proto::AKEYTYPE_ERROR;
    proto::KeyRole role_ = proto::KEYROLE_ERROR;
    OTAsymmetricKey(const proto::AsymmetricKeyType keyType, const proto::KeyRole role);

public:
    /** Caller IS responsible to delete! */
    EXPORT static OTAsymmetricKey* KeyFactory(
        const proto::AsymmetricKeyType keyType,
        const String& pubkey);
    /** Caller IS responsible to delete! */
    EXPORT static OTAsymmetricKey* KeyFactory(
        const NymParameters& nymParameters,
        const proto::KeyRole role);
    /** Caller IS responsible to delete! */
    EXPORT static OTAsymmetricKey* KeyFactory(
        const proto::AsymmetricKey& serializedKey);

public:
    static void SetPasswordCallback(OT_OPENSSL_CALLBACK* pCallback);
    EXPORT static OT_OPENSSL_CALLBACK* GetPasswordCallback();
    static bool IsPasswordCallbackSet()
    {
        return (nullptr == s_pwCallback) ? false : true;
    }
    EXPORT static bool SetPasswordCaller(OTCaller& theCaller);
    static OTCaller* GetPasswordCaller();

protected:
    static OT_OPENSSL_CALLBACK* s_pwCallback;
    static OTCaller* s_pCaller;

protected:
    bool m_bIsPublicKey = false;
    bool m_bIsPrivateKey = false;
    Timer m_timer;  // Useful for keeping track how long since I last entered my
                    // passphrase...
public:
    /** Just access this directly, like a struct. (Check for nullptr.) */
    OTSignatureMetadata* m_pMetadata{nullptr};

    // To use m_metadata, call m_metadata.HasMetadata(). If it's true, then you
    // can see these values:
    //    char m_metadata::Getproto::AsymmetricKeyType()             // Can be A, E, or S
    //    (authentication, encryption, or signing. Also, E would be unusual.)
    //    char m_metadata::FirstCharNymID()         // Can be any letter from
    //    base62 alphabet. Represents first letter of a Nym's ID.
    //    char m_metadata::FirstCharMasterCredID()  // Can be any letter from
    //    base62 alphabet. Represents first letter of a Master Credential ID
    //    (for that Nym.)
    //    char m_metadata::FirstCharChildCredID()     // Can be any letter from
    //    base62 alphabet. Represents first letter of a Credential ID (signed by
    //    that Master.)
    //
    // Here's how metadata works: It's optional. You can set it, or not. If it's
    // there, OT will add it to the signature on the contract itself, when this
    // key is used to sign something. (OTSignature has the same
    // OTSignatureMetadata struct.) Later on when verifying the signature, the
    // metadata is used to speed up the lookup/verification process so we don't
    // have to verify the signature against every single child key credential
    // available for that Nym. In practice, however, we are adding metadata to
    // every single signature (except possibly cash...) (And we will make it
    // mandatory for Nyms who use credentials.)

protected:
    void ReleaseKeyLowLevel();                         // call this.
    virtual void ReleaseKeyLowLevel_Hook() const = 0;  // override this.
    // CONSTRUCTION (PROTECTED)
    explicit OTAsymmetricKey(const proto::AsymmetricKey& serializedKey);
    EXPORT OTAsymmetricKey();

public:
    Data SerializeKeyToData(const proto::AsymmetricKey& rhs) const;
    bool operator==(const proto::AsymmetricKey&) const;
    EXPORT virtual ~OTAsymmetricKey();
    virtual void Release();
    void Release_AsymmetricKey();
    void ReleaseKey();

    // PUBLIC METHODS
    virtual bool hasCapability(const NymCapability& capability) const;
    virtual bool IsEmpty() const = 0;
    inline bool IsPublic() const { return m_bIsPublicKey; }
    inline bool IsPrivate() const { return m_bIsPrivateKey; }
    /** Don't use this, normally it's not necessary. */
    inline void SetAsPublic()
    {
        m_bIsPublicKey = true;
        m_bIsPrivateKey = false;
    }
    /** (Only if you really know what you are doing.) */
    inline void SetAsPrivate()
    {
        m_bIsPublicKey = false;
        m_bIsPrivateKey = true;
    }
    const proto::KeyRole& Role() { return role_; }
    // We're moving to a system where the actual key isn't kept loaded in memory
    // except under 2 circumstances: 1. We are using it currently, and we're
    // going to destroy it when we're done with it. 2. A timer is running, and
    // until the 10 minutes are up, the private key is available. But:
    // Presumably it's stored in PROTECTED MEMORY, either with specific tricks
    // used to prevent swapping and to zero after we're done, and to prevent
    // core dumps, or it's stored in ssh-agent or some similar standard API
    // (gpg-agent, keychain Mac Keychain, etc) or Windows protected memory etc
    // etc. Inside OT I can also give the option to go with our own security
    // tricks (listed above...) or to keep the timer length at 0, forcing the
    // password to be entered over and over again. IDEA: When the user enters
    // the passphrase for a specific Nym, hash it (so your plaintext passphrase
    // isn't stored in memory anywhere) and then USE that hash as the passphrase
    // on the actual key. (Meaning also that the user will not be able to use
    // his passphrase outside of OT, until he EXPORTS the Nym, since he would
    // also have to hash the passphrase before manipulating the raw key file.)
    // At this point, we have the hash of the user's passphrase, which is what
    // we actually use for opening his private key (which is also normally kept
    // in an encrypted form, on the hard drive AND in RAM!!) So everything from
    // above still applies: I don't want to reveal that hash, so I store it
    // using tricks to secure the memory (I have to do this part anyway, ANYTIME
    // I touch certain forms of data), or in ssh-agent, and so on, except a
    // timer can be set after the user first enters his passphrase. For ultimate
    // security, just set the timer to 0 and type your passphrase every single
    // time. But instead let's say you set it to 10 minutes. I don't want to
    // store that password hash, either. (The hash protects the plaintext
    // password, but the hash IS the ACTUAL password, so while the plaintext PW
    // is protected, the actual one is still not.) Therefore I will select some
    // random data from OpenSSL for use as a TEMPORARY password, a session key,
    // and then encrypt all the hashes to that session key. This way, the actual
    // passphrases (hashed version) do NOT appear in memory, AND NEITHER DO the
    // plaintext versions! You might ask, well then why not just encrypt the
    // plaintext passphrase itself and use that? The answer is, to prevent
    // making it recoverable. You don't even want someone to get that session
    // key and then recover your PLAINTEXT password! Maybe he'll go use it on a
    // thousand websites!
    //
    // Next: How to protect the session key (an OTSymmetricKey) from being
    // found? First: destroy it often. Make a new session key EVERY time the
    // timeout happens. Also: use it in protected memory as before. This could
    // ALWAYS have a timeout of 0! If it's in ssh-agent, what more can you do?
    // At least OT will make this configurable, and will be pretty damned secure
    // in its own right. Ultimately the best solution here is an extern hardware
    // such as a smart card.

    /** Only works for public keys. */
    virtual bool CalculateID(Identifier& theOutput) const;
    virtual bool GetPublicKey(String& strKey) const = 0;
    virtual bool ReEncryptPrivateKey(
        const OTPassword& theExportPassword,
        bool bImporting) const = 0;
    virtual serializedAsymmetricKey Serialize() const;
    virtual bool Verify(const Data& plaintext, const proto::Signature& sig)
        const;
    virtual proto::HashType SigHashType() const
    {
        return CryptoEngine::StandardHash;
    }
    virtual bool Sign(
        const Data& plaintext,
        proto::Signature& sig,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* exportPassword = nullptr,
        const String& credID = String(""),
        const proto::SignatureRole role = proto::SIGROLE_ERROR) const;
    virtual bool TransportKey(
        Data& publicKey,
        OTPassword& privateKey) const = 0;

    template<class C>
    bool SignProto(
        C& serialized,
        proto::Signature& signature,
        const String& credID = String(""),
        const OTPasswordData* pPWData = nullptr) const
            {
                if (IsPublic()) {
                    otErr << "You must use private keys to create signatures."
                          << std::endl;

                    return false;
                }

                if (0 == signature.version()) {
                    signature.set_version(1);
                }

                signature.set_credentialid(credID.Get());

                if ((proto::HASHTYPE_ERROR == signature.hashtype()) ||
                    !signature.has_hashtype()) {
                    signature.set_hashtype(SigHashType());
                }

                Data sig;
                bool goodSig = engine().Sign(
                    proto::ProtoAsData<C>(serialized),
                    *this,
                    signature.hashtype(),
                    sig,
                    pPWData,
                    nullptr);

                if (goodSig) {
                    signature.set_signature(sig.GetPointer(), sig.GetSize());
                }

                return goodSig;
            }
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_OTASYMMETRICKEY_HPP
