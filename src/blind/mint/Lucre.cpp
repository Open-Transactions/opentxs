// // Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CASH_USING_LUCRE
#include "opentxs/blind/Mint.hpp"
#include "opentxs/blind/Token.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"

#include "blind/token/Lucre.hpp"
#include "blind/Lucre.hpp"
#include "blind/Mint.hpp"
#include "crypto/library/OpenSSL_BIO.hpp"

#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/ossl_typ.h>
#include <stdio.h>
#include <sys/types.h>
#include <ostream>
#endif

#include "Lucre.hpp"

#if OT_CASH_USING_LUCRE
#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define OT_METHOD "opentxs::blind::mint::implementation::Lucre"

namespace opentxs
{
blind::Mint* Factory::MintLucre(const api::Core& core)
{
    return new blind::mint::implementation::Lucre(core);
}

blind::Mint* Factory::MintLucre(
    const api::Core& core,
    const String& strNotaryID,
    const String& strInstrumentDefinitionID)
{
    return new blind::mint::implementation::Lucre(
        core, strNotaryID, strInstrumentDefinitionID);
}

blind::Mint* Factory::MintLucre(
    const api::Core& core,
    const String& strNotaryID,
    const String& strServerNymID,
    const String& strInstrumentDefinitionID)
{
    return new blind::mint::implementation::Lucre(
        core, strNotaryID, strServerNymID, strInstrumentDefinitionID);
}

}  // namespace opentxs

namespace opentxs::blind::mint::implementation
{
Lucre::Lucre(const api::Core& core)
    : Contract(core)
    , Mint(core)
{
}

Lucre::Lucre(
    const api::Core& core,
    const String& strNotaryID,
    const String& strInstrumentDefinitionID)
    : Contract(core, strInstrumentDefinitionID)
    , Mint(core, strNotaryID, strInstrumentDefinitionID)
{
}

Lucre::Lucre(
    const api::Core& core,
    const String& strNotaryID,
    const String& strServerNymID,
    const String& strInstrumentDefinitionID)
    : Contract(core, strInstrumentDefinitionID)
    , Mint(core, strNotaryID, strServerNymID, strInstrumentDefinitionID)
{
}

// The mint has a different key pair for each denomination.
// Pass the actual denomination such as 5, 10, 20, 50, 100...
bool Lucre::AddDenomination(
    const Nym& theNotary,
    const std::int64_t denomination,
    const std::size_t keySize)
{
    bool bReturnValue = false;

    // Let's make sure it doesn't already exist
    auto theArmor = Armored::Factory();
    if (GetPublic(theArmor, denomination)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Denomination public already exists in AddDenomination.")
            .Flush();
        return false;
    }
    if (GetPrivate(theArmor, denomination)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Denomination private already exists in "
            "AddDenomination.")
            .Flush();
        return false;
    }

    if ((keySize / 8) < (MIN_COIN_LENGTH + DIGEST_LENGTH)) {

        LogOutput(OT_METHOD)(__FUNCTION__)(": Prime must be at least ")(
            (MIN_COIN_LENGTH + DIGEST_LENGTH) * 8)(" bits.")
            .Flush();
        return false;
    }

    if (keySize % 8) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Prime length must be a multiple of 8.")
            .Flush();
        return false;
    }

#if OT_LUCRE_DEBUG
#ifdef _WIN32
    BIO* out = BIO_new_file("openssl.dump", "w");
    assert(out);
    SetDumper(out);
#else
    SetMonitor(stderr);
#endif
#endif

    crypto::implementation::OpenSSL_BIO bio = BIO_new(BIO_s_mem());
    crypto::implementation::OpenSSL_BIO bioPublic = BIO_new(BIO_s_mem());

    // Generate the mint private key information
    Bank bank(keySize / 8);
    bank.WriteBIO(bio);

    // Generate the mint public key information
    PublicBank pbank(bank);
    pbank.WriteBIO(bioPublic);
    const auto strPrivateBank = bio.ToString();
    const auto strPublicBank = bioPublic.ToString();

    if (strPrivateBank->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to generate private mint")
            .Flush();

        return false;
    }

    if (strPublicBank->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to generate public mint")
            .Flush();

        return false;
    }

    auto pPublic = Armored::Factory();
    auto pPrivate = Armored::Factory();

    // Set the public bank info onto pPublic
    pPublic->SetString(strPublicBank, true);  // linebreaks = true

    // Seal the private bank info up into an encrypted Envelope
    // and set it onto pPrivate
    OTEnvelope theEnvelope;
    theEnvelope.Seal(theNotary, strPrivateBank);  // Todo check the return
                                                  // values on these two
                                                  // functions
    theEnvelope.GetCiphertext(pPrivate);

    // Add the new key pair to the maps, using denomination as the key
    m_mapPublic.emplace(denomination, std::move(pPublic));
    m_mapPrivate.emplace(denomination, std::move(pPrivate));

    // Grab the Server Nym ID and save it with this Mint
    theNotary.GetIdentifier(m_ServerNymID);
    m_nDenominationCount++;
    bReturnValue = true;
    LogDetail(OT_METHOD)(__FUNCTION__)(": Successfully added denomination: ")(
        denomination)
        .Flush();

    return bReturnValue;
}

