// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/LowLevelKeyGenerator.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/Native.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/core/crypto/mkcert.hpp"
#endif
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Timer.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/core/Data.hpp"
#endif
#include "opentxs/core/Log.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/key/Ed25519.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/key/Keypair.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/crypto/key/RSA.hpp"
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/key/Secp256k1.hpp"
#endif
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/crypto/library/Secp256k1.hpp"
#endif
#include "opentxs/crypto/library/Sodium.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "crypto/key/RSA_private.hpp"
#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif
#include "crypto/key/Keypair.hpp"
#include "crypto/key/RSA.hpp"

#include <cstdint>
#include <ostream>

#define OT_METHOD "opentxs::LowLevelKeyGenerator::"

namespace opentxs
{

class LowLevelKeyGenerator::LowLevelKeyGeneratordp
{
public:
    LowLevelKeyGeneratordp() = default;
    virtual void Cleanup() = 0;
    virtual ~LowLevelKeyGeneratordp() = default;
};

class LowLevelKeyGenerator::LowLevelKeyGeneratorECdp
    : public LowLevelKeyGeneratordp
{
public:
    OTPassword privateKey_;
    OTData publicKey_;

    LowLevelKeyGeneratorECdp();
    virtual void Cleanup();
};

#if OT_CRYPTO_SUPPORTED_KEY_RSA
class LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp
    : public LowLevelKeyGeneratordp
{
public:
    virtual void Cleanup();

    X509* m_pX509 = nullptr;
    /** Instantiated form of key. (For private keys especially, we don't want it
     *  instantiated for any longer than absolutely necessary.)
     */
    EVP_PKEY* m_pKey = nullptr;
};
#endif

LowLevelKeyGenerator::~LowLevelKeyGenerator()
{
    if (m_bCleanup) { Cleanup(); }
    dp.reset();
    if (pkeyData_) { pkeyData_.release(); }
}

LowLevelKeyGenerator::LowLevelKeyGenerator(const NymParameters& pkeyData)
    : m_bCleanup(true)
{
    pkeyData_.reset(const_cast<NymParameters*>(&pkeyData));

    OT_ASSERT(pkeyData_);

    switch (pkeyData_->nymParameterType()) {
#if OT_CRYPTO_USING_LIBSECP256K1
        case (NymParameterType::SECP256K1):
#endif
        case (NymParameterType::ED25519): {
            dp.reset(new LowLevelKeyGeneratorECdp);

            break;
        }
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (NymParameterType::RSA): {
            dp.reset(new LowLevelKeyGeneratorOpenSSLdp);

            break;
        }
#endif
        default: {
        }
    }
}

// Don't force things by explicitly calling this function, unless you are SURE
// there's no one else cleaning up the same objects. Notice the if (m_bCleanup)
// just above in the destructor, for that very reason.
void LowLevelKeyGenerator::Cleanup()
{

    if (dp) { dp->Cleanup(); }
}

#if OT_CRYPTO_SUPPORTED_KEY_RSA
void LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp::Cleanup()
{
    if (nullptr != m_pKey) {
        EVP_PKEY_free(m_pKey);
        m_pKey = nullptr;
    }
    if (nullptr != m_pX509) {
        X509_free(m_pX509);
        m_pX509 = nullptr;
    }
}
#endif

LowLevelKeyGenerator::LowLevelKeyGeneratorECdp::LowLevelKeyGeneratorECdp()
    : privateKey_()
    , publicKey_(Data::Factory())
{
}

void LowLevelKeyGenerator::LowLevelKeyGeneratorECdp::Cleanup()
{
    privateKey_.zeroMemory();
}

bool LowLevelKeyGenerator::MakeNewKeypair()
{
    if (!pkeyData_) { return false; }

    switch (pkeyData_->nymParameterType()) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case (NymParameterType::ED25519): {
            const auto& engine = dynamic_cast<const crypto::EcdsaProvider&>(
                OT::App().Crypto().ED25519());
            LowLevelKeyGenerator::LowLevelKeyGeneratorECdp& ldp =
                static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorECdp&>(
                    *dp);

            return engine.RandomKeypair(ldp.privateKey_, ldp.publicKey_);
        } break;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (NymParameterType::SECP256K1): {
            const auto& engine = dynamic_cast<const crypto::EcdsaProvider&>(
                OT::App().Crypto().SECP256K1());
            LowLevelKeyGenerator::LowLevelKeyGeneratorECdp& ldp =
                static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorECdp&>(
                    *dp);

            return engine.RandomKeypair(ldp.privateKey_, ldp.publicKey_);
        } break;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (NymParameterType::RSA): {
#if OT_CRYPTO_USING_OPENSSL
            //  OpenSSL_BIO bio_err = nullptr;
            X509* x509 = nullptr;
            EVP_PKEY* pNewKey = nullptr;

            //  CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON); // memory leak detection.
            // Leaving this for now.
            //  bio_err    =    BIO_new_fp(stderr, BIO_NOCLOSE);

            // actually generate the things. // TODO THESE PARAMETERS...(mkcert)
            mkcert(&x509, &pNewKey, pkeyData_->keySize(), 0, 3650);  // 3650=10
            // years. Todo
            // hardcoded.
            // Note: 512 bit key CRASHES
            // 1024 is apparently a minimum requirement, if not an only
            // requirement.
            // Will need to go over just what sorts of keys are involved here...
            // todo.

            if (nullptr == x509) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed attempting to generate new x509 cert.")
                    .Flush();

                if (nullptr != pNewKey) EVP_PKEY_free(pNewKey);
                pNewKey = nullptr;

                return false;
            }

            if (nullptr == pNewKey) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failed attempting to generate new private key.")
                    .Flush();

                if (nullptr != x509) X509_free(x509);
                x509 = nullptr;

                return false;
            }

