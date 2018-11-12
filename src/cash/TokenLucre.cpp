// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/cash/TokenLucre.hpp"

#include "opentxs/cash/DigitalCash.hpp"
#include "opentxs/cash/Mint.hpp"
#include "opentxs/cash/Token.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"

#if OT_CASH_USING_LUCRE
#include "crypto/library/OpenSSL_BIO.hpp"
#endif

#include <openssl/bio.h>
#include <openssl/ossl_typ.h>
#include <sys/types.h>
#include <ostream>

#define OT_METHOD "opentxs::TokenLucre::"

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

// SUBCLASSES OF OTTOKEN FOR EACH DIGITAL CASH ALGORITHM.

#if OT_CASH_USING_MAGIC_MONEY
// Todo:  Someday...
#endif  // Magic Money

namespace opentxs
{
#if OT_CASH_USING_LUCRE && OT_CRYPTO_USING_OPENSSL

Token_Lucre::Token_Lucre(const api::Core& core)
    : ot_super(core)
{
}

Token_Lucre::Token_Lucre(
    const api::Core& core,
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID)
    : ot_super(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID)
{
}

Token_Lucre::Token_Lucre(const api::Core& core, const Purse& thePurse)
    : ot_super(core, thePurse)
{
}

// Lucre step 2 (client generates coin request)
// nDenomination must be one of the denominations supported by the mint.
// sets m_nTokenCount and populates the maps with prototokens (in ASCII-armored
// format.)
bool Token_Lucre::GenerateTokenRequest(
    const Nym& theNym,
    Mint& theMint,
    std::int64_t lDenomination,
    std::int32_t nTokenCount)
{
    //    otErr << "%s <bank public info> <coin request private output file>
    // <coin request public output file>\n", argv[0]);
    //
    if (Token::blankToken != m_State) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Blank token expected.").Flush();
        return false;
    }

    LucreDumper setDumper;  // todo security.

    crypto::implementation::OpenSSL_BIO bioBank =
        BIO_new(BIO_s_mem());  // Input. We must supply the
                               // bank's public lucre info

    // This version base64-DECODES the ascii-armored string passed in,
    // and then sets the decoded plaintext string onto the string.
    // OTString::OTString(const Armored & strValue)
    auto ascPublicMint = Armored::Factory();

    theMint.GetPublic(ascPublicMint, lDenomination);
    //    otErr << "DEBUG: OTToken  public asc: \n%s\n", ascPublicMint.Get());

    auto strPublicMint = String::Factory(ascPublicMint);
    //    otErr << "DEBUG: OTToken  public str: \n%s\n", strPublicMint.Get());

    // Get the bank's public key (now decoded in strPublicMint)
    // and put it into bioBank so we can use it with Lucre.
    BIO_puts(bioBank, strPublicMint->Get());

    // Instantiate a PublicBank (Lucre) object.
    // We will use it to generate all the prototokens in the loop below.
    PublicBank bank;
    bank.ReadBIO(bioBank);

    Release();  // note: why is this here? I guess to release the prototokens,
                // the signature (is there one?) and m_ascSpendable (exists?
                // doubt it.) This WAS also doing "InitToken" (no longer) which
    // WAS setting series and expiration range back to 0 (no longer.)
    // Which was causing problems for all series above 0. I'm leaving
    // this call here, to do the stuff I guess it was put here for.
    // But things such as the series, expiration date range, and
    // token count, etc are no longer (inadvertantly) set to 0 here
    // on this line. I'm also moving the SetSeriesAndExpiration call
    // to be BELOW this line, since it's not apparently needed above
    // this line anyway.

    // We are supposed to set these values here.
    // The server actually sets them again, for security reasons.
    // But we should still set them since server may choose to reject the
    // request.
    // Update: the series information on this token IS used by the server, since
    // more than
    // one mint may be currently valid, and since the server has to process the
    // request using
    // the proper mint, corresponding to the same mint that was used to GENERATE
    // that request.
    // So the server uses the series value from this token in order to choose
    // which mint is loaded,
    // on the server side. When this call WAS above the Release() call above,
    // these values were
    // getting wiped in Release back to 0. So I've moved it below the Release()
    // call. (I've also
    // stopped wiping them in Release.)
    //
    SetSeriesAndExpiration(
        theMint.GetSeries(), theMint.GetValidFrom(), theMint.GetValidTo());