#if OT_CRYPTO_USING_OPENSSL

// Lucre step 3: the mint signs the token
//
bool Lucre::SignToken(const Nym& notary, blind::Token& token)
{
#if OT_LUCRE_DEBUG
    LucreDumper setDumper;
#endif

    if (proto::CASHTYPE_LUCRE != token.Type()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect token type").Flush();

        return false;
    } else {
        LogInsane(OT_METHOD)(__FUNCTION__)(": Signing a lucre token").Flush();
    }

    auto& lToken = dynamic_cast<blind::token::implementation::Lucre&>(token);
    crypto::implementation::OpenSSL_BIO bioBank = BIO_new(BIO_s_mem());
    crypto::implementation::OpenSSL_BIO bioRequest = BIO_new(BIO_s_mem());
    crypto::implementation::OpenSSL_BIO bioSignature = BIO_new(BIO_s_mem());

    auto armoredPrivate = Armored::Factory();

    if (false == GetPrivate(armoredPrivate, lToken.Value())) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load private key")
            .Flush();

        return false;
    } else {
        LogInsane(OT_METHOD)(__FUNCTION__)(": Loaded private mint key").Flush();
    }

    OTEnvelope envelope(armoredPrivate);
    auto privateKey = String::Factory();

    if (false == envelope.Open(notary, privateKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt private key")
            .Flush();

        return false;
    } else {
        LogInsane(OT_METHOD)(__FUNCTION__)(": Decrypted private mint key")
            .Flush();
    }

    BIO_puts(bioBank, privateKey->Get());
    Bank bank(bioBank);
    auto prototoken = String::Factory();

    if (false == lToken.GetPublicPrototoken(prototoken)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to extract prototoken")
            .Flush();

        return false;
    } else {
        LogInsane(OT_METHOD)(__FUNCTION__)(": Extracted prototoken").Flush();
    }

    BIO_puts(bioRequest, prototoken->Get());
    PublicCoinRequest req(bioRequest);
    BIGNUM* bnSignature = bank.SignRequest(req);

    if (nullptr == bnSignature) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to sign prototoken")
            .Flush();

        return false;
    } else {
        LogInsane(OT_METHOD)(__FUNCTION__)(": Signed prototoken").Flush();
    }

    req.WriteBIO(bioSignature);
    DumpNumber(bioSignature, "signature=", bnSignature);
    BN_free(bnSignature);
    char sig_buf[1024]{};
    auto sig_len = BIO_read(bioSignature, sig_buf, 1023);
    sig_buf[sig_len] = '\0';

    if (0 == sig_len) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to copy signature")
            .Flush();

        return false;
    } else {
        LogInsane(OT_METHOD)(__FUNCTION__)(": Signature copied").Flush();
    }

    auto signature = String::Factory(sig_buf);

    if (false == lToken.AddSignature(signature)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to set signature").Flush();

        return false;
    } else {
        LogInsane(OT_METHOD)(__FUNCTION__)(": Signature serialized").Flush();
    }

    return true;
}

bool Lucre::VerifyToken(const Nym& notary, const blind::Token& token)
{

    if (proto::CASHTYPE_LUCRE != token.Type()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect token type").Flush();

        return false;
    }

    const auto& lucreToken =
        dynamic_cast<const blind::token::implementation::Lucre&>(token);

#if OT_LUCRE_DEBUG
    LucreDumper setDumper;
#endif

    crypto::implementation::OpenSSL_BIO bioBank = BIO_new(BIO_s_mem());
    crypto::implementation::OpenSSL_BIO bioCoin = BIO_new(BIO_s_mem());
    auto spendable = String::Factory();

    if (false == lucreToken.GetSpendable(spendable)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to extract").Flush();

        return false;
    }

    BIO_puts(bioCoin, spendable->Get());
    auto armoredPrivate = Armored::Factory();
    GetPrivate(armoredPrivate, token.Value());
    OTEnvelope envelope(armoredPrivate);
    auto privateKey = String::Factory();

    if (false == envelope.Open(notary, privateKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to decrypt private mint key")
            .Flush();

        return false;
    }

    BIO_puts(bioBank, privateKey->Get());
    Bank bank(bioBank);
    Coin coin(bioCoin);

    return bank.Verify(coin);
}

#endif  // OT_CRYPTO_USING_OPENSSL
}  // namespace opentxs::blind::mint::implementation
#endif
