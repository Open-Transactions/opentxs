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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/crypto/LowLevelKeyGenerator.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/AsymmetricKeyEd25519.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/core/crypto/AsymmetricKeySecp256k1.hpp"
#endif
#include "opentxs/core/crypto/CryptoEngine.hpp"
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/core/crypto/Libsecp256k1.hpp"
#endif
#include "opentxs/core/crypto/Libsodium.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/core/crypto/mkcert.hpp"
#endif
#include "opentxs/core/crypto/NymParameters.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/core/crypto/OTAsymmetricKey_OpenSSLPrivdp.hpp"
#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif
#include "opentxs/core/crypto/OTKeypair.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/core/crypto/OTPassword.hpp"
#endif
#include "opentxs/core/util/Assert.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/core/Data.hpp"
#endif
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Types.hpp"

#include <stdint.h>
#include <ostream>

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
    std::unique_ptr<Data> publicKey_;

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
    if (m_bCleanup) {
        Cleanup();
    }
    dp.reset();
    if (pkeyData_) {
        pkeyData_.release();
    }
}

LowLevelKeyGenerator::LowLevelKeyGenerator(const NymParameters& pkeyData)
    : m_bCleanup(true)
{
    pkeyData_.reset(const_cast<NymParameters*>(&pkeyData));

    OT_ASSERT(pkeyData_);

    switch (pkeyData_->nymParameterType()) {
#if OT_CRYPTO_USING_LIBSECP256K1
        case (NymParameterType::SECP256K1) :
#endif
        case (NymParameterType::ED25519) : {
            dp.reset(new LowLevelKeyGeneratorECdp);

            break;
        }
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (NymParameterType::RSA) : {
            dp.reset(new LowLevelKeyGeneratorOpenSSLdp);

            break;
        }
#endif
        default : {}
    }
}

