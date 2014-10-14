/************************************************************
 *
 *  OTCryptoPP.cpp
 *
 *  Implementation of OTCrypto using CryptoPP
 *
 *  By Cameron Garnham
 *
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

#include "stdafx.hpp"

#include <crypto/OTCryptoPP.hpp>

#include <crypto/OTCrypto.hpp>
#include <crypto/OTPassword.hpp>
#include <crypto/OTAsymmetricKey.hpp>
#include <crypto/OTAsymmetricKeyOpenSSL.hpp>
#include <crypto/OTSignature.hpp>

#include <OTIdentifier.hpp>
#include <OTLog.hpp>
#include <OTPseudonym.hpp>

#include <thread>
#include <iostream>
#include <string>
#include <locale>
#include <cstdint>
#include <functional>
#include <deque>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Winsock2.h> // For htonl()
#endif

extern "C" {
#ifdef _WIN32
#else
#include <arpa/inet.h> // For htonl()
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#endif
}

// will remove later, with lots of testing.
#define CRYPTOPP_DISABLE_ASM
#define CRYPTOPP_DISABLE_SSSE3
#define CRYPTOPP_DISABLE_AESNI

#include <c5/zlib.h>
#include <c5/basecode.h>
#include <c5/base64.h>

#include <c5/osrng.h>

#include <c5/sha.h>
#include <c5/whrlpool.h>

#include <c5/hmac.h>
#include <c5/pwdbased.h>

#include <c5/modes.h>
#include <c5/pssr.h>

#include <c5/aes.h>

#include <c5/rsa.h>
#include <c5/des.h>

#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace opentxs
{

typedef OTCrypto::Exception Exception;

// Caller responsible to delete [].
char* OTCryptoPP::Base64Encode(const uint8_t* input, int32_t in_len,
                               bool bLineBreaks) const
{
    ot_data_t in(input, input + in_len);
    std::string out = {};
    OTCryptoPP_pvt::encode_data_base64(in, out);

    auto out_char = new char[out.size() + 1];
    std::copy(out.begin(), out.end(), out_char);
    out_char[out.size()] = '\0';

    return out_char;
}

// Caller responsible to delete using delete [];
uint8_t* OTCryptoPP::Base64Decode(const char* input, size_t* out_len,
                                  bool bLineBreaks) const
{
    std::string in(input);

    ot_data_t out = {};
    OTCryptoPP_pvt::decode_data_base64(in, out);

    auto data = new uint8_t[out.size()];
    std::copy(out.begin(), out.end(), data);

    *out_len = out.size();

    return data;
}

void OTCryptoPP::SetIDFromBase62String(const OTString& strInput,
                                       OTIdentifier& theOutput) const
{
    std::string input(strInput.Get());
    ot_data_t output = {};

    OTCryptoPP_pvt::decode_data_base62(input, output);

    theOutput.Release();
    theOutput.Assign(output.data(), output.size());
}

void OTCryptoPP::SetBase62StringFromID(const OTIdentifier& theInput,
                                       OTString& strOutput) const
{
    std::string out_string;
    ot_data_t input(theInput.GetDataCopy());

    OTCryptoPP_pvt::encode_data_base62(input, out_string);
    strOutput.Set(out_string.c_str());
}

// pre_allocated memory.
bool OTCryptoPP::RandomizeMemory(uint8_t* szDestination,
                                 uint32_t nNewSize) const
{
    ot_data_secure_t random_memory(nNewSize);
    OTCryptoPP_pvt::get_random_data_secure(random_memory);
    std::copy(random_memory.begin(), random_memory.end(), szDestination);

    return true;
}

OTPassword* OTCryptoPP::DeriveKey(
    const OTPassword& userPassword, const OTPayload& dataSalt,
    uint32_t uIterations,
    const OTPayload& dataCheckHash /*= OTPayload()*/) const
{
    OTPayload tempPayload = dataCheckHash;
    return OTCryptoPP::DeriveNewKey(userPassword, dataSalt, uIterations,
                                    tempPayload);
}

// dataCheckHash will be set if empty.
// return nullptr if checkHash existed but didn't match.
OTPassword* OTCryptoPP::DeriveNewKey(const OTPassword& userPassword,
                                     const OTPayload& dataSalt,
                                     uint32_t uIterations,
                                     OTPayload& dataCheckHash) const
{
    //  OT_ASSERT(userPassword.isPassword());
    OT_ASSERT(!dataSalt.IsEmpty());

    otInfo << __FUNCTION__
           << ": Using a text passphrase, salt, and iteration count, "
              "to make a derived key...\n";

    OTPassword* pDerivedKey(InstantiateBinarySecret()); // already asserts.

    ot_data_secure_t output_key(16), check_key(16);
    {
        ot_data_secure_t salt;
        {
            auto salt_pair = std::make_pair(
                static_cast<const uint8_t*>(dataSalt.GetPayloadPointer()),
                dataSalt.GetSize());

            salt.assign(salt_pair.first, salt_pair.first + salt_pair.second);
        }
        {
            ot_data_secure_t password;
            {
                auto password_pair = std::make_pair(
                    static_cast<const uint8_t*>(
                        userPassword.isPassword()
                            ? userPassword.getPassword_uint8()
                            : userPassword.getMemory_uint8()),
                    userPassword.isPassword() ? userPassword.getPasswordSize()
                                              : userPassword.getMemorySize());

                password.assign(password_pair.first,
                                password_pair.first + password_pair.second);
            }

            // first: Derive Key
            OTCryptoPP_pvt::pkcs5_pbkdf2_hmac_sha1(password, salt, uIterations,
                                                   output_key);

            for (volatile auto& a : password) {
                a = 0;
            }
            password.clear();
        }

        // second: Derive Checksum
        OTCryptoPP_pvt::pkcs5_pbkdf2_hmac_sha1(output_key, salt, uIterations,
                                               check_key);

        for (volatile auto& a : salt) {
            a = 0;
        }
        salt.clear();
    }

    // check checksum
    if (dataCheckHash.IsEmpty()) {
        dataCheckHash.Assign(check_key.data(), check_key.size());
    }
    else {
        ot_data_secure_t checksum;
        {
            auto check_pair = std::make_pair(
                static_cast<const uint8_t*>(dataCheckHash.GetPayloadPointer()),
                dataCheckHash.GetSize());

            checksum.assign(check_pair.first,
                            check_pair.first + check_pair.second);
        }

        // if longer, will have value initialization of uint8_t: zero.
        checksum.resize(check_key.size());

        if (checksum != check_key) // fail!
        {
            delete pDerivedKey;
            pDerivedKey = nullptr;
            return pDerivedKey;
        }
    }

    std::copy(output_key.begin(), output_key.end(),
              static_cast<uint8_t*>(pDerivedKey->getMemoryWritable()));

    for (volatile auto& a : output_key) {
        a = 0;
    }
    output_key.clear();

    return pDerivedKey;
}

