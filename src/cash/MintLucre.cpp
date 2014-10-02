/************************************************************
 *
 *  MintLucre.cpp
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

#include <opentxs/cash/MintLucre.hpp>
#include <opentxs/cash/DigitalCash.hpp>
#include <opentxs/cash/Token.hpp>

#include <opentxs/core/crypto/OTAsymmetricKey.hpp>
#include <opentxs/core/crypto/OTEnvelope.hpp>

#if defined(OT_CASH_USING_LUCRE)
#include <opentxs/core/crypto/OpenSSL_BIO.hpp>
#endif

#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTPseudonym.hpp>

namespace opentxs
{

#if defined(OT_CASH_USING_LUCRE)

MintLucre::MintLucre()
    : ot_super()
{
}

MintLucre::MintLucre(const OTString& strServerID,
                     const OTString& strAssetTypeID)
    : ot_super(strServerID, strAssetTypeID)
{
}

MintLucre::MintLucre(const OTString& strServerID,
                     const OTString& strServerNymID,
                     const OTString& strAssetTypeID)
    : ot_super(strServerID, strServerNymID, strAssetTypeID)
{
}

MintLucre::~MintLucre()
{
}

// The mint has a different key pair for each denomination.
// Pass the actual denomination such as 5, 10, 20, 50, 100...
bool MintLucre::AddDenomination(OTPseudonym& theNotary, int64_t lDenomination,
                                int32_t nPrimeLength)
{
    OT_ASSERT(nullptr != m_pKeyPublic);

    bool bReturnValue = false;

    // Let's make sure it doesn't already exist
    OTASCIIArmor theArmor;
    if (GetPublic(theArmor, lDenomination)) {
        otErr << "Error: Denomination public already exists in "
                 "OTMint::AddDenomination\n";
        return false;
    }
    if (GetPrivate(theArmor, lDenomination)) {
        otErr << "Error: Denomination private already exists in "
                 "OTMint::AddDenomination\n";
        return false;
    }

    if ((nPrimeLength / 8) < (MIN_COIN_LENGTH + DIGEST_LENGTH)) {
        otErr << "Prime must be at least "
              << (MIN_COIN_LENGTH + DIGEST_LENGTH) * 8 << " bits\n";
        return false;
    }

    if (nPrimeLength % 8) {
        otErr << "Prime length must be a multiple of 8\n";
        return false;
    }

#ifdef _WIN32
    BIO* out = BIO_new_file("openssl.dump", "w");
    assert(out);
    SetDumper(out);
#else
    SetMonitor(stderr);
#endif

    OpenSSL_BIO bio = BIO_new(BIO_s_mem());
    OpenSSL_BIO bioPublic = BIO_new(BIO_s_mem());

    // Generate the mint private key information
    Bank bank(nPrimeLength / 8);
    bank.WriteBIO(bio);

    // Generate the mint public key information
    PublicBank pbank(bank);
    pbank.WriteBIO(bioPublic);

    // Copy from BIO back to a normal OTString or Ascii-Armor
    char privateBankBuffer[4096],
        publicBankBuffer[4096]; // todo stop hardcoding these string lengths
    int32_t privatebankLen =
        BIO_read(bio, privateBankBuffer, 4000); // cutting it a little short on
                                                // purpose, with the buffer.
    int32_t publicbankLen =
        BIO_read(bioPublic, publicBankBuffer,
                 4000); // Just makes me feel more comfortable for some reason.

    if (privatebankLen && publicbankLen) {
        // With this, we have the Lucre public and private bank info converted
        // to OTStrings
        OTString strPublicBank;
        strPublicBank.Set(publicBankBuffer, publicbankLen);
        OTString strPrivateBank;
        strPrivateBank.Set(privateBankBuffer, privatebankLen);

        OTASCIIArmor* pPublic = new OTASCIIArmor();
        OTASCIIArmor* pPrivate = new OTASCIIArmor();

        OT_ASSERT(nullptr != pPublic);
        OT_ASSERT(nullptr != pPrivate);

        // Set the public bank info onto pPublic
        pPublic->SetString(strPublicBank, true); // linebreaks = true

        // Seal the private bank info up into an encrypted Envelope
        // and set it onto pPrivate
        OTEnvelope theEnvelope;
        theEnvelope.Seal(theNotary, strPrivateBank); // Todo check the return
                                                     // values on these two
                                                     // functions
        theEnvelope.GetAsciiArmoredData(*pPrivate);

        // Add the new key pair to the maps, using denomination as the key
        m_mapPublic[lDenomination] = pPublic;
        m_mapPrivate[lDenomination] = pPrivate;

        // Grab the Server Nym ID and save it with this Mint
        theNotary.GetIdentifier(m_ServerNymID);

        // Grab the Server's public key and save it with this Mint
        //
        const OTAsymmetricKey& theNotaryPubKey = theNotary.GetPublicSignKey();
        delete m_pKeyPublic;
        m_pKeyPublic = theNotaryPubKey.ClonePubKey();

        m_nDenominationCount++;
        bReturnValue = true;
        otWarn << "Successfully added denomination: " << lDenomination << "\n";
    }

    return bReturnValue;
}

// Lucre step 3: the mint signs the token
//
bool MintLucre::SignToken(OTPseudonym& theNotary, Token& theToken,
                          OTString& theOutput, int32_t nTokenIndex)
{
    bool bReturnValue = false;

    LucreDumper setDumper;

    OpenSSL_BIO bioBank = BIO_new(BIO_s_mem());      // input
    OpenSSL_BIO bioRequest = BIO_new(BIO_s_mem());   // input
    OpenSSL_BIO bioSignature = BIO_new(BIO_s_mem()); // output

    OTASCIIArmor thePrivate;
    GetPrivate(thePrivate, theToken.GetDenomination());

    // The Mint private info is encrypted in
    // m_mapPrivates[theToken.GetDenomination()].
    // So I need to extract that first before I can use it.
    OTEnvelope theEnvelope(thePrivate);

    OTString strContents; // output from opening the envelope.
    // Decrypt the Envelope into strContents
    if (!theEnvelope.Open(theNotary, strContents)) return false;

    // copy strContents to a BIO
    BIO_puts(bioBank, strContents.Get());

    // Instantiate the Bank with its private key
    Bank bank(bioBank);

    // I need the request. the prototoken.
    OTASCIIArmor ascPrototoken;
    bool bFoundToken = theToken.GetPrototoken(ascPrototoken, nTokenIndex);

    if (bFoundToken) {
        // base64-Decode the prototoken
        OTString strPrototoken(ascPrototoken);

        // copy strPrototoken to a BIO
        BIO_puts(bioRequest, strPrototoken.Get());

        // Load up the coin request from the bio (the prototoken)
        PublicCoinRequest req(bioRequest);

        // Sign it with the bank we previously instantiated.
        // results will be in bnSignature (BIGNUM)
        BIGNUM* bnSignature = bank.SignRequest(req);

        if (nullptr == bnSignature) {
            otErr << "MAJOR ERROR!: Bank.SignRequest failed in "
                     "MintLucre::SignToken\n";
        }
        else {

            // Write the request contents, followed by the signature contents,
            // to the Signature bio. Then free the BIGNUM.
            req.WriteBIO(bioSignature); // the original request contents
            DumpNumber(bioSignature, "signature=",
                       bnSignature); // the new signature contents
            BN_free(bnSignature);

            // Read the signature bio into a C-style buffer...
            char sig_buf[1024]; // todo stop hardcoding these string lengths

            int32_t sig_len = BIO_read(bioSignature, sig_buf,
                                       1000); // cutting it a little short on
                                              // purpose, with the buffer. Just
                                              // makes me feel more comfortable
                                              // for some reason.

            // Add the null terminator by hand (just in case.)
            sig_buf[sig_len] = '\0';

            if (sig_len) {
                // Copy the original coin request into the spendable field of
                // the token object.
                // (It won't actually be spendable until the client processes
                // it, though.)
                theToken.SetSpendable(ascPrototoken);

                // Base64-encode the signature contents into
                // theToken.m_Signature.
                OTString strSignature(sig_buf);

                // Here we pass the signature back to the caller.
                // He will probably set it onto the token.
                theOutput.Set(sig_buf, sig_len);
                bReturnValue = true;

                // This is also where we set the expiration date on the token.
                // The client should have already done this, but we are
                // explicitly
                // setting the values here to prevent any funny business.
                theToken.SetSeriesAndExpiration(m_nSeries, m_VALID_FROM,
                                                m_VALID_TO);
            }
        }
    }

    return bReturnValue;
}

// Lucre step 5: mint verifies token when it is redeemed by merchant.
// This function is called by OTToken::VerifyToken.
// That's the one you should be calling, most likely, not this one.
bool MintLucre::VerifyToken(OTPseudonym& theNotary, OTString& theCleartextToken,
                            int64_t lDenomination)
{
    bool bReturnValue = false;
    LucreDumper setDumper;

    OpenSSL_BIO bioBank = BIO_new(BIO_s_mem()); // input
    OpenSSL_BIO bioCoin = BIO_new(BIO_s_mem()); // input

    // --- copy theCleartextToken to bioCoin so lucre can load it
    BIO_puts(bioCoin, theCleartextToken.Get());

    // --- The Mint private info is encrypted in m_mapPrivate[lDenomination].
    // So I need to extract that first before I can use it.
    OTASCIIArmor theArmor;
    GetPrivate(theArmor, lDenomination);
    OTEnvelope theEnvelope(theArmor);

    OTString strContents; // will contain output from opening the envelope.
    // Decrypt the Envelope into strContents
    if (theEnvelope.Open(theNotary, strContents)) {
        // copy strContents to a BIO
        BIO_puts(bioBank, strContents.Get());

        // ---- Now the bank and coin bios are both ready to go...

        Bank bank(bioBank);
        Coin coin(bioCoin);

        if (bank.Verify(coin)) // Here's the boolean output: coin is verified!
        {
            bReturnValue = true;

            // (Done): When a token is redeemed, need to store it in the spent
            // token database.
            // Right now I can verify the token, but unless I check it against a
            // database, then
            // even though the signature verifies, it doesn't stop people from
            // redeeming the same
            // token again and again and again.
            //
            // (done): also need to make sure issuer has double-entries for
            // total amount outstanding.
            //
            // UPDATE: These are both done now.  The Spent Token database is
            // implemented in the transaction server,
            // (not OTLib proper) and the same server also now keeps a cash
            // account to match all cash withdrawals.
            // (Meaning, if 10,000 clams total have been withdrawn by various
            // users, then the server actually has
            // a clam account containing 10,000 clams. As the cash comes in for
            // redemption, the server debits it from
            // this account again before sending it to its final destination.
            // This way the server tracks total outstanding
            // amount, as an additional level of security after the blind
            // signature itself.)
        }
    }

    return bReturnValue;
}

} // namespace opentxs