// Don't force things by explicitly calling this function, unless you are SURE
// there's no one else cleaning up the same objects. Notice the if (m_bCleanup)
// just above in the destructor, for that very reason.
void LowLevelKeyGenerator::Cleanup()
{

    if (dp) {
        dp->Cleanup();
    }
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

LowLevelKeyGenerator::LowLevelKeyGeneratorECdp::
    LowLevelKeyGeneratorECdp()
{
    publicKey_.reset(new Data);
}

void LowLevelKeyGenerator::LowLevelKeyGeneratorECdp::Cleanup()
{
    privateKey_.zeroMemory();

}

bool LowLevelKeyGenerator::MakeNewKeypair()
{
    if (!pkeyData_) { return false; }

    switch (pkeyData_->nymParameterType()) {
        case (NymParameterType::ED25519) : {
            Libsodium& engine =
                static_cast<Libsodium&>(OT::App().Crypto().ED25519());
            LowLevelKeyGenerator::LowLevelKeyGeneratorECdp& ldp =
                static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorECdp&>
                    (*dp);

            return engine.RandomKeypair(ldp.privateKey_, *ldp.publicKey_);
        }
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (NymParameterType::SECP256K1) : {
#if OT_CRYPTO_USING_LIBSECP256K1
            Libsecp256k1& engine =
            static_cast<Libsecp256k1&>(OT::App().Crypto().SECP256K1());
#endif
            LowLevelKeyGenerator::LowLevelKeyGeneratorECdp& ldp =
                static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorECdp&>
                    (*dp);

            return engine.RandomKeypair(ldp.privateKey_, *ldp.publicKey_);
        }
#endif
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (NymParameterType::RSA) : {
#if OT_CRYPTO_USING_OPENSSL
            //  OpenSSL_BIO bio_err = nullptr;
            X509* x509          = nullptr;
            EVP_PKEY* pNewKey   = nullptr;

            //  CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON); // memory leak detection.
                // Leaving this for now.
            //  bio_err    =    BIO_new_fp(stderr, BIO_NOCLOSE);

            // actually generate the things. // TODO THESE PARAMETERS...(mkcert)
            mkcert(&x509, &pNewKey, pkeyData_->keySize(), 0, 3650); // 3650=10 years. Todo hardcoded.
            // Note: 512 bit key CRASHES
            // 1024 is apparently a minimum requirement, if not an only requirement.
            // Will need to go over just what sorts of keys are involved here... todo.

            if (nullptr == x509) {
                otErr << __FUNCTION__
                    << ": Failed attempting to generate new x509 cert.\n";

                if (nullptr != pNewKey) EVP_PKEY_free(pNewKey);
                pNewKey = nullptr;

                return false;
            }

            if (nullptr == pNewKey) {
                otErr << __FUNCTION__
                    << ": Failed attempting to generate new private key.\n";

                if (nullptr != x509) X509_free(x509);
                x509 = nullptr;

                return false;
            }

            // Below this point, x509 and pNewKey will need to be cleaned up properly.

            if (m_bCleanup) Cleanup();

            m_bCleanup = true;

            LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp& ldp =
                static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp&>
                    (*dp);

            ldp.m_pKey = pNewKey;
            ldp.m_pX509 = x509;

            return true;
#else
            break;
#endif
        }
#endif
        default : {

            return false; //unsupported keyType
        }
    }
}

bool LowLevelKeyGenerator::SetOntoKeypair(
    OTKeypair& theKeypair,
    OTPasswordData& passwordData)
{
    if (!pkeyData_) { return false; }

    switch (pkeyData_->nymParameterType()) {
        case (NymParameterType::ED25519) : {
            Libsodium& engine =
                static_cast<Libsodium&>(OT::App().Crypto().ED25519());
            LowLevelKeyGenerator::LowLevelKeyGeneratorECdp& ldp =
                static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorECdp&>
                    (*dp);

            OT_ASSERT(theKeypair.m_pkeyPublic);
            OT_ASSERT(theKeypair.m_pkeyPrivate);

            // Since we are in ed25519-specific code, we have to make sure these
            // are ed25519-specific keys.
            std::shared_ptr<AsymmetricKeyEd25519> pPublicKey =
                std::dynamic_pointer_cast<AsymmetricKeyEd25519>
                (theKeypair.m_pkeyPublic);

            std::shared_ptr<AsymmetricKeyEd25519> pPrivateKey =
                std::dynamic_pointer_cast<AsymmetricKeyEd25519>
                    (theKeypair.m_pkeyPrivate);

            if (!pPublicKey) {
                otErr << __FUNCTION__ << ": dynamic_cast of public key to "
                    << "AsymmetricKeyEd25519 failed." << std::endl;

                return false;
            }

            if (!pPrivateKey) {
                otErr << __FUNCTION__ << ": dynamic_cast of private key to "
                    << "AsymmetricKeyEd25519 failed." << std::endl;

                return false;
            }

            pPublicKey->SetAsPublic();
            pPrivateKey->SetAsPrivate();

            bool pubkeySet =
                engine.ECPubkeyToAsymmetricKey(ldp.publicKey_, *pPublicKey);
            bool privkeySet = engine.ECPrivatekeyToAsymmetricKey(
                ldp.privateKey_, passwordData, *pPrivateKey);

            return (pubkeySet && privkeySet);
        }
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        case (NymParameterType::SECP256K1) : {
#if OT_CRYPTO_USING_LIBSECP256K1
            Libsecp256k1& engine =
                static_cast<Libsecp256k1&>(OT::App().Crypto().SECP256K1());
#endif
            LowLevelKeyGenerator::LowLevelKeyGeneratorECdp& ldp =
                static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorECdp&>
                    (*dp);

            OT_ASSERT(theKeypair.m_pkeyPublic);
            OT_ASSERT(theKeypair.m_pkeyPrivate);

            // Since we are in secp256k1-specific code, we have to make sure
            // these are secp256k1-specific keys.
            std::shared_ptr<AsymmetricKeySecp256k1> pPublicKey =
                std::dynamic_pointer_cast<AsymmetricKeySecp256k1>
                    (theKeypair.m_pkeyPublic);

            std::shared_ptr<AsymmetricKeySecp256k1> pPrivateKey =
                std::dynamic_pointer_cast<AsymmetricKeySecp256k1>
                    (theKeypair.m_pkeyPrivate);

            if (!pPublicKey) {
                otErr << __FUNCTION__ << ": dynamic_cast of public key to "
                      << "OTAsymmetricKeySecp256k1 failed." << std::endl;

                return false;
            }

            if (!pPrivateKey) {
                otErr << __FUNCTION__ << ": dynamic_cast of private key to "
                      << "OTAsymmetricKeySecp256k1 failed." << std::endl;

                return false;
            }

            pPublicKey->SetAsPublic();
            pPrivateKey->SetAsPrivate();

            bool pubkeySet =
                engine.ECPubkeyToAsymmetricKey(ldp.publicKey_, *pPublicKey);
            bool privkeySet = engine.ECPrivatekeyToAsymmetricKey(
                ldp.privateKey_, passwordData, *pPrivateKey);

            return (pubkeySet && privkeySet);
        }
#endif // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        case (NymParameterType::RSA) : {
#if OT_CRYPTO_USING_OPENSSL
            LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp& ldp =
                static_cast<LowLevelKeyGenerator::LowLevelKeyGeneratorOpenSSLdp&>
                    (*dp);

            OT_ASSERT(theKeypair.m_pkeyPublic);
            OT_ASSERT(theKeypair.m_pkeyPrivate);

            // Since we are in OpenSSL-specific code, we have to make sure these
            // are OpenSSL-specific keys.
            std::shared_ptr<OTAsymmetricKey_OpenSSL> pPublicKey =
                std::dynamic_pointer_cast<OTAsymmetricKey_OpenSSL>
                    (theKeypair.m_pkeyPublic);

            std::shared_ptr<OTAsymmetricKey_OpenSSL> pPrivateKey =
                std::dynamic_pointer_cast<OTAsymmetricKey_OpenSSL>
                (theKeypair.m_pkeyPrivate);

            if (!pPublicKey) {
                otErr << __FUNCTION__ << ": dynamic_cast of public key to "
                      << "OTAsymmetricKey_OpenSSL failed." << std::endl;

                return false;
            }
            if (!pPrivateKey) {
                otErr << __FUNCTION__ << ": dynamic_cast of private key to "
                      << "OTAsymmetricKey_OpenSSL failed." << std::endl;

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
#endif // OT_CRYPTO_USING_OPENSSL
        }
#endif // OT_CRYPTO_SUPPORTED_KEY_RSA
        default : {

            return false; //unsupported keyType
        }
    }
}
} // namespace opentxs