bool OTCryptoPP::CalculateDigest(const OTString& strInput,
                                 const OTString& strHashAlgorithm,
                                 OTIdentifier& theOutput) const
{
    theOutput.Release();

    ot_data_t input(strInput.Get(), strInput.Get() + strInput.GetLength());

    ot_array_32_t output;

    OTCryptoPP_pvt::get_func_by_name(strHashAlgorithm.Get())(input, output);

    theOutput.Assign(output.data(), output.size());

    return true;
}

bool OTCryptoPP::CalculateDigest(const OTData& dataInput,
                                 const OTString& strHashAlgorithm,
                                 OTIdentifier& theOutput) const
{
    theOutput.Release();

    const std::pair<const uint8_t*, const size_t> data(
        static_cast<const uint8_t*>(dataInput.GetPointer()),
        dataInput.GetSize());

    ot_data_t input(data.first, data.first + data.second);

    ot_array_32_t output;

    OTCryptoPP_pvt::get_func_by_name(strHashAlgorithm.Get())(input, output);

    theOutput.Assign(output.data(), output.size());

    return true;
}

OTPassword* OTCryptoPP::InstantiateBinarySecret() const
{
    uint8_t* tmp_data = new uint8_t[OTCryptoConfig::SymmetricKeySize()];
    OTPassword* pNewKey = new OTPassword(static_cast<void*>(&tmp_data[0]),
                                         OTCryptoConfig::SymmetricKeySize());
    OT_ASSERT_MSG(nullptr != pNewKey, "pNewKey = new OTPassword");

    if (nullptr != tmp_data) {
        delete[] tmp_data;
        tmp_data = nullptr;
    }

    return pNewKey;
}

bool OTCryptoPP::Encrypt(
    const OTPassword& theRawSymmetricKey, // The symmetric key, in clear form.
    const char* szInput,                  // This is the Plaintext.
    const uint32_t lInputLength, const OTPayload& theIV, // (We assume this IV
    // is already generated
    // and passed in.)
    OTPayload& theEncryptedOutput) const // OUTPUT. (Ciphertext.)
{
    ot_data_secure_t key(16);
    ot_array_16_t iv = {};

    OT_ASSERT(key.size() == theRawSymmetricKey.getMemorySize());
    OT_ASSERT(iv.size() == theIV.GetSize());

    std::copy(theRawSymmetricKey.getMemory_uint8(),
              theRawSymmetricKey.getMemory_uint8() +
                  theRawSymmetricKey.getMemorySize(),
              key.data());

    std::copy(static_cast<const uint8_t*>(theIV.GetPointer()),
              static_cast<const uint8_t*>(theIV.GetPointer()) + theIV.GetSize(),
              iv.data());

    auto input_uint8 = reinterpret_cast<const uint8_t*>(szInput);

    ot_data_secure_t in_data(input_uint8, input_uint8 + lInputLength);
    ot_data_t out_data;

    OTCryptoPP_pvt::encrypt_aes_128_cbc(in_data, key, iv, out_data);

    theEncryptedOutput.Assign(out_data.data(), out_data.size());

    for (volatile auto& a : iv) {
        a = 0;
    }

    return true;
}

bool OTCryptoPP::Decrypt(
    const OTPassword& theRawSymmetricKey, // The symmetric key, in clear form.
    const char* szInput,                  // This is the Ciphertext.
    const uint32_t lInputLength, const OTPayload& theIV, // (We assume this IV
    // is already generated
    // and passed in.)
    OTCrypto_Decrypt_Output theDecryptedOutput) const // OUTPUT. (Recovered
                                                      // plaintext.) You can
                                                      // pass OTPassword& OR
                                                      // OTPayload& here (either
                                                      // will work.)
{
    ot_data_secure_t key(16);
    ot_array_16_t iv = {};

    OT_ASSERT(key.size() == theRawSymmetricKey.getMemorySize());
    OT_ASSERT(iv.size() == theIV.GetSize());

    theDecryptedOutput.Release();

    std::copy(theRawSymmetricKey.getMemory_uint8(),
              theRawSymmetricKey.getMemory_uint8() +
                  theRawSymmetricKey.getMemorySize(),
              key.data());

    std::copy(static_cast<const uint8_t*>(theIV.GetPointer()),
              static_cast<const uint8_t*>(theIV.GetPointer()) + theIV.GetSize(),
              iv.data());

    auto input_uint8 = reinterpret_cast<const uint8_t*>(szInput);

    ot_data_t in_data(input_uint8, input_uint8 + lInputLength);
    ot_data_secure_t out_data = {};

    OTCryptoPP_pvt::decrypt_aes_128_cbc(in_data, key, iv, out_data);
    if (out_data.empty()) return false; // error

    theDecryptedOutput.Concatenate(out_data.data(), out_data.size());

    for (volatile auto& a : iv) {
        a = 0;
    }

    return true;
}

struct envelope_t
{
    struct addr_t
    {
        std::string id;
        ot_data_t ek;
    };

    typedef std::vector<addr_t> addr_v_t;

    uint16_t type;
    addr_v_t addresses;
    ot_data_t iv;
    ot_data_t ciphertext;
};

void encode_envelope(const envelope_t& envelope, ot_data_t& data);
void decode_envelope(const ot_data_t& data, envelope_t& envelope);

void appendShort(uint16_t in, ot_data_t& data);
void appendLong(uint32_t in, ot_data_t& data);

uint16_t readShort(ot_data_t::const_iterator* it,
                   const ot_data_t::const_iterator& end);
uint32_t readLong(ot_data_t::const_iterator* it,
                  const ot_data_t::const_iterator& end);

// set out to size wanted
void readDataVector(ot_data_t::const_iterator* it,
                    const ot_data_t::const_iterator& end, ot_data_t& out);

void encode_envelope(const envelope_t& envelope, ot_data_t& data)
{
    data.clear();

    // Type
    appendShort(envelope.type, data);

    // Number of Addressees
    appendLong(envelope.addresses.size(), data);

    for (auto addr : envelope.addresses) {
        // Nym ID
        appendLong(addr.id.size() + 1, data);
        data.insert(data.end(), addr.id.begin(), addr.id.end());
        data.push_back(0); // null terminator for string.

        // Encrypted Key
        appendLong(addr.ek.size(), data);
        data.insert(data.end(), addr.ek.begin(), addr.ek.end());
    }

    // IV
    appendLong(envelope.iv.size(), data);
    data.insert(data.end(), envelope.iv.begin(), envelope.iv.end());

    // Ciphertext
    data.insert(data.end(), envelope.ciphertext.begin(),
                envelope.ciphertext.end());
}

