/************************************************************
 *
 *  OTCrypto.hpp
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

#ifndef OPENTXS_CORE_CRYPTO_OTCRYPTO_HPP
#define OPENTXS_CORE_CRYPTO_OTCRYPTO_HPP

#include <opentxs/core/OTData.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/util/Assert.hpp>

#include <mutex>

#include <set>

namespace opentxs
{

class OTAsymmetricKey;
class Identifier;
class OTPassword;
class OTPasswordData;
class OTPseudonym;
class OTSettings;
class OTSignature;

typedef std::multimap<std::string, OTAsymmetricKey*> mapOfAsymmetricKeys;

class OTCryptoConfig
{
private:
    static bool GetSetAll();

    static bool GetSetValue(OTSettings& config, std::string strKeyName,
                            int32_t nDefaultValue, const int32_t*& out_nValue);

    static const int32_t& GetValue(const int32_t*& pValue);

    static const int32_t* sp_nIterationCount;
    static const int32_t* sp_nSymmetricSaltSize;
    static const int32_t* sp_nSymmetricKeySize;
    static const int32_t* sp_nSymmetricKeySizeMax;
    static const int32_t* sp_nSymmetricIvSize;
    static const int32_t* sp_nSymmetricBufferSize;
    static const int32_t* sp_nPublicKeysize;
    static const int32_t* sp_nPublicKeysizeMax;

public:
    EXPORT static uint32_t IterationCount();
    EXPORT static uint32_t SymmetricSaltSize();
    EXPORT static uint32_t SymmetricKeySize();
    EXPORT static uint32_t SymmetricKeySizeMax();
    EXPORT static uint32_t SymmetricIvSize();
    EXPORT static uint32_t SymmetricBufferSize();
    EXPORT static uint32_t PublicKeysize();
    EXPORT static uint32_t PublicKeysizeMax();
};

// Sometimes I want to decrypt into an OTPassword (for encrypted symmetric
// keys being decrypted) and sometimes I want to decrypt into an OTData
// (For most other types of data.) This class allows me to do it either way
// without duplicating the static Decrypt() function, by wrapping both
// types.
//
class OTCrypto_Decrypt_Output
{
private:
    OTPassword* m_pPassword;
    ot_data_t* m_pPayload;

    OTCrypto_Decrypt_Output();

public:
    EXPORT ~OTCrypto_Decrypt_Output();

    EXPORT OTCrypto_Decrypt_Output(const OTCrypto_Decrypt_Output& rhs);

    EXPORT OTCrypto_Decrypt_Output(OTPassword& thePassword);
    EXPORT OTCrypto_Decrypt_Output(ot_data_t& thePayload);

    EXPORT void swap(OTCrypto_Decrypt_Output& other);

    EXPORT OTCrypto_Decrypt_Output& operator=(
        OTCrypto_Decrypt_Output other); // passed by value.

    EXPORT bool Concatenate(const void* pAppendData,
                            uint32_t lAppendSize) const;

    EXPORT void Release(); // Someday make this virtual, if we ever subclass it.
    EXPORT void Release_Envelope_Decrypt_Output() const;
};

// OT CRYPTO -- ABSTRACT INTERFACE
//
// We are now officially at the point where we can easily swap crypto libs!
// Just make a subclass of OTCrypto (copy an existing subclass such as
// OTCrypto_OpenSSL)
class OTCrypto
{
private:
    static int32_t s_nCount; // Instance count, should never exceed 1.
protected:
    OTCrypto();

    virtual void Init_Override() const;
    virtual void Cleanup_Override() const;

public:
    virtual ~OTCrypto();
    // InstantiateBinarySecret
    // (To instantiate a text secret, just do this: OTPassword thePass;)
    //
    virtual OTPassword* InstantiateBinarySecret() const = 0;
    // GET PASSPHRASE FROM CONSOLE
    //
    EXPORT bool GetPasswordFromConsole(OTPassword& theOutput,
                                       bool bRepeat = false) const;

    EXPORT virtual bool GetPasswordFromConsoleLowLevel(
        OTPassword& theOutput, const char* szPrompt) const = 0;
    // RANDOM NUMBERS
    //
    virtual bool RandomizeMemory(uint8_t* szDestination,
                                 uint32_t nNewSize) const = 0;

    // BASE 58 ENCODING  (for IDs)
    //
    bool IsBase62(const std::string& str) const;

    virtual void SetIDFromEncoded(const String& strInput,
                                  Identifier& theOutput) const = 0;
    virtual void EncodeID(const Identifier& theInput,
                          String& strOutput) const = 0;
    // BASE 64 ENCODING
    //
    virtual bool Base64Encode(const ot_data_t& theInput, std::string& strOutput,
                              bool bLineBreaks = true) const = 0;
    virtual bool Base64Decode(const std::string& strInput, ot_data_t& theOutput,
                              bool bLineBreaks = true) const = 0;

    // KEY DERIVATION
    //
    // DeriveNewKey derives a 128-bit symmetric key from a passphrase.
    //
    // The OTPassword* returned is the actual derived key. (The result.)
    //
    // However, you would not use it directly for symmetric-key crypto, but
    // instead you'd use the OTSymmetricKey class. This is because you still
    // need an object to manage everything about the symmetric key. It stores
    // the salt and the iteration count, as well as ONLY the ENCRYPTED version
    // of the symmetric key, which is a completely random number and is only
    // decrypted briefly for specific operations. The derived key (below) is
    // what we use for briefly decrypting that actual (random) symmetric key.
    //
    // Therefore this function is mainly used INSIDE OTSymmetricKey as part of
    // its internal operations.
    //
    // userPassword argument contains the user's password which is used to
    // derive the key. Presumably you already obtained this passphrase...
    // Then the derived key is returned, or nullptr if failure. CALLER
    // IS RESPONSIBLE TO DELETE!
    // Todo: return a smart pointer here.
    //
    virtual OTPassword* DeriveNewKey(const OTPassword& userPassword,
                                     const ot_data_t& dataSalt,
                                     uint32_t uIterations,
                                     ot_data_t& dataCheckHash) const = 0;

    // ENCRYPT / DECRYPT
    //
    // Symmetric (secret key) encryption / decryption
    //
    virtual bool Encrypt(
        const OTPassword& theRawSymmetricKey, // The symmetric key, in clear
                                              // form.
        const char* szInput,                  // This is the Plaintext.
        uint32_t lInputLength,
        const ot_data_t& theIV, // (We assume this IV is already generated and
        // passed in.)
        ot_data_t& theEncryptedOutput) const = 0; // OUTPUT. (Ciphertext.)

    virtual bool Decrypt(const OTPassword& theRawSymmetricKey, // The symmetric
                                                               // key, in clear
                                                               // form.
                         const char* szInput, // This is the Ciphertext.
                         uint32_t lInputLength,
                         const ot_data_t& theIV, // (We assume this IV is
                                                 // already generated and passed
                                                 // in.)
                         OTCrypto_Decrypt_Output theDecryptedOutput)
        const = 0; // OUTPUT. (Recovered plaintext.) You can pass OTPassword& OR
                   // OTData& here (either will work.)
    // SEAL / OPEN (RSA envelopes...)
    //
    // Asymmetric (public key) encryption / decryption
    //
    virtual bool Seal(mapOfAsymmetricKeys& RecipPubKeys, const String& theInput,
                      ot_data_t& dataOutput) const = 0;

    virtual bool Open(ot_data_t& dataInput, const OTPseudonym& theRecipient,
                      String& theOutput,
                      const OTPasswordData* pPWData = nullptr) const = 0;
    // SIGN / VERIFY
    //
    // Sign or verify using the Asymmetric Key itself.
    //
    virtual bool SignContract(const String& strContractUnsigned,
                              const OTAsymmetricKey& theKey,
                              OTSignature& theSignature, // output
                              const String& strHashType,
                              const OTPasswordData* pPWData = nullptr) = 0;

    virtual bool VerifySignature(
        const String& strContractToVerify, const OTAsymmetricKey& theKey,
        const OTSignature& theSignature, const String& strHashType,
        const OTPasswordData* pPWData = nullptr) const = 0;
    // Sign or verify using the contents of a Certfile.
    //
    virtual bool SignContract(const String& strContractUnsigned,
                              const String& strSigHashType,
                              const std::string& strCertFileContents,
                              OTSignature& theSignature, // output
                              const OTPasswordData* pPWData = nullptr) = 0;

    virtual bool VerifySignature(
        const String& strContractToVerify, const String& strSigHashType,
        const std::string& strCertFileContents, const OTSignature& theSignature,
        const OTPasswordData* pPWData = nullptr) const = 0;
    EXPORT static OTCrypto* It();

    EXPORT void Init() const;
    EXPORT void Cleanup() const;
};

/*
 ftp://ftp.rsasecurity.com/pub/pkcs/pkcs-5v2/pkcs5v2_1.pdf

 4.2 Iteration count

 An iteration count has traditionally served the purpose of increasing
 the cost of producing keys from a password, thereby also increasing
 the difficulty of attack. For the methods in this document, a minimum
 of 1000 iterations is recommended. This will increase the cost of
 exhaustive search for passwords significantly, without a noticeable
 impact in the cost of deriving individual keys.

 Time the KDF on your systems and see how many iterations are practical
 without signficant resource impact on legitimate use cases. If it is
 practical to make the count configurable, do so, otherwise hard-code
 a sensible value that is at least 1000.

 The iteration count is a multiplier on the CPU cost of brute-force
 dictionary attacks. If you are not sure that users are choosing "strong"
 passwords (they rarely do), you want to make dictionary attacks difficult
 by making individual password->key calculations sufficiently slow thereby
 limiting the throughput of brute-force attacks.

 If no legitimate system is computing multiple password-based keys per
 second, you could set the iteration count to consume 10-100ms of CPU
 on 2008 processors, this is likely much more than 1000 iterations.
 At a guess 1000 iterations will be O(1ms) per key on a typical modern CPU.
 This is based on "openssl speed sha1" reporting:

 type             16 bytes     64 bytes    256 bytes   1024 bytes   8192 bytes
 sha1             18701.67k    49726.06k   104600.90k   141349.84k   157502.27k

 or about 10^6 16-byte SHA1 ops per second, doing 1000 iterations of HMAC
 is approximately 2000 SHA1 ops and so should take about 2ms. In many
 applications 10,000 or even 100,000 may be practical provided no
 legitimate "actor" needs to perform password->key comptutations at
 a moderately high rate.

 */

/*
 int32_t PKCS5_PBKDF2_HMAC_SHA1    (
    const void*     password,
    size_t          password_len,

    const void*     salt,
    size_t          salt_len,

    uint64_t     iter,

    size_t          keylen,
    void*          key
)
*/

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTCRYPTO_HPP