    const std::int32_t nFinalTokenCount =
        (nTokenCount < Token::GetMinimumPrototokenCount())
            ? Token::GetMinimumPrototokenCount()
            : nTokenCount;

    // Token count is actually 1 (always) with Lucre, although this lib has
    // potential to work with
    // multiple proto-tokens, you can see this loop as though it always executes
    // just once.
    for (std::int32_t i = 0; i < nFinalTokenCount; i++) {
        crypto::implementation::OpenSSL_BIO bioCoin =
            BIO_new(BIO_s_mem());  // These two are output. We
                                   // must write these bios,
                                   // after
        crypto::implementation::OpenSSL_BIO bioPublicCoin = BIO_new(
            BIO_s_mem());  // the operation, back into some form we can use

        CoinRequest req(bank);

        // write the private coin request to BIO
        req.WriteBIO(bioCoin);

        // write the public coin request to BIO
        static_cast<PublicCoinRequest*>(&req)->WriteBIO(bioPublicCoin);

        // Convert the two bios to our format
        char privateCoinBuffer[4096],
            publicCoinBuffer[4096];  // todo stop hardcoding these string
                                     // lengths
        std::int32_t privatecoinLen = BIO_read(
            bioCoin,
            privateCoinBuffer,
            4000);  // cutting it a little short on
                    // purpose, with the buffer.
                    // Just makes me feel more
                    // comfortable for some reason.
        std::int32_t publiccoinLen =
            BIO_read(bioPublicCoin, publicCoinBuffer, 4000);

        if (privatecoinLen && publiccoinLen) {
            // With this, we have the Lucre public and private bank info
            // converted to OTStrings
            auto strPublicCoin = String::Factory();
            strPublicCoin->Set(publicCoinBuffer, publiccoinLen);
            auto strPrivateCoin = String::Factory();
            strPrivateCoin->Set(privateCoinBuffer, privatecoinLen);

            auto pArmoredPublic = Armored::Factory(strPublicCoin);
            auto pArmoredPrivate = Armored::Factory();

            // Change the state. It's no longer a blank token, but a prototoken.
            m_State = Token::protoToken;

            // Seal the private coin info up into an encrypted Envelope
            // and set it onto pArmoredPrivate (which was just added to our
            // internal map, above.)
            OTEnvelope theEnvelope;
            theEnvelope.Seal(theNym, strPrivateCoin);  // Todo check the return
                                                       // values on these two
                                                       // functions
            theEnvelope.GetCiphertext(pArmoredPrivate);

            m_mapPublic.emplace(i, std::move(pArmoredPublic));
            m_mapPublic.emplace(i, std::move(pArmoredPrivate));
            m_nTokenCount = nFinalTokenCount;
            SetDenomination(lDenomination);
        } else {
            // Error condition todo
        }
    }

    return true;
}