void decode_envelope(const ot_data_t& data, envelope_t& envelope)
{
    auto data_it = data.begin();

    envelope.type = 0;
    envelope.addresses.clear();
    envelope.iv.clear();
    envelope.ciphertext.clear();

    auto out = envelope; // take copy.

    // Envelope Type (only 1, for now)
    {
        out.type = readShort(&data_it, data.end());

        if (out.type != 1) {
            throw Exception(Exception::INVALID_DATA_FORMAT,
                            "Type 1 is the only supported envelope type");
        }
    }

    // Number of Addressees
    {
        out.addresses.resize(readLong(&data_it, data.end()));

        if (out.addresses.empty()) {
            throw Exception(Exception::INVALID_DATA_FORMAT,
                            "Must have at least one addressee");
        }
    }

    for (auto& addr : out.addresses) {
        // Nym ID
        {
            ot_data_t nym(readLong(&data_it, data.end()));

            readDataVector(&data_it, data.end(), nym);

            std::string nym_temp(nym.begin(), nym.end());
            addr.id.clear();
            addr.id = nym_temp.c_str();
        }

        // Encrypted Key
        {
            addr.ek.clear();
            addr.ek.resize(readLong(&data_it, data.end()));
            readDataVector(&data_it, data.end(), addr.ek);

            if (addr.ek.empty()) {
                throw Exception(Exception::INVALID_DATA_FORMAT,
                                "Key must not be empty!");
            }
        }
    }

    // IV
    {
        out.iv.clear();
        out.iv.resize(readLong(&data_it, data.end()));
        readDataVector(&data_it, data.end(), out.iv);

        if (out.iv.empty()) {
            throw Exception(Exception::INVALID_DATA_FORMAT,
                            "IV must not be empty!");
        }
    }

    // Ciphertext (the rest of the data).
    out.ciphertext.clear();
    out.ciphertext.assign(data_it, data.end());

    if (out.ciphertext.empty()) {
        throw Exception(Exception::INVALID_DATA_FORMAT,
                        "Ciphertext must not be empty!");
    }

    envelope = out;
    return; // success
}

void appendShort(uint16_t in, ot_data_t& data)
{
    auto in_n = htons(in);
    auto in_n_p = reinterpret_cast<uint8_t*>(&in_n);
    data.insert(data.end(), in_n_p, in_n_p + sizeof(in_n));
}

void appendLong(uint32_t in, ot_data_t& data)
{
    auto in_n = htonl(in);
    auto in_n_p = reinterpret_cast<uint8_t*>(&in_n);
    data.insert(data.end(), in_n_p, in_n_p + sizeof(in_n));
}

uint16_t readShort(ot_data_t::const_iterator* it,
                   const ot_data_t::const_iterator& end)
{
    OT_ASSERT(nullptr != it);

    uint16_t out = 0;
    ot_data_t v(sizeof(out));

    for (auto& a : v) {
        if (*it == end)
            throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                            "unexpected end of vector");
        a = *(*it)++;
    }
    out = ntohs(*reinterpret_cast<uint16_t*>(v.data()));
    return out;
}

uint32_t readLong(ot_data_t::const_iterator* it,
                  const ot_data_t::const_iterator& end)
{
    OT_ASSERT(nullptr != it);

    uint32_t out = 0;
    ot_data_t v(sizeof(out));

    for (auto& a : v) {
        if (*it == end)
            throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                            "unexpected end of vector");
        a = *(*it)++;
    }
    out = ntohl(*reinterpret_cast<uint32_t*>(v.data()));
    return out;
}

void readDataVector(ot_data_t::const_iterator* it,
                    const ot_data_t::const_iterator& end, ot_data_t& out)
{
    OT_ASSERT(nullptr != it);

    for (auto& a : out) {
        if (*it == end)
            throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                            "unexpected end of vector");
        a = *(*it)++;
    }
}

// Seal up as envelope (Asymmetric, using public key and then AES key.)

bool OTCryptoPP::Seal(mapOfAsymmetricKeys& RecipPubKeys,
                      const OTString& theInput, OTData& dataOutput) const
{
    if (RecipPubKeys.empty()) {
        otErr << __FUNCTION__ << " cannot seal an envelope to noone!"
              << std::endl;
        return false;
    }

    ot_data_secure_t pvt_rand_key(16);
    ot_array_16_t rand_iv = {};

    {
        CryptoPP::AutoSeededRandomPool rng;

        rng.GenerateBlock(pvt_rand_key.data(), pvt_rand_key.size());
        rng.GenerateBlock(rand_iv.data(), rand_iv.size());
    }

    envelope_t envelope = {};
    envelope.type = 1; // only one type for now.

    envelope.iv.assign(rand_iv.begin(), rand_iv.end());
    envelope.addresses.resize(RecipPubKeys.size());

    std::vector<std::vector<uint8_t>> pk_vv(RecipPubKeys.size());

    // get data from RecipPubKeys, and decode pubkey
    {
        auto pk_vv_it = pk_vv.begin();
        auto addr_it = envelope.addresses.begin();
        for (auto pubkey_pair : RecipPubKeys) {
            (*addr_it++).id = pubkey_pair.first;

            auto lowlevel_pubkey =
                dynamic_cast<OTAsymmetricKey_OpenSSL*>(pubkey_pair.second);

            OTString pk_otstr = {};
            lowlevel_pubkey->SavePubkeyToString(pk_otstr);
            std::string pk_str(pk_otstr.Get());
            ot_data_t ek;
            OTCryptoPP_pvt::pem_pubkey_to_der_pubkey(pk_str, ek);
            *pk_vv_it++ = ek;
        }
    }

    // Encrypt secret key to to each public key.
    {
        auto addr_it = envelope.addresses.begin();
        for (auto pk : pk_vv) {
            ot_array_128_t ek = {};
            ot_data_secure_t msg(pvt_rand_key.begin(), pvt_rand_key.end());
            OTCryptoPP_pvt::encrypt_rsa_pkcs1(msg, pk, ek);
            (*addr_it++).ek.assign(ek.begin(), ek.end());
        }
    }

    // Encrypt the message to the secret key.

    if (!theInput.Exists()) {
        otErr << __FUNCTION__ << " cannot seal no message!" << std::endl;
        return false;
    }

    {
        std::string input_str(theInput.Get());
        ot_data_secure_t input(input_str.begin(), input_str.end());

        OTCryptoPP_pvt::encrypt_aes_128_cbc(input, pvt_rand_key, rand_iv,
                                            envelope.ciphertext);
    }

    ot_data_t envelope_data;
    encode_envelope(envelope, envelope_data);

    dataOutput.Release();
    dataOutput.Assign(envelope_data.data(), envelope_data.size());

    return true;
}

