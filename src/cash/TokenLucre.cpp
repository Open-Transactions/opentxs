/************************************************************
 *
 *  TokenLucre.cpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

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
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#include <opentxs/core/stdafx.hpp>

#include <opentxs/cash/TokenLucre.hpp>
#include <opentxs/cash/DigitalCash.hpp>
#include <opentxs/cash/Mint.hpp>
#include <opentxs/cash/Token.hpp>

#include <opentxs/core/crypto/OTEnvelope.hpp>

#if defined(OT_CASH_USING_LUCRE)
#include <opentxs/core/crypto/OpenSSL_BIO.hpp>
#endif

#include <opentxs/core/OTLog.hpp>

// SUBCLASSES OF OTTOKEN FOR EACH DIGITAL CASH ALGORITHM.

#if defined(OT_CASH_USING_MAGIC_MONEY)
// Todo:  Someday...
#endif // Magic Money

namespace opentxs
{

#if defined(OT_CASH_USING_LUCRE)

Token_Lucre::Token_Lucre()
    : ot_super()
{
}

Token_Lucre::Token_Lucre(const OTIdentifier& SERVER_ID,
                         const OTIdentifier& ASSET_ID)
    : ot_super(SERVER_ID, ASSET_ID)
{
}

Token_Lucre::Token_Lucre(const Purse& thePurse)
    : ot_super(thePurse)
{
}

Token_Lucre::~Token_Lucre()
{
}

// Lucre step 2 (client generates coin request)
// nDenomination must be one of the denominations supported by the mint.
// sets m_nTokenCount and populates the maps with prototokens (in ASCII-armored
// format.)
bool Token_Lucre::GenerateTokenRequest(const OTPseudonym& theNym, Mint& theMint,
                                       int64_t lDenomination,
                                       int32_t nTokenCount)
{
    //    otErr << "%s <bank public info> <coin request private output file>
    // <coin request public output file>\n", argv[0]);
    //
    if (Token::blankToken != m_State) {
        otErr << "Token_Lucre::GenerateTokenRequest: Blank token expected.\n";
        return false;
    }

    LucreDumper setDumper; // todo security.

    OpenSSL_BIO bioBank = BIO_new(
        BIO_s_mem()); // Input. We must supply the bank's public lucre info

    // This version base64-DECODES the ascii-armored string passed in,
    // and then sets the decoded plaintext string onto the string.
    // OTString::OTString(const OTASCIIArmor & strValue)
    OTASCIIArmor ascPublicMint;

    theMint.GetPublic(ascPublicMint, lDenomination);
    //    otErr << "DEBUG: OTToken  public asc: \n%s\n", ascPublicMint.Get());

    OTString strPublicMint(ascPublicMint);
    //    otErr << "DEBUG: OTToken  public str: \n%s\n", strPublicMint.Get());

    // Get the bank's public key (now decoded in strPublicMint)
    // and put it into bioBank so we can use it with Lucre.
    BIO_puts(bioBank, strPublicMint.Get());

    // Instantiate a PublicBank (Lucre) object.
    // We will use it to generate all the prototokens in the loop below.
    PublicBank bank;
    bank.ReadBIO(bioBank);

    Release(); // note: why is this here? I guess to release the prototokens,
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
    SetSeriesAndExpiration(theMint.GetSeries(), theMint.GetValidFrom(),
                           theMint.GetValidTo());

    const int32_t nFinalTokenCount =
        (nTokenCount < Token::GetMinimumPrototokenCount())
            ? Token::GetMinimumPrototokenCount()
            : nTokenCount;

    // Token count is actually 1 (always) with Lucre, although this lib has
    // potential to work with
    // multiple proto-tokens, you can see this loop as though it always executes
    // just once.
    for (int32_t i = 0; i < nFinalTokenCount; i++) {
        OpenSSL_BIO bioCoin = BIO_new(BIO_s_mem()); // These two are output. We
                                                    // must write these bios,
                                                    // after
        OpenSSL_BIO bioPublicCoin = BIO_new(
            BIO_s_mem()); // the operation, back into some form we can use

        CoinRequest req(bank);

        // write the private coin request to BIO
        req.WriteBIO(bioCoin);

        // write the public coin request to BIO
        static_cast<PublicCoinRequest*>(&req)->WriteBIO(bioPublicCoin);

        // Convert the two bios to our format
        char privateCoinBuffer[4096],
            publicCoinBuffer[4096]; // todo stop hardcoding these string lengths
        int32_t privatecoinLen = BIO_read(bioCoin, privateCoinBuffer,
                                          4000); // cutting it a little short on
                                                 // purpose, with the buffer.
                                                 // Just makes me feel more
                                                 // comfortable for some reason.
        int32_t publiccoinLen = BIO_read(bioPublicCoin, publicCoinBuffer, 4000);

        if (privatecoinLen && publiccoinLen) {
            // With this, we have the Lucre public and private bank info
            // converted to OTStrings
            OTString strPublicCoin;
            strPublicCoin.Set(publicCoinBuffer, publiccoinLen);
            OTString strPrivateCoin;
            strPrivateCoin.Set(privateCoinBuffer, privatecoinLen);

            OTASCIIArmor* pArmoredPublic = new OTASCIIArmor(strPublicCoin);
            OTASCIIArmor* pArmoredPrivate = new OTASCIIArmor();

            OT_ASSERT_MSG(
                ((nullptr != pArmoredPublic) && (nullptr != pArmoredPrivate)),
                "ERROR: Unable to allocate memory in "
                "Token_Lucre::GenerateTokenRequest\n");

            // Change the state. It's no longer a blank token, but a prototoken.
            m_State = Token::protoToken;

            // Seal the private coin info up into an encrypted Envelope
            // and set it onto pArmoredPrivate (which was just added to our
            // internal map, above.)
            OTEnvelope theEnvelope;
            theEnvelope.Seal(theNym, strPrivateCoin); // Todo check the return
                                                      // values on these two
                                                      // functions
            theEnvelope.GetAsciiArmoredData(*pArmoredPrivate);

            m_mapPublic[i] = pArmoredPublic;
            m_mapPrivate[i] = pArmoredPrivate;

            m_nTokenCount = nFinalTokenCount;
            SetDenomination(lDenomination);
        }
        else {
            // Error condition todo
        }
    }

    return true;
}

// Lucre step 4: client unblinds token -- now it's ready for use.
// Final unblinded spendable token is encrypted to theNym for safe storage.
//
bool Token_Lucre::ProcessToken(const OTPseudonym& theNym, Mint& theMint,
                               Token& theRequest)
{
    //    otErr << "%s <bank public info> <private coin request> <signed coin
    // request> <coin>\n",
    bool bReturnValue = false;

    // When the Mint has signed a token and sent it back to the client,
    // the client must unblind the token and set it as spendable. Thus,
    // this function is only performed on tokens in the signedToken state.
    if (Token::signedToken != m_State) {
        otErr << "Signed token expected in Token_Lucre::ProcessToken\n";
        return false;
    }

    // Lucre
    LucreDumper setDumper; // todo security.

    OpenSSL_BIO bioBank = BIO_new(BIO_s_mem());           // input
    OpenSSL_BIO bioSignature = BIO_new(BIO_s_mem());      // input
    OpenSSL_BIO bioPrivateRequest = BIO_new(BIO_s_mem()); // input
    OpenSSL_BIO bioCoin = BIO_new(BIO_s_mem());           // output

    // Get the bank's public key (decoded into strPublicMint)
    // and put it into bioBank so we can use it with Lucre.
    //
    OTASCIIArmor ascPublicMint;
    theMint.GetPublic(ascPublicMint, GetDenomination());
    OTString strPublicMint(ascPublicMint);
    BIO_puts(bioBank, strPublicMint.Get());

    // Get the existing signature into a bio.
    //    otErr << "DEBUGGING, m_Signature: -------------%s--------------\n",
    // m_Signature.Get());
    OTString strSignature(m_Signature);
    BIO_puts(bioSignature, strSignature.Get());

    // I need the Private coin request also. (Only the client has this private
    // coin request data.)
    OTASCIIArmor thePrototoken; // The server sets m_nChosenIndex when it signs
                                // the token.
    bool bFoundToken =
        theRequest.GetPrivatePrototoken(thePrototoken, m_nChosenIndex);

    if (bFoundToken) {
        //        otErr << "THE PRIVATE REQUEST ARMORED
        // CONTENTS:\n------------------>%s<-----------------------\n",
        //                thePrototoken.Get());

        // Decrypt the prototoken
        OTString strPrototoken;
        OTEnvelope theEnvelope(thePrototoken);
        theEnvelope.Open(theNym, strPrototoken); // todo check return value.

        //        otErr << "THE PRIVATE REQUEST
        // CONTENTS:\n------------------>%s<-----------------------\n",
        //                strPrototoken.Get());

        // copy strPrototoken to a BIO
        BIO_puts(bioPrivateRequest, strPrototoken.Get());

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
        Coin coin; // Coin Request, processes into Coin, with Bank and Signature
                   // passed in.
        req.ProcessResponse(&coin, bank, bnSignature); // Notice still
                                                       // apparently "request"
                                                       // info is discarded.
        coin.WriteBIO(bioCoin);

        // convert bioCoin to a C-style string...
        char CoinBuffer[1024]; // todo stop hardcoding these string lengths
        int32_t coinLen =
            BIO_read(bioCoin, CoinBuffer, 1000); // cutting it a little short on
                                                 // purpose, with the buffer.
                                                 // Just makes me feel more
                                                 // comfortable for some reason.

        if (coinLen) {
            // ...to OTString...
            OTString strCoin;
            strCoin.Set(CoinBuffer, coinLen);

            //            otErr << "Processing token...\n%s\n", strCoin.Get());

            // ...to Envelope stored in m_ascSpendable (encrypted and
            // base64-encoded)
            OTEnvelope theEnvelope;
            theEnvelope.Seal(theNym, strCoin); // Todo check the return values
                                               // on these two functions
            theEnvelope.GetAsciiArmoredData(
                m_ascSpendable); // Here's the final product.

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
            m_Signature.Release();
        }
    }
    // Todo log error here if the private prototoken is not found. (Very strange
    // if so!!)
    //  else {}

    return bReturnValue;
}

#endif // defined(OT_CASH_USING_LUCRE)

} // namespace opentxs