            // Below this point, x509 and pNewKey will need to be cleaned up
            // properly.

            if (m_bCleanup) Cleanup();

            m_bCleanup = true;

            LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp& ldp =
                static_cast<
                    LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp&>(*dp);

            ldp.m_pKey = pNewKey;
            ldp.m_pX509 = x509;

            return true;
#endif
        } break;
#endif
        default: {

            return false;  // unsupported keyType
        }
    }
}

bool LowLevelKeyGenerator::SetOntoKeypair(
    crypto::key::Keypair& input,
    OTPasswordData& passwordData)
{
    if (!pkeyData_) { return false; }

    auto& keypair = dynamic_cast<crypto::key::implementation::Keypair&>(input);

    switch (pkeyData_->nymParameterType()) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
        case (NymParameterType::ED25519): {
            const auto& engine = dynamic_cast<const crypto::EcdsaProvider&>(
                OT::App().Crypto().ED25519());
            LowLevelKeyGenerator::LowLevelKeyGeneratorECdp& ldp =
                static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorECdp&>(
                    *dp);

            // Since we are in ed25519-specific code, we have to make sure these
            // are ed25519-specific keys.
            auto* pPublicKey = dynamic_cast<crypto::key::Ed25519*>(
                &keypair.m_pkeyPublic.get());
            auto* pPrivateKey = dynamic_cast<crypto::key::Ed25519*>(
                &keypair.m_pkeyPrivate.get());

            if (nullptr == pPublicKey) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": dynamic_cast of public key to "
                    "crypto::key::Ed25519 failed.")
                    .Flush();

                return false;
            }

            if (nullptr == pPrivateKey) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": dynamic_cast of private key to "
                    "crypto::key::Ed25519 failed.")
                    .Flush();

                return false;
            }

            pPublicKey->SetAsPublic();
            pPrivateKey->SetAsPrivate();
            const bool pubkeySet =
                engine.ECPubkeyToAsymmetricKey(ldp.publicKey_, *pPublicKey);
            const bool privkeySet = engine.ECPrivatekeyToAsymmetricKey(
                ldp.privateKey_, passwordData, *pPrivateKey);

            return (pubkeySet && privkeySet);
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (NymParameterType::SECP256K1): {
            const auto& engine = dynamic_cast<const crypto::EcdsaProvider&>(
                OT::App().Crypto().SECP256K1());
            LowLevelKeyGenerator::LowLevelKeyGeneratorECdp& ldp =
                static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorECdp&>(
                    *dp);

            // Since we are in secp256k1-specific code, we have to make sure
            // these are secp256k1-specific keys.
            auto* pPublicKey = dynamic_cast<crypto::key::Secp256k1*>(
                &keypair.m_pkeyPublic.get());
            auto* pPrivateKey = dynamic_cast<crypto::key::Secp256k1*>(
                &keypair.m_pkeyPrivate.get());

            if (nullptr == pPublicKey) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": dynamic_cast of public key to "
                    "crypto::key::AsymmetricSecp256k1 failed.")
                    .Flush();

                return false;
            }

            if (nullptr == pPrivateKey) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": dynamic_cast of private key to "
                    "crypto::key::AsymmetricSecp256k1 failed.")
                    .Flush();

                return false;
            }

            pPublicKey->SetAsPublic();
            pPrivateKey->SetAsPrivate();
            const bool pubkeySet =
                engine.ECPubkeyToAsymmetricKey(ldp.publicKey_, *pPublicKey);
            const bool privkeySet = engine.ECPrivatekeyToAsymmetricKey(
                ldp.privateKey_, passwordData, *pPrivateKey);

            return (pubkeySet && privkeySet);
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (NymParameterType::RSA): {
#if OT_CRYPTO_USING_OPENSSL
            LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp& ldp =
                static_cast<
                    LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp&>(*dp);

            // Since we are in OpenSSL-specific code, we have to make sure these
            // are OpenSSL-specific keys.
            auto* pPublicKey = dynamic_cast<crypto::key::implementation::RSA*>(
                &keypair.m_pkeyPublic.get());
            auto* pPrivateKey = dynamic_cast<crypto::key::implementation::RSA*>(
                &keypair.m_pkeyPrivate.get());

            if (nullptr == pPublicKey) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": dynamic_cast of public key to "
                    "crypto::key::RSA failed.")
                    .Flush();

                return false;
            }
            if (nullptr == pPrivateKey) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": dynamic_cast of private key to "
                    "crypto::key::RSA failed.")
                    .Flush();

                return false;
            }

            // Now we can call OpenSSL-specific methods on these keys...
            //
            pPublicKey->SetAsPublic();
            pPublicKey->dp->SetKeyAsCopyOf(*ldp.m_pKey);
            pPublicKey->dp->SetX509(ldp.m_pX509);
            ldp.m_pX509 = nullptr;
            pPrivateKey->SetAsPrivate();
            pPrivateKey->dp->SetKeyAsCopyOf(*ldp.m_pKey, true);
            EVP_PKEY_free(ldp.m_pKey);
            ldp.m_pKey = nullptr;

            return true;
#else
            break;
#endif  // OT_CRYPTO_USING_OPENSSL
        }
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
        default: {

            return false;  // unsupported keyType
        }
    }
}
}  // namespace opentxs