bool OTCryptoPP::Open(OTData& dataInput, const OTPseudonym& theRecipient,
                      OTString& theOutput, const OTPasswordData* pPWData) const
{
    struct private_key_ptr
    {
        OTAsymmetricKey_OpenSSL* pvtKey;

        ~private_key_ptr()
        {
            if (nullptr != pvtKey) pvtKey->ReleaseKey();
        }
    };

    std::string our_nym_id;
    {
        OTString nymID;
        theRecipient.GetIdentifier(nymID);
        our_nym_id = nymID.Get();
    }

    ot_data_secure_t pvtkey = {};
    {
        private_key_ptr ot_privateKey = {};

        auto& theTempPrivateKey =
            const_cast<OTAsymmetricKey&>(theRecipient.GetPrivateEncrKey());

        ot_privateKey.pvtKey =
            dynamic_cast<OTAsymmetricKey_OpenSSL*>(&theTempPrivateKey);
        OT_ASSERT(nullptr != ot_privateKey.pvtKey);

        ot_string_secure_t pvtk_otstr;
        if (!ot_privateKey.pvtKey->SaveDecryptedPrivateKeyToString(
                pvtk_otstr)) {
            otErr << __FUNCTION__
                  << ": Unable to get Private Key from theRecipient!\n";
            return false;
        };

        OTCryptoPP_pvt::pem_privkey_to_der_privkey(pvtk_otstr, pvtkey);

        if (pvtkey.empty()) {
            otErr << __FUNCTION__ << ": Invalid private key supplied!\n";
            return false;
        }
    }

    envelope_t envelope = {};

    try {
        auto data = dataInput.GetDataCopy();
        decode_envelope(data, envelope);
    } catch (Exception e) {
        otErr << __FUNCTION__ << ": " << e.GetWhat() << std::endl;
        return false;
    }

    ot_data_secure_t secret_key = {};

    for (auto addr : envelope.addresses) {
        if (!our_nym_id.empty() && !addr.id.empty()) {
            if (0 != our_nym_id.compare(addr.id)) {
                continue; // we have id's, but they don't match.
            }
        }

        try {

            ot_array_128_t ek_a = {};
            OT_ASSERT(addr.ek.size() == ek_a.size());

            std::copy(addr.ek.begin(), addr.ek.end(), ek_a.begin());

            OTCryptoPP_pvt::decrypt_rsa_pkcs1(ek_a, pvtkey, secret_key);
        } catch (Exception e) {
            continue;
        }
    }
    if (secret_key.empty()) {
        otErr << __FUNCTION__ << ": No secret key! (Returning false.)\n";
        return false;
    }

    if (secret_key.size() != envelope.iv.size()) {
        otErr << __FUNCTION__ << ": Invalid secret key found!\n";
        return false;
    }

    ot_array_16_t iv_a = {};

    if (envelope.iv.size() != iv_a.size()) {
        otErr << __FUNCTION__ << ": Invalid secret key found!\n";
        return false;
    }

    std::copy(envelope.iv.begin(), envelope.iv.end(), iv_a.begin());

    ot_data_secure_t msg = {};
    OTCryptoPP_pvt::decrypt_aes_128_cbc(envelope.ciphertext, secret_key, iv_a,
                                        msg);

    std::string msg_str(msg.begin(), msg.end());
    theOutput.Release();
    theOutput = msg_str.c_str();

    return true;
}

bool OTCryptoPP::SignContract(const OTString& strContractUnsigned,
                              const OTAsymmetricKey& theKey,
                              OTSignature& theSignature, // output
                              const OTString& strHashType,
                              const OTPasswordData* pPWData) const
{
    auto& theTempKey = const_cast<OTAsymmetricKey&>(theKey);
    auto pTempOpenSSLKey = dynamic_cast<OTAsymmetricKey_OpenSSL*>(&theTempKey);
    OT_ASSERT(nullptr != pTempOpenSSLKey);

    if (!pTempOpenSSLKey->IsPrivate()) {
        otErr << __FUNCTION__ << " Not Private Key, returning false! \n";
        return false;
    }

    ot_string_secure_t pubKey;
    if (!pTempOpenSSLKey->SaveDecryptedPrivateKeyToString(pubKey)) {
        otErr << __FUNCTION__
              << " Unable to get Private Key, returning false! \n";
        return false;
    }

    std::string contract(strContractUnsigned.Get());
    ot_data_t contract_v(contract.begin(), contract.end());

    ot_data_secure_t key = {};
    OTCryptoPP_pvt::pem_privkey_to_der_privkey(pubKey, key);

    ot_array_128_t sig = {};
    OTCryptoPP_pvt::sign_rsa_pss_sha256_samy(contract_v, key, sig);
    OTData sig_data(sig.data(), sig.size());
    theSignature.SetAndPackData(sig_data);

    return true;
}

bool OTCryptoPP::VerifySignature(const OTString& strContractToVerify,
                                 const OTAsymmetricKey& theKey,
                                 const OTSignature& theSignature,
                                 const OTString& strHashType,
                                 const OTPasswordData* pPWData) const
{
    ot_data_t key;

    // find key.
    {
        auto& theTempKey = const_cast<OTAsymmetricKey&>(theKey);
        auto pTempOpenSSLKey =
            dynamic_cast<OTAsymmetricKey_OpenSSL*>(&theTempKey);
        OT_ASSERT(nullptr != pTempOpenSSLKey);

        if (!pTempOpenSSLKey->IsPublic()) {
            otErr << __FUNCTION__ << " Not Public Key, returning false! \n";
            return false;
        }

        OTString pubKey;
        if (!pTempOpenSSLKey->SavePubkeyToString(pubKey)) {
            otErr << __FUNCTION__
                  << " Unable to get Public Key, returning false! \n";
            return false;
        }

        OTCryptoPP_pvt::pem_pubkey_to_der_pubkey(pubKey.Get(), key);
    }

    OTData sigData;
    theSignature.GetData(sigData);

    auto sig_pair = std::make_pair(
        static_cast<const uint8_t*>(sigData.GetPointer()), sigData.GetSize());

    ot_data_secure_t sig_v(sig_pair.first, sig_pair.first + sig_pair.second);
    ot_array_128_t sig;
    std::copy_n(sig_v.begin(), sig.size(), sig.begin());

    std::string msg_str(strContractToVerify.Get());
    ot_data_t msg(msg_str.begin(), msg_str.end());

    OTCryptoPP_pvt::verify_rsa_pss_sha256_samy(msg, key, sig);

    return true;
}

// static
void OTCryptoPP_pvt::encode_data_base64(const ot_data_t& in, std::string& out)
{
    const auto input = in; // take a copy
    if (input.empty()) return;
    out.clear();

    CryptoPP::ArraySource(
        input.data(), input.size(), true,
        new CryptoPP::Base64Encoder(new CryptoPP::StringSink(out)));
}

// static
void OTCryptoPP_pvt::decode_data_base64(const std::string& in, ot_data_t& out)
{
    const auto input = in; // take a copy
    if (input.empty()) return;
    out.clear();

    CryptoPP::Base64Decoder dec;

    ot_data_t in_v(in.begin(), in.end());
    dec.PutMessageEnd(in_v.data(), in_v.size());

    out.resize(static_cast<size_t>(dec.TotalBytesRetrievable()));
    out.resize(dec.Get(out.data(), out.size()));
}