// Lucre step 4: client unblinds token -- now it's ready for use.
// Final unblinded spendable token is encrypted to theNym for safe storage.
//
bool Token_Lucre::ProcessToken(
    const Nym& theNym,
    Mint& theMint,
    Token& theRequest)
{
    //    otErr << "%s <bank public info> <private coin request> <signed coin
    // request> <coin>\n",
    bool bReturnValue = false;

    // When the Mint has signed a token and sent it back to the client,
    // the client must unblind the token and set it as spendable. Thus,
    // this function is only performed on tokens in the signedToken state.
    if (Token::signedToken != m_State) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Signed token expected.")
            .Flush();
        return false;
    }

    // Lucre
    LucreDumper setDumper;  // todo security.

    crypto::implementation::OpenSSL_BIO bioBank =
        BIO_new(BIO_s_mem());  // input
    crypto::implementation::OpenSSL_BIO bioSignature =
        BIO_new(BIO_s_mem());  // input
    crypto::implementation::OpenSSL_BIO bioPrivateRequest =
        BIO_new(BIO_s_mem());  // input
    crypto::implementation::OpenSSL_BIO bioCoin =
        BIO_new(BIO_s_mem());  // output

    // Get the bank's public key (decoded into strPublicMint)
    // and put it into bioBank so we can use it with Lucre.
    //
    auto ascPublicMint = Armored::Factory();
    theMint.GetPublic(ascPublicMint, GetDenomination());
    auto strPublicMint = String::Factory(ascPublicMint);
    BIO_puts(bioBank, strPublicMint->Get());

    // Get the existing signature into a bio.
    //    otErr << "DEBUGGING, m_Signature: -------------%s--------------\n",
    // m_Signature.Get());
    auto strSignature = String::Factory(m_Signature);
    BIO_puts(bioSignature, strSignature->Get());

    // I need the Private coin request also. (Only the client has this private
    // coin request data.)
    auto thePrototoken = Armored::Factory();  // The server sets m_nChosenIndex
                                              // when it signs the token.
    bool bFoundToken =
        theRequest.GetPrivatePrototoken(thePrototoken, m_nChosenIndex);

    if (bFoundToken) {
        //        otErr << "THE PRIVATE REQUEST ARMORED
        // CONTENTS:\n------------------>%s<-----------------------\n",
        //                thePrototoken.Get());

        // Decrypt the prototoken
        auto strPrototoken = String::Factory();
        OTEnvelope theEnvelope(thePrototoken);
        theEnvelope.Open(theNym, strPrototoken);  // todo check return value.

        //        otErr << "THE PRIVATE REQUEST
        // CONTENTS:\n------------------>%s<-----------------------\n",
        //                strPrototoken.Get());

        // copy strPrototoken to a BIO
        BIO_puts(bioPrivateRequest, strPrototoken->Get());

        // ------- Okay, the BIOs are all loaded.... let's process...

        PublicBank bank(bioBank);
        CoinRequest req(bioPrivateRequest);

        // TODO make sure I'm not leaking memory with these ReadNumbers
        // Probably need to be calling some free function for each one.

        // Apparently reading the request id here and then just discarding it...
        ReadNumber(bioSignature, "request=");

        // Versus the signature data, which is read into bnSignature apparently.
        BIGNUM* bnSignature = ReadNumber(bioSignature, "signature=");
        DumpNumber("signature=", bnSignature);

        // Produce the final unblinded token in Coin coin, and write it to
        // bioCoin...
        Coin coin;  // Coin Request, processes into Coin, with Bank and
                    // Signature
                    // passed in.
        req.ProcessResponse(&coin, bank, bnSignature);  // Notice still
                                                        // apparently "request"
                                                        // info is discarded.
        coin.WriteBIO(bioCoin);

        // convert bioCoin to a C-style string...
        char CoinBuffer[1024];  // todo stop hardcoding these string lengths
        std::int32_t coinLen = BIO_read(
            bioCoin, CoinBuffer, 1000);  // cutting it a little short on
                                         // purpose, with the buffer.
                                         // Just makes me feel more
                                         // comfortable for some reason.

        if (coinLen) {
            // ...to OTString...
            auto strCoin = String::Factory();
            strCoin->Set(CoinBuffer, coinLen);

            //            otErr << "Processing token...\n%s\n", strCoin.Get());

            // ...to Envelope stored in m_ascSpendable (encrypted and
            // base64-encoded)
            OTEnvelope envelope;
            // Todo check the return values on these two functions
            envelope.Seal(theNym, strCoin);
            // Here's the final product.
            envelope.GetCiphertext(m_ascSpendable);

            //            otErr << "NEW SPENDABLE
            // token...\n--------->%s<----------------\n",
            // m_ascSpendable.Get());

            // Now the coin is encrypted from here on out, and otherwise
            // ready-to-spend.
            m_State = Token::spendableToken;
            bReturnValue = true;

            // Lastly, we free the signature data, which is no longer needed,
            // and which could be
            // otherwise used to trace the token. (Which we don't want.)
            m_Signature->Release();
        }
    }
    // Todo log error here if the private prototoken is not found. (Very strange
    // if so!!)
    //  else {}

    return bReturnValue;
}

#endif  // OT_CASH_USING_LUCRE && OT_CRYPTO_USING_OPENSSL

}  // namespace opentxs
