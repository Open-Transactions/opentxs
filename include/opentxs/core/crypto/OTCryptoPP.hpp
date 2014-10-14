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

#ifndef OPENTXS_CORE_CRYPTO_OTCRYPTOPP_HPP
#define OPENTXS_CORE_CRYPTO_OTCRYPTOPP_HPP

#include "OTCrypto.hpp"

#include "../OTPayload.hpp"
#include "../OTString.hpp"
#include "../util/Assert.hpp"

#include <mutex>

#include <set>

namespace opentxs
{

class OTPassword;
class OTPasswordData;

class OTCryptoPP : public OTCrypto
{

    char* Base64Encode(const uint8_t* input, int32_t in_len,
                       bool bLineBreaks) const;

    uint8_t* Base64Decode(const char* input, size_t* out_len,
                          bool bLineBreaks) const;

    void SetIDFromBase62String(const OTString& strInput,
                               OTIdentifier& theOutput) const;

    void SetBase62StringFromID(const OTIdentifier& theInput,
                               OTString& strOutput) const;

    bool RandomizeMemory(uint8_t* szDestination, uint32_t nNewSize) const;

    OTPassword* DeriveKey(
        const OTPassword& userPassword, const OTPayload& dataSalt,
        uint32_t uIterations,
        const OTPayload& dataCheckHash /*= OTPayload()*/) const;

    OTPassword* DeriveNewKey(const OTPassword& userPassword,
                             const OTPayload& dataSalt, uint32_t uIterations,
                             OTPayload& dataCheckHash) const;

    bool CalculateDigest(const OTString& strInput,
                         const OTString& strHashAlgorithm,
                         OTIdentifier& theOutput) const;

    bool CalculateDigest(const OTData& dataInput,
                         const OTString& strHashAlgorithm,
                         OTIdentifier& theOutput) const;

    OTPassword* InstantiateBinarySecret() const;

    bool Encrypt(const OTPassword& theRawSymmetricKey, const char* szInput,
                 const uint32_t lInputLength, const OTPayload& theIV,
                 OTPayload& theEncryptedOutput) const;

    bool Decrypt(const OTPassword& theRawSymmetricKey, const char* szInput,
                 const uint32_t lInputLength, const OTPayload& theIV,
                 OTCrypto_Decrypt_Output theDecryptedOutput) const;

    bool Seal(mapOfAsymmetricKeys& RecipPubKeys, const OTString& theInput,
              OTData& dataOutput) const;

    bool Open(OTData& dataInput, const OTPseudonym& theRecipient,
              OTString& theOutput, const OTPasswordData* pPWData) const;

    bool SignContract(const OTString& strContractUnsigned,
                      const OTAsymmetricKey& theKey, OTSignature& theSignature,
                      const OTString& strHashType,
                      const OTPasswordData* pPWData) const;

    bool VerifySignature(const OTString& strContractToVerify,
                         const OTAsymmetricKey& theKey,
                         const OTSignature& theSignature,
                         const OTString& strHashType,
                         const OTPasswordData* pPWData) const;
};

// low level
struct OTCryptoPP_pvt
{

    // encoding
    static void encode_data_base64(const ot_data_t& in, std::string& out);
    static void decode_data_base64(const std::string& in, ot_data_t& out);
    static void decode_data_base64_secure(const ot_string_secure_t& in,
                                          ot_data_secure_t& out);

    static void encode_data_base62(const ot_data_t& in, std::string& out);
    static void decode_data_base62(const std::string& in, ot_data_t& out);

    // compression
    static void compress_data_zlib(const ot_data_t& in, ot_data_t& out);
    static void decompress_data_zlib(const ot_data_t& in, ot_data_t& out,
                                     const bool attempt = false);

    // random data

    // preset out-size to amount of data wanted.
    static void get_random_data(ot_data_t& out);
    // preset out-size to amount of data wanted.
    static void get_random_data_secure(ot_data_secure_t& out);

    // hashing
    typedef std::function<void(const ot_data_t& in, ot_array_32_t& out)>
        hash256_function;

    static hash256_function get_func_by_name(const std::string& name);

    static void hash_sha256(const ot_data_t& in, ot_array_32_t& out);
    static void hash_whirlpool(const ot_data_t& in, ot_array_64_t& out);

    static void hash_whirlpool256(const ot_data_t& in, ot_array_32_t& out);

    static void hash_samy(const ot_data_t& in, ot_array_32_t& out);
    static void hash_samy_secure(const ot_data_secure_t& in,
                                 ot_array_32_t& out);

    // key-streching
    static void hmac_sha1(const ot_data_secure_t& in, ot_data_secure_t& out);
    static void pkcs5_pbkdf2_hmac_sha1(const ot_data_secure_t& in,
                                       const ot_data_secure_t& salt,
                                       const uint32_t uIterations,
                                       ot_data_secure_t& out);

    // symmetric encryption
    static void encrypt_aes_128_cbc(const ot_data_secure_t& in,
                                    const ot_data_secure_t& key_16,
                                    const ot_array_16_t& iv, ot_data_t& out);

    static void decrypt_aes_128_cbc(const ot_data_t& in,
                                    const ot_data_secure_t& key_16,
                                    const ot_array_16_t& iv,
                                    ot_data_secure_t& out);

    // rsa
    static void pem_cert_to_der_cert(const std::string& in, ot_data_t& out);
    static void ber_cert_to_der_pubkey(const ot_data_t& in, ot_data_t& out);

    static void pem_pubkey_to_der_pubkey(const std::string& in, ot_data_t& out);
    static void pem_privkey_to_der_privkey(const ot_string_secure_t& in,
                                           ot_data_secure_t& out);

    static void ber_privkey_to_der_pubkey(const ot_data_secure_t& in,
                                          ot_data_t& out);

    // rsa raw (for pss)
    static void rsa_raw_decrypt_1024(const ot_array_128_t& in,
                                     const ot_data_t& key, ot_array_128_t& out);

    static void rsa_raw_encrypt_1024(const ot_array_128_t& in,
                                     const ot_data_secure_t& key,
                                     ot_array_128_t& out);

    // rsa pss padding
    static void rsa_verify_pkcs1_pss_mgf1_sha256(const ot_data_t& rep,
                                                 const ot_array_32_t& dgst,
                                                 const int32_t saltLen = -2);

    // pre-set out to desired size, must equal your rsa keylength
    static void rsa_add_pkcs1_pss_mgf1_sha256(const ot_array_32_t& dgst,
                                              ot_data_t& out,
                                              const int32_t saltLen = -2);

    // sign
    static void sign_rsa_pss_sha256_samy(const ot_data_t& msg,
                                         const ot_data_secure_t& key,
                                         ot_array_128_t& sig);

    // verify
    static void verify_rsa_pss_sha256_samy(const ot_data_t& msg,
                                           const ot_data_t& key,
                                           const ot_array_128_t& sig);

    // encrypt
    static void encrypt_rsa_pkcs1(const ot_data_secure_t& msg,
                                  const ot_data_t& key, ot_array_128_t& out);

    // decrypt
    static void decrypt_rsa_pkcs1(const ot_array_128_t& msg,
                                  const ot_data_secure_t& key,
                                  ot_data_secure_t& out);
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_OTCRYPTO_HPP