// static
void OTCryptoPP_pvt::decode_data_base64_secure(const ot_string_secure_t& in,
                                               ot_data_secure_t& out)
{
    const auto input = in; // take a copy
    if (input.empty()) return;
    out.clear();

    CryptoPP::Base64Decoder dec;

    ot_data_secure_t in_v(in.begin(), in.end());
    dec.PutMessageEnd(in_v.data(), in_v.size());

    out.resize(static_cast<size_t>(dec.TotalBytesRetrievable()));
    out.resize(dec.Get(out.data(), out.size()));
}

// static
void OTCryptoPP_pvt::encode_data_base62(const ot_data_t& in, std::string& out)
{
    static const ot_data_t vec = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
        'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c',
        'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
        'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    const auto input = in; // take a copy
    out.clear();
    if (input.empty()) return;

    ot_data_t encoded_values = {};

    CryptoPP::Integer in_n(input.data(), input.size());
    CryptoPP::Integer dividend = in_n;
    const CryptoPP::Integer divisor(62);

    for (;;) {
        CryptoPP::Integer remainder, quotient;
        CryptoPP::Integer::Divide(remainder, quotient, dividend, divisor);
        encoded_values.push_back(
            static_cast<uint8_t>(remainder.ConvertToLong()));
        if (quotient == 0) break;
        dividend = quotient;
    }

    std::deque<char> out_deque = {};

    for (auto val : encoded_values) {
        out_deque.push_front(vec.at(val));
    }

    out.clear();
    out.assign(out_deque.begin(), out_deque.end());
}

// static
void OTCryptoPP_pvt::decode_data_base62(const std::string& in, ot_data_t& out)
{
    static const ot_data_t vec = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
        'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c',
        'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
        'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};

    static std::vector<signed int> vec_lookup = {};

    if (vec_lookup.empty()) {
        vec_lookup.resize(255, -1);
        size_t i = 0;
        for (auto v : vec) {
            vec_lookup.at(v) = i++;
        }
    }

    const auto input = in; // take a copy
    if (input.empty()) return;
    out.clear();

    std::vector<byte> in_data = {};

    for (auto c : input) {

        auto r = vec_lookup.at(c);
        if (r < 0)
            throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                            "input contains non-base62 character!");

        in_data.push_back(r);
    }

    CryptoPP::Integer out_n;
    const CryptoPP::Integer multiplier(62);

    for (auto a : in_data) {
        out_n *= multiplier; // first will be 0 * 62 = 0;
        out_n += a;
    }

    ot_data_t out_v(in_data.size());
    out_n.Encode(out_v.data(), out_v.size());

    const uint8_t zero = {};
    out.assign(std::find_if_not(out_v.begin(), out_v.end(),
                                std::bind2nd(std::equal_to<uint8_t>(), 0)),
               out_v.end());
}

// static
void OTCryptoPP_pvt::compress_data_zlib(const ot_data_t& in, ot_data_t& out)
{
    const auto input = in; // take a copy
    if (input.empty()) return;
    out.clear();

    CryptoPP::ZlibCompressor zibcompressor;

    zibcompressor.Initialize();
    zibcompressor.PutMessageEnd(&input.at(0), input.size());
    zibcompressor.Flush(1);

    out.resize(static_cast<size_t>(zibcompressor.TotalBytesRetrievable()));
    out.resize(zibcompressor.Get(out.data(), out.size()));
}

// static
void OTCryptoPP_pvt::decompress_data_zlib(const ot_data_t& in, ot_data_t& out,
                                          const bool attempt)
{
    const auto input = in; // take a copy
    if (input.empty()) return;
    out.clear();

    CryptoPP::ZlibDecompressor zlibdecompressor;

    try {
        zlibdecompressor.Initialize();
        zlibdecompressor.PutMessageEnd(input.data(), input.size());
        zlibdecompressor.Flush(1);
    } catch (CryptoPP::ZlibDecompressor::Err e) {

        if (attempt) {
            if (e.GetErrorType() ==
                CryptoPP::ZlibDecompressor::Err::INVALID_DATA_FORMAT) {
                return; // we are only trying.
            }
        }

        otErr << __FUNCTION__ << " Error: " << e.GetWhat() << "\n";

        std::string input_string(input.begin(), input.end());
        otErr << "Input: \n --- \n" << input_string << "\n --- \n";

        throw std::exception(e);
    }

    out.resize(static_cast<size_t>(zlibdecompressor.TotalBytesRetrievable()));
    out.resize(zlibdecompressor.Get(out.data(), out.size()));
}

// static
void OTCryptoPP_pvt::get_random_data(ot_data_t& out)
{
    CryptoPP::AutoSeededRandomPool rng;
    rng.GenerateBlock(out.data(), out.size());
}

// static
void OTCryptoPP_pvt::get_random_data_secure(ot_data_secure_t& out)
{
    CryptoPP::AutoSeededRandomPool rng;
    rng.GenerateBlock(out.data(), out.size());
}

// static
OTCryptoPP_pvt::hash256_function OTCryptoPP_pvt::get_func_by_name(
    const std::string& name)
{

    if (name.compare("SHA256") == 0) {
        return &hash_sha256;
    }

    if (name.compare("WHIRLPOOL") == 0) {
        return &hash_whirlpool256;
    }

    if (name.compare("SAMY") == 0) {
        return &hash_samy;
    }

    // we have not the hash you are looking for
    OT_FAIL;
}

// static
void OTCryptoPP_pvt::hash_sha256(const ot_data_t& in, ot_array_32_t& out)
{
    CryptoPP::SHA256 hash;
    hash.CalculateDigest(out.data(), in.data(), in.size());
}

// static
void OTCryptoPP_pvt::hash_whirlpool(const ot_data_t& in, ot_array_64_t& out)
{
    CryptoPP::Whirlpool hash;
    hash.CalculateDigest(out.data(), in.data(), in.size());
}

// static
void OTCryptoPP_pvt::hash_whirlpool256(const ot_data_t& in, ot_array_32_t& out)
{
    CryptoPP::Whirlpool hash;

    hash.CalculateTruncatedDigest(out.data(), out.size(), in.data(), in.size());
}

// static
void OTCryptoPP_pvt::hash_samy(const ot_data_t& in, ot_array_32_t& out)
{
    CryptoPP::SHA256 hash_sha256;
    CryptoPP::Whirlpool hash_whirlpool;

    // we only want the first 32 bytes of each.
    ot_array_32_t dgst_sha256 = {};
    ot_array_32_t dgst_whirlpool = {};

    hash_sha256.CalculateTruncatedDigest(dgst_sha256.data(), dgst_sha256.size(),
                                         in.data(), in.size());
    hash_whirlpool.CalculateTruncatedDigest(
        dgst_whirlpool.data(), dgst_whirlpool.size(), in.data(), in.size());

    CryptoPP::xorbuf(out.data(), dgst_sha256.data(), dgst_whirlpool.data(),
                     out.size());
}

