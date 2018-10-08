// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Asymmetric.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_RSA
namespace opentxs::crypto::key::implementation
{
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

class RSA final : virtual public key::RSA, public Asymmetric
{
public:
    const crypto::AsymmetricProvider& engine() const override;
    bool GetPrivateKey(
        String& strOutput,
        const key::Asymmetric* pPubkey,
        const String& pstrReason = String::Factory(),
        const OTPassword* pImportPassword = nullptr) const override;
    bool GetPublicKey(String& strKey) const override;
    bool IsEmpty() const override;
    bool ReEncryptPrivateKey(
        const OTPassword& theExportPassword,
        bool bImporting) const override;
    /** Don't ever call this. It's only here because it's impossible to get rid
     * of unless and until RSA key support is removed entirely. */
    bool SaveCertToString(
        String& strOutput,
        const String& pstrReason = String::Factory(),
        const OTPassword* pImportPassword = nullptr) const override;
    std::shared_ptr<proto::AsymmetricKey> Serialize() const override;
    proto::HashType SigHashType() const override
    {
        return proto::HASHTYPE_SHA256;
    }
    bool TransportKey(Data& publicKey, OTPassword& privateKey) const override;

    void Release() override;
    bool SetPrivateKey(
        const String& strCert,
        const String& pstrReason = String::Factory(),
        const OTPassword* pImportPassword = nullptr) override;
    bool SetPublicKey(const String& strKey) override;
    bool SetPublicKeyFromPrivateKey(
        const String& strCert,
        const String& pstrReason = String::Factory(),
        const OTPassword* pImportPassword = nullptr) override;

    ~RSA();

private:
    typedef Asymmetric ot_super;

    friend opentxs::Factory;
    friend LowLevelKeyGenerator;
    friend crypto::implementation::OpenSSL;

    class d;

    d* dp{nullptr};
    /** base64-encoded, string form of key. (Encrypted too, for private keys.
     * Should store it in this form most of the time.) m_p_ascKey is the most
     * basic value. m_pKey is derived from it, for example. */
    OTArmored m_p_ascKey = Armored::Factory();

    RSA* clone() const override;

    void Release_AsymmetricKey_OpenSSL();
    void ReleaseKeyLowLevel_Hook() override;

    explicit RSA(const proto::AsymmetricKey& serializedKey);
    explicit RSA(const String& publicKey);
    explicit RSA(const proto::KeyRole role);
    RSA();
    RSA(const RSA&) = delete;
    RSA(RSA&&) = delete;
    RSA& operator=(const RSA&) = delete;
    RSA& operator=(RSA&&) = delete;
};
}  // namespace opentxs::crypto::key::implementation
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