// static
void OTCryptoPP_pvt::hash_samy_secure(const ot_data_secure_t& in,
                                      ot_array_32_t& out)
{
    CryptoPP::SHA256 hash_sha256;
    CryptoPP::Whirlpool hash_whirlpool;

    // we only want the first 32 bytes of each.
    ot_array_32_t dgst_sha256 = {};
    ot_array_32_t dgst_whirlpool = {};

    hash_sha256.CalculateTruncatedDigest(dgst_sha256.data(), dgst_sha256.size(),
                                         in.data(), in.size());
    hash_whirlpool.CalculateTruncatedDigest(
        dgst_whirlpool.data(), dgst_whirlpool.size(), in.data(), in.size());

    CryptoPP::xorbuf(out.data(), dgst_sha256.data(), dgst_whirlpool.data(),
                     out.size());
}

// static
void OTCryptoPP_pvt::hmac_sha1(const ot_data_secure_t& in,
                               ot_data_secure_t& out)
{
    const auto input = in; // take a copy
    if (input.empty()) return;
    {
        const auto outsize = out.size();
        out.clear();
        out.resize(outsize);
    }

    CryptoPP::HMAC<CryptoPP::SHA1> hmac;

    hmac.SetKey(input.data(), input.size());

    hmac.TruncatedFinal(out.data(), out.size());
}

// static
void OTCryptoPP_pvt::pkcs5_pbkdf2_hmac_sha1(const ot_data_secure_t& in,
                                            const ot_data_secure_t& salt,
                                            const uint32_t uIterations,
                                            ot_data_secure_t& out)
{
    const auto input = in; // take a copy
    if (input.empty()) return;
    {
        const auto outsize = out.size();
        out.clear();
        out.resize(outsize);
    }

    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA1> pkcs5;

    pkcs5.DeriveKey(out.data(), out.size(), 0, input.data(), input.size(),
                    salt.data(), salt.size(), uIterations);
}

// static
void OTCryptoPP_pvt::encrypt_aes_128_cbc(const ot_data_secure_t& in,
                                         const ot_data_secure_t& key,
                                         const ot_array_16_t& iv,
                                         ot_data_t& out)
{
    class AesEncryptor : public CryptoPP::ProxyFilter
    {
    public:
        AesEncryptor(const ot_data_secure_t& key, const ot_array_16_t& iv,
                     CryptoPP::BufferedTransformation* attachment = nullptr)
            : ProxyFilter(nullptr, 0, 0, attachment)
            , m_key(key)
            , m_iv(iv){};

    private:
        CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption m_cipher;

        const ot_data_secure_t& m_key;
        const ot_array_16_t& m_iv;

    protected:
        void FirstPut(const byte*)
        {
            m_cipher.SetKeyWithIV(m_key.data(), m_key.size(), m_iv.data());
            SetFilter(new CryptoPP::StreamTransformationFilter(m_cipher));
        };

        void LastPut(const byte* inString, size_t length)
        {
            m_filter->MessageEnd();
        }
    };

    const auto input = in; // take a copy
    if (input.empty()) return;

    out.clear();
    out.resize(input.size());

    AesEncryptor enc(key, iv);

    enc.PutMessageEnd(input.data(), input.size());

    out.resize(static_cast<size_t>(enc.TotalBytesRetrievable()));
    out.resize(enc.Get(out.data(), out.size()));
}

// static
void OTCryptoPP_pvt::decrypt_aes_128_cbc(const ot_data_t& in,
                                         const ot_data_secure_t& key,
                                         const ot_array_16_t& iv,
                                         ot_data_secure_t& out)
{
    class AesDecryptor : public CryptoPP::ProxyFilter
    {
    public:
        AesDecryptor(const ot_data_secure_t& key, const ot_array_16_t& iv,
                     CryptoPP::BufferedTransformation* attachment = nullptr)
            : ProxyFilter(nullptr, 0, 0, attachment)
            , m_key(key)
            , m_iv(iv){};

    private:
        const ot_data_secure_t& m_key;
        const ot_array_16_t& m_iv;

        CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption m_cipher;
        CryptoPP::member_ptr<FilterWithBufferedInput> m_decryptor;

    protected:
        void FirstPut(const byte* inString)
        {
            m_cipher.SetKeyWithIV(m_key.data(), m_key.size(), m_iv.data());
            SetFilter(new CryptoPP::StreamTransformationFilter(m_cipher));
        }
        void LastPut(const byte* inString, size_t length)
        {
            m_filter->MessageEnd();
        }
    };

    const auto input = in; // take a copy
    if (input.empty()) return;

    out.clear();
    out.resize(input.size());

    AesDecryptor dec(key, iv);

    try {
        dec.PutMessageEnd(input.data(), input.size());
    } catch (CryptoPP::InvalidCiphertext e) {

        otErr << "bad input Ciphertext: " << e.GetWhat() << e.GetErrorType()
              << std::endl;
        out.clear();
        return;
    }

    out.resize(static_cast<size_t>(dec.TotalBytesRetrievable()));
    out.resize(dec.Get(out.data(), out.size()));
}

// static
void OTCryptoPP_pvt::pem_cert_to_der_cert(const std::string& in, ot_data_t& out)
{
    std::istringstream ss(in);

    const std::vector<std::string> pem_lines = {
        "-----BEGIN", "CERTIFICATE-----", "-----END", "CERTIFICATE-----"};

    std::vector<std::string> key_lines{std::istream_iterator<std::string>{ss},
                                       std::istream_iterator<std::string>{}};

    std::stringstream key_stream;
    for (auto line : key_lines) {
        bool good = true;
        for (auto bad_line : pem_lines) {
            if (line == bad_line) {
                good = false;
            }
        }
        if (good) key_stream << line;
    }

    std::string key_str(key_stream.str());

    OTCryptoPP_pvt::decode_data_base64(key_str, out);
}

/**
* Reads an X.509 v3 certificate from certin, extracts the subjectPublicKeyInfo
* structure
* (which is one way PK_Verifiers can get their key material) and writes it to
* keyout
*/

// static
void OTCryptoPP_pvt::ber_cert_to_der_pubkey(const ot_data_t& in, ot_data_t& out)
{
    using namespace CryptoPP;

    const auto input = in; // take a copy
    if (input.empty()) return;

    out.clear();
    out.resize(input.size());

    ArraySource as(input.data(), input.size(), true);

    BERSequenceDecoder x509Cert(as);
    BERSequenceDecoder tbsCert(x509Cert);

    // ASN.1 from RFC 3280
    // TBSCertificate  ::=  SEQUENCE  {
    // version         [0]  EXPLICIT Version DEFAULT v1,

    // consume the context tag on the version
    BERGeneralDecoder context(tbsCert, 0xa0);
    word32 ver;

    // only want a v3 cert
    BERDecodeUnsigned<word32>(context, ver, INTEGER, 2, 2);

    // serialNumber         CertificateSerialNumber,
    Integer serial;
    serial.BERDecode(tbsCert);

    // signature            AlgorithmIdentifier,
    BERSequenceDecoder signature(tbsCert);
    signature.SkipAll();

    // issuer               Name,
    BERSequenceDecoder issuerName(tbsCert);
    issuerName.SkipAll();

    // validity             Validity,
    BERSequenceDecoder validity(tbsCert);
    validity.SkipAll();

    // subject              Name,
    BERSequenceDecoder subjectName(tbsCert);
    subjectName.SkipAll();

    // subjectPublicKeyInfo SubjectPublicKeyInfo,
    BERSequenceDecoder spki(tbsCert);

    MeterFilter outBuff;
    DERSequenceEncoder spkiEncoder(outBuff);

    spki.CopyTo(spkiEncoder);
    spkiEncoder.MessageEnd();

    spki.SkipAll();
    tbsCert.SkipAll();
    x509Cert.SkipAll();

    out.resize(static_cast<size_t>(outBuff.TotalBytesRetrievable()));
    out.resize(outBuff.Get(out.data(), out.size()));
}

// static
void OTCryptoPP_pvt::pem_pubkey_to_der_pubkey(const std::string& in,
                                              ot_data_t& out)
{
    std::istringstream ss(in);

    const std::vector<std::string> pem_lines = {"-----BEGIN", "PUBLIC",
                                                "KEY-----", "-----END"};

    std::vector<std::string> key_lines{std::istream_iterator<std::string>{ss},
                                       std::istream_iterator<std::string>{}};

    std::stringstream key_stream;
    for (auto line : key_lines) {
        bool good = true;
        for (auto bad_line : pem_lines) {
            if (line == bad_line) {
                good = false;
            }
        }
        if (good) key_stream << line;
    }

    std::string key_str(key_stream.str());

    OTCryptoPP_pvt::decode_data_base64(key_str, out);
}

// static
void OTCryptoPP_pvt::pem_privkey_to_der_privkey(const ot_string_secure_t& in,
                                                ot_data_secure_t& out)
{
    ot_istringstream_secure_t ss(in);

    const std::vector<ot_string_secure_t> pem_lines = {
        "-----BEGIN", "ENCRYPTED", "PRIVATE", "KEY-----", "-----END"};

    std::vector<ot_string_secure_t> key_lines{
        std::istream_iterator<ot_string_secure_t>{ss},
        std::istream_iterator<ot_string_secure_t>{}};

    ot_stringstream_secure_t key_stream;
    for (auto line : key_lines) {
        bool good = true;
        for (auto bad_line : pem_lines) {
            if (line == bad_line) {
                good = false;
            }
        }
        if (good) key_stream << line;
    }

    ot_string_secure_t key_str(key_stream.str());

    decode_data_base64_secure(key_str, out);
}

// static
void OTCryptoPP_pvt::ber_privkey_to_der_pubkey(const ot_data_secure_t& in,
                                               ot_data_t& out)
{
    CryptoPP::RSA::PrivateKey pvt;
    pvt.BERDecode(CryptoPP::ArraySource(in.data(), in.size(), true));
    CryptoPP::RSA::PublicKey pub(pvt);
    CryptoPP::MeterFilter s;
    pub.BEREncode(s);

    out.resize(static_cast<size_t>(s.TotalBytesRetrievable()));
    out.resize(s.Get(out.data(), out.size()));
}

// static
void OTCryptoPP_pvt::rsa_raw_decrypt_1024(const ot_array_128_t& in,
                                          const ot_data_t& key,
                                          ot_array_128_t& out)
{
    CryptoPP::ArraySource as(key.data(), key.size(), true);

    const ot_array_128_t encrypted = in;

    CryptoPP::RSAFunction rsa;
    rsa.BERDecode(as);
    CryptoPP::Integer a(encrypted.data(), encrypted.size());
    CryptoPP::Integer res = rsa.ApplyFunction(a);
    res.Encode(out.data(), out.size());
}

// static
void OTCryptoPP_pvt::rsa_raw_encrypt_1024(const ot_array_128_t& in,
                                          const ot_data_secure_t& key,
                                          ot_array_128_t& out)
{
    CryptoPP::ArraySource as(key.data(), key.size(), true);
    CryptoPP::AutoSeededRandomPool rng;

    const ot_array_128_t encrypted = in;

    CryptoPP::InvertibleRSAFunction rsa;
    rsa.BERDecode(as);
    CryptoPP::Integer a(encrypted.data(), encrypted.size());
    CryptoPP::Integer res = rsa.CalculateInverse(rng, a);
    res.Encode(out.data(), out.size());
}

// by Cameron, inspired from OpenSSL code.

// static
void OTCryptoPP_pvt::rsa_verify_pkcs1_pss_mgf1_sha256(const ot_data_t& rep,
                                                      const ot_array_32_t& dgst,
                                                      const int32_t saltLen)
{
    CryptoPP::SHA256 hash;

    if (dgst.size() != hash.DigestSize()) {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "RSA_R_MISSMATCH_HASH_LENGTH");
    }

    OT_ASSERT(saltLen < 100000); // cannot be tool large.

    /*
    * Negative sLen has special meanings:
    *	-1	sLen == hLen
    *	-2	salt length is autorecovered from signature
    *	-N	reserved
    */
    auto sLen = saltLen;
    if (sLen == -1)
        sLen = hash.DigestSize();
    else if (sLen == -2)
        sLen = -2;
    else if (sLen < -2) {
        throw Exception(Exception::INVALID_ARGUMENT, "RSA_R_SLEN_CHECK_FAILED");
    }

    const auto MSBits = (rep.size() * 8 - 1) & 0x7;

    if (rep.at(0) & (0xFF << MSBits)) {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "RSA_R_FIRST_OCTET_INVALID");
    }
    if (rep.at(rep.size() - 1) != 0xbc) {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "RSA_R_LAST_OCTET_INVALID");
    }

    const auto rep_it = rep.begin() + (MSBits == 0 ? 1 : 0);

    // the msg must be at-least as large as the hash + salt length
    if ((rep.end() - rep_it) <
        static_cast<int32_t>(
            (hash.DigestSize() + sLen + 2))) /* sLen can be small negative */
    {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "RSA_R_DATA_TOO_LARGE");
    }

    ot_data_t db(rep_it, rep.end() - hash.DigestSize() - 1);
    const ot_data_t h(rep_it + db.size(), rep.end() - 1);
    if (h.size() != hash.DigestSize()) {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "RSA_R_MISMATCH_DERIVED_HASH");
    }

    CryptoPP::P1363_MGF1 mgf1;
    mgf1.GenerateAndMask(hash, db.data(), db.size(), h.data(), h.size());

    if (MSBits) db.at(0) &= 0xFF >> (8 - MSBits);

    auto db_it = db.begin();
    for (; *db_it == 0 && db_it != db.end(); db_it++)
        ;
    if (*db_it++ != 0x1) {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "RSA_R_SLEN_RECOVERY_FAILED");
    }

    const ot_data_secure_t salt(db_it, db.end());

    if (sLen >= 0 && salt.size() != sLen) {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "RSA_R_SLEN_CHECK_FAILED");
    }

    ot_array_8_t zero = {};

    hash.Restart();
    hash.Update(zero.data(), zero.size());
    hash.Update(dgst.data(), dgst.size());
    hash.Update(salt.data(), salt.size());
    if (!hash.Verify(h.data())) {
        throw Exception(Exception::VERIFICATION_FAILURE, "RSA_R_BAD_SIGNATURE");
    }
    return;
}

// by Cameron, inspired from OpenSSL code.

// static
void OTCryptoPP_pvt::rsa_add_pkcs1_pss_mgf1_sha256(const ot_array_32_t& dgst,
                                                   ot_data_t& out,
                                                   const int32_t saltLen)
{

    OT_ASSERT(saltLen < 100000);

    if (out.size() < 128) {
        throw Exception(Exception::INVALID_ARGUMENT, "RSA_R_OUT_TOO_SMALL");
    }

    {
        auto a = out.size();
        out.clear();
        out.resize(a);
    }

    CryptoPP::SHA256 hash;

    if (dgst.size() != hash.DigestSize()) {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "RSA_R_MISSMATCH_HASH_LENGTH");
    }

    /*
    * Negative sLen has special meanings:
    *	-1	sLen == hLen
    *	-2	salt length is autorecovered from signature
    *	-N	reserved
    */
    ot_data_t salt = {};
    if (saltLen == -1)
        salt.resize(hash.DigestSize());
    else if (saltLen == -2)
        ;
    else if (saltLen < -2) {
        throw Exception(Exception::INVALID_ARGUMENT, "RSA_R_SLEN_CHECK_FAILED");
    }

    const auto MSBits = (out.size() - 1) & 0x7;
    auto out_it = out.begin();

    if (MSBits == 0) {
        *out_it++ = 0;
    }

    if (saltLen == -2) {
        auto a = (out.end() - out_it) - hash.DigestSize() - 2;
        salt.resize(a);
    }
    else if ((out.end() - out_it) <
               static_cast<int64_t>(hash.DigestSize() + salt.size() + 2)) {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "RSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE");
    }

    ot_data_t db((out.end() - out_it) - hash.DigestSize() - 1);
    ot_data_t h((out.end() - out_it) - db.size() - 1);

    if (h.size() != hash.DigestSize()) {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "RSA_R_MISSMATCH_HASH_LENGTH");
    }

    ot_array_8_t zero = {};

    if (salt.size() != 0) {
        CryptoPP::AutoSeededRandomPool rng;
        CryptoPP::RandomNumberStore rn(rng, salt.size());
        rn.TransferAllTo(CryptoPP::ArraySink(salt.data(), salt.size()));
    }

    hash.Restart();
    hash.Update(zero.data(), zero.size());
    hash.Update(dgst.data(), dgst.size());
    hash.Update(salt.data(), salt.size());
    hash.Final(h.data());

    auto out_end_it = out.end();
    *--out_end_it = 0xbc;
    std::copy_backward(h.begin(), h.end(), out_end_it);
    out_end_it -= h.size();

    db.at(0) = 0x1;
    std::copy(salt.begin(), salt.end(), db.begin() + 1);

    hash.Restart();
    CryptoPP::P1363_MGF1 mgf1;
    mgf1.GenerateAndMask(hash, db.data(), db.size(), h.data(), h.size());

    std::copy_backward(db.begin(), db.end(), out_end_it);

    if (MSBits) out.at(0) &= 0xFF >> (8 - MSBits);

#ifdef _DEBUG
    rsa_verify_pkcs1_pss_mgf1_sha256(out, dgst, saltLen); // verify
#endif
}

// static
void OTCryptoPP_pvt::sign_rsa_pss_sha256_samy(const ot_data_t& msg,
                                              const ot_data_secure_t& key,
                                              ot_array_128_t& sig)
{
    ot_array_32_t dgst = {};

    hash_samy(msg, dgst);

    ot_data_t rep(128);
    rsa_add_pkcs1_pss_mgf1_sha256(dgst, rep);

    ot_array_128_t rep_a = {};
    std::copy_n(rep.begin(), rep_a.size(), rep_a.begin());
    rsa_raw_encrypt_1024(rep_a, key, sig);

#ifdef _DEBUG

    ot_data_t key_public = {};
    ber_privkey_to_der_pubkey(key, key_public);

    ot_array_128_t rep_b = {};
    rsa_raw_decrypt_1024(sig, key_public, rep_b);

    if (rep_a != rep_b) {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "check of signture failed!");
    }

    ot_data_t rep2(rep_b.begin(), rep_b.end());

    rsa_verify_pkcs1_pss_mgf1_sha256(rep2, dgst);

#endif
}

// static
void OTCryptoPP_pvt::verify_rsa_pss_sha256_samy(const ot_data_t& msg,
                                                const ot_data_t& key,
                                                const ot_array_128_t& sig)
{
    ot_array_128_t rep_a = {};
    rsa_raw_decrypt_1024(sig, key, rep_a);

    ot_data_t rep(rep_a.begin(), rep_a.end());

    ot_array_32_t dgst = {};
    hash_samy(msg, dgst);

    rsa_verify_pkcs1_pss_mgf1_sha256(rep, dgst);
}

// static
void OTCryptoPP_pvt::encrypt_rsa_pkcs1(const ot_data_secure_t& msg,
                                       const ot_data_t& key,
                                       ot_array_128_t& out)
{
    CryptoPP::RSAES_PKCS1v15_Encryptor enc;

    enc.AccessMaterial().Load(
        CryptoPP::ArraySource(key.data(), key.size(), true));

    CryptoPP::AutoSeededRandomPool rng;
    if (!enc.AccessMaterial().Validate(rng, 2)) {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "bad public key");
    }

    enc.Encrypt(rng, msg.data(), msg.size(), out.data());
}

// static
void OTCryptoPP_pvt::decrypt_rsa_pkcs1(const ot_array_128_t& msg,
                                       const ot_data_secure_t& key,
                                       ot_data_secure_t& out)
{
    CryptoPP::RSAES_PKCS1v15_Decryptor dec;

    dec.AccessMaterial().Load(
        CryptoPP::ArraySource(key.data(), key.size(), true));

    CryptoPP::AutoSeededRandomPool rng;
    if (!dec.AccessMaterial().Validate(rng, 2)) {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "bad public key");
    }

    out.resize(dec.MaxPlaintextLength(msg.size()));
    auto res = dec.Decrypt(rng, msg.data(), msg.size(), out.data());

    if (!res.isValidCoding) {
        throw Exception(Exception::DATA_INTEGRITY_CHECK_FAILED,
                        "bad decrypted result");
    }

    out.resize(res.messageLength);
}

} // namespace opentxs
