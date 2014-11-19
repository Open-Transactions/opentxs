/************************************************************
 *
 *  OTCryptoOpenSSL.cpp
 *
 *  Initial implementation of the abstract OTCrypto class based on OpenSSL.
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

#include <bitcoin-base58/hash.h> // for Hash()
#include <opentxs/core/crypto/BitcoinCrypto.hpp>
#include <opentxs/core/crypto/OTCryptoOpenSSL.hpp>
#include <opentxs/core/OTData.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/crypto/OTSignature.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/util/stacktrace.h>

#include <bitcoin-base58/base58.h>

#include <bigint/BigIntegerLibrary.hh>

#include <thread>

extern "C" {
#ifdef _WIN32
#else
#include <arpa/inet.h> // For htonl()
#endif
}

extern "C" {
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/dsa.h>
#include <openssl/err.h>
#include <openssl/ui.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/asn1.h>
#include <openssl/objects.h>
#include <openssl/ssl.h>
#include <openssl/sha.h>
#include <openssl/conf.h>
#include <openssl/x509v3.h>

#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif
}

#include <opentxs/core/crypto/OTAsymmetricKey_OpenSSLPrivdp.hpp>
#include <opentxs/core/crypto/OpenSSL_BIO.hpp>

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=36750
#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

// for BIO_flush and htons
#ifndef _WIN32
#pragma GCC diagnostic ignored "-Wold-style-cast"
#ifndef __clang__
#pragma GCC diagnostic warning "-Wuseless-cast"
#endif
#endif

namespace opentxs
{

// OpenSSL / Crypto-lib d-pointer
#if defined(OT_CRYPTO_USING_GPG)

// Someday    }:-)        OTCrypto_GPG

#elif defined(OT_CRYPTO_USING_OPENSSL)

class OTCrypto_OpenSSL::OTCrypto_OpenSSLdp
{
public:
    // These are protected because they contain OpenSSL-specific parameters.

    bool SignContractDefaultHash(const String& strContractUnsigned,
                                 const EVP_PKEY* pkey,
                                 OTSignature& theSignature, // output
                                 const OTPasswordData* pPWData = nullptr) const;

    bool VerifyContractDefaultHash(
        const String& strContractToVerify, const EVP_PKEY* pkey,
        const OTSignature& theSignature,
        const OTPasswordData* pPWData = nullptr) const;

    // Sign or verify using the actual OpenSSL EVP_PKEY
    //
    bool SignContract(const String& strContractUnsigned, const EVP_PKEY* pkey,
                      OTSignature& theSignature, // output
                      const String& strHashType,
                      const OTPasswordData* pPWData = nullptr) const;

    bool VerifySignature(const String& strContractToVerify,
                         const EVP_PKEY* pkey, const OTSignature& theSignature,
                         const String& strHashType,
                         const OTPasswordData* pPWData = nullptr) const;

    static const EVP_MD* GetOpenSSLDigestByName(const String& theName);
};

#else // Apparently NO crypto engine is defined!

// Perhaps error out here...

#endif // if defined (OT_CRYPTO_USING_OPENSSL), elif defined
// (OT_CRYPTO_USING_GPG), else, endif.

#if defined(OT_CRYPTO_USING_OPENSSL)

extern "C" {

#include <openssl/crypto.h>
#include <openssl/asn1.h>
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>

#include <openssl/opensslconf.h>
#include <openssl/opensslv.h>
}

OTCrypto_OpenSSL::OTCrypto_OpenSSL()
    : OTCrypto()
    , dp(nullptr)
{
}

OTCrypto_OpenSSL::~OTCrypto_OpenSSL()
{
}

/*
 #include <openssl/ssl.h>
 void SSL_load_error_strings(void);

 #include <openssl/err.h>
 void ERR_free_strings(void);
 //void ERR_load_crypto_strings(void);


 #include <openssl/ssl.h>
 int32_t SSL_library_init(void);
 //#define OpenSSL_add_ssl_algorithms()    SSL_library_init()
 //#define SSLeay_add_ssl_algorithms()     SSL_library_init()


 #include <openssl/evp.h>
 void OpenSSL_add_all_algorithms(void);
 //void OpenSSL_add_all_ciphers(void);
 //void OpenSSL_add_all_digests(void);
 void EVP_cleanup(void);


 #include <openssl/conf.h>
 void OPENSSL_config(const char* config_name);
 //void OPENSSL_no_config(void);
 //Applications should free up configuration at application closedown by calling
 CONF_modules_free().

 #include <openssl/conf.h>
 void CONF_modules_free(void);
 //void CONF_modules_finish(void);
 //void CONF_modules_unload(int32_t all);
 */

/*
#include <openssl/crypto.h>

/ Don't use this structure directly.
typedef struct crypto_threadid_st
{
void *ptr;
uint64_t val;
} CRYPTO_THREADID;

// Only use CRYPTO_THREADID_set_[numeric|pointer]() within callbacks
void CRYPTO_THREADID_set_numeric(CRYPTO_THREADID* id, uint64_t val);
void CRYPTO_THREADID_set_pointer(CRYPTO_THREADID* id, void* ptr);

int32_t CRYPTO_THREADID_set_callback(void (*threadid_func)(CRYPTO_THREADID *));

void (*CRYPTO_THREADID_get_callback(void))(CRYPTO_THREADID *);

void CRYPTO_THREADID_current(CRYPTO_THREADID* id);

int32_t CRYPTO_THREADID_cmp(const CRYPTO_THREADID* a, const CRYPTO_THREADID* b);
void CRYPTO_THREADID_cpy(CRYPTO_THREADID* dest, const CRYPTO_THREADID* src);

uint64_t CRYPTO_THREADID_hash(const CRYPTO_THREADID* id);

int32_t CRYPTO_num_locks(void);

Description


OpenSSL can safely be used in multi-threaded applications provided that at
least two callback functions are set,
locking_function and threadid_func.

locking_function(int32_t mode, int32_t n, const char* file, int32_t line) is
needed to perform locking on shared data structures.
(Note that OpenSSL uses a number of global data structures that will be
implicitly shared whenever multiple threads
use OpenSSL.) Multi-threaded applications will crash at random if it is not
set.

locking_function() must be able to handle up to CRYPTO_num_locks() different
mutex locks. It sets the n-th lock if
mode & CRYPTO_LOCK , and releases it otherwise.

file and line are the file number of the function setting the lock. They can be
useful for debugging.

threadid_func(CRYPTO_THREADID* id) is needed to record the currently-executing
thread's identifier into id. The
implementation of this callback should not fill in id directly, but should use
CRYPTO_THREADID_set_numeric() if
thread IDs are numeric, or CRYPTO_THREADID_set_pointer() if they are
pointer-based. If the application does not
register such a callback using CRYPTO_THREADID_set_callback(), then a default
implementation is used - on Windows
and BeOS this uses the system's default thread identifying APIs, and on all
other platforms it uses the address
of errno. The latter is satisfactory for thread-safety if and only if the
platform has a thread-local error number
facility.
*/

/*

// struct CRYPTO_dynlock_value needs to be defined by the user
struct CRYPTO_dynlock_value;

void CRYPTO_set_dynlock_create_callback(struct CRYPTO_dynlock_value *
(*dyn_create_function)(char* file, int32_t line));
void CRYPTO_set_dynlock_lock_callback(void (*dyn_lock_function)
(int32_t mode, struct CRYPTO_dynlock_value *l, const char* file, int32_t
line));
void CRYPTO_set_dynlock_destroy_callback(void (*dyn_destroy_function)
(struct CRYPTO_dynlock_value *l, const char* file, int32_t line));

int32_t CRYPTO_get_new_dynlockid(void);

void CRYPTO_destroy_dynlockid(int32_t i);

void CRYPTO_lock(int32_t mode, int32_t n, const char* file, int32_t line);

#define CRYPTO_w_lock(type)    \
CRYPTO_lock(CRYPTO_LOCK|CRYPTO_WRITE,type,__FILE__,__LINE__)
#define CRYPTO_w_unlock(type)  \
CRYPTO_lock(CRYPTO_UNLOCK|CRYPTO_WRITE,type,__FILE__,__LINE__)
#define CRYPTO_r_lock(type)    \
CRYPTO_lock(CRYPTO_LOCK|CRYPTO_READ,type,__FILE__,__LINE__)
#define CRYPTO_r_unlock(type)  \
CRYPTO_lock(CRYPTO_UNLOCK|CRYPTO_READ,type,__FILE__,__LINE__)
#define CRYPTO_add(addr,amount,type)   \
CRYPTO_add_lock(addr,amount,type,__FILE__,__LINE__)

*/

std::mutex* OTCrypto_OpenSSL::s_arrayMutex = nullptr;

extern "C" {
#if OPENSSL_VERSION_NUMBER - 0 < 0x10000000L
unsigned long ot_openssl_thread_id(void);
#else
void ot_openssl_thread_id(CRYPTO_THREADID*);
#endif

void ot_openssl_locking_callback(int32_t mode, int32_t type, const char* file,
                                 int32_t line);
}

// done
/*
 threadid_func(CRYPTO_THREADID* id) is needed to record the currently-executing
 thread's identifier into id.
 The implementation of this callback should not fill in id directly, but should
 use CRYPTO_THREADID_set_numeric()
 if thread IDs are numeric, or CRYPTO_THREADID_set_pointer() if they are
 pointer-based. If the application does
 not register such a callback using CRYPTO_THREADID_set_callback(), then a
 default implementation is used - on
 Windows and BeOS this uses the system's default thread identifying APIs, and on
 all other platforms it uses the
 address of errno. The latter is satisfactory for thread-safety if and only if
 the platform has a thread-local
 error number facility.

 */

#if OPENSSL_VERSION_NUMBER - 0 < 0x10000000L
unsigned long ot_openssl_thread_id(void)
{
    uint64_t ret = std::hash<std::thread::id>()(std::this_thread::get_id());

    return (ret);
}

#else
void ot_openssl_thread_id(CRYPTO_THREADID* id)
{
    OT_ASSERT(nullptr != id);

    // TODO: Possibly do this by pointer instead of by uint64_t,
    // for certain platforms. (OpenSSL provides functions for both.)
    //

    unsigned long val =
        std::hash<std::thread::id>()(std::this_thread::get_id());

    //    void CRYPTO_THREADID_set_numeric(CRYPTO_THREADID* id, uint64_t val);
    //    void CRYPTO_THREADID_set_pointer(CRYPTO_THREADID* id, void* ptr);

    CRYPTO_THREADID_set_numeric(id, val);
}
#endif

/*
 locking_function(int32_t mode, int32_t n, const char* file, int32_t line) is
 needed to perform locking on
 shared data structures. (Note that OpenSSL uses a number of global data
 structures that will
 be implicitly shared whenever multiple threads use OpenSSL.) Multi-threaded
 applications will
 crash at random if it is not set.

 locking_function() must be able to handle up to CRYPTO_num_locks() different
 mutex locks. It
 sets the n-th lock if mode & CRYPTO_LOCK , and releases it otherwise.

 file and line are the file number of the function setting the lock. They can be
 useful for
 debugging.
 */

extern "C" {
void ot_openssl_locking_callback(int32_t mode, int32_t type, const char*,
                                 int32_t)
{
    if (mode & CRYPTO_LOCK) {
        OTCrypto_OpenSSL::s_arrayMutex[type].lock();
    }
    else {
        OTCrypto_OpenSSL::s_arrayMutex[type].unlock();
    }
}
} // extern "C"

// virtual
bool OTCrypto_OpenSSL::Base64Encode(const ot_data_t& theInput,
                                    std::string& strOutput,
                                    bool bLineBreaks) const
{
    strOutput.clear();

    char* buf = nullptr;
    BUF_MEM* bptr = nullptr;

    OpenSSL_BIO b64 = BIO_new(BIO_f_base64());

    if (!b64) return buf;

    if (!bLineBreaks) BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    OpenSSL_BIO bmem = BIO_new(BIO_s_mem());

    if (bmem) {
        OpenSSL_BIO b64join = BIO_push(b64, bmem);
        b64.release();
        bmem.release();

        if (BIO_write(b64join, theInput.data(), theInput.size()) ==
            static_cast<int64_t>(theInput.size())) {
            (void)BIO_flush(b64join);
#ifndef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#endif
            BIO_get_mem_ptr(b64join, &bptr);
#ifndef _WIN32
#pragma GCC diagnostic pop
#endif
            //    otLog5 << "DEBUG base64_encode size: %" PRId64 ",  in_len:
            // %" PRId64 "\n", bptr->length+1, in_len);
            buf = new char[bptr->length + 1];
            OT_ASSERT(nullptr != buf);
            memcpy(buf, bptr->data, bptr->length); // Safe.
            buf[bptr->length] = '\0';              // Forcing null terminator.
        }
    }
    else {
        OT_FAIL_MSG("Failed creating new Bio in base64_encode.\n");
    }

    strOutput = buf;
    delete[] buf;

    return true;
}

// virtual
bool OTCrypto_OpenSSL::Base64Decode(const std::string& strInput,
                                    ot_data_t& theOutput,
                                    bool bLineBreaks) const
{
    theOutput.clear();

    size_t out_len = 0;
    OT_ASSERT(!strInput.empty());

    int32_t in_len = strInput.size();
    int32_t out_max_len = (in_len * 6 + 7) / 8;
    uint8_t* buf = new uint8_t[out_max_len];
    OT_ASSERT(nullptr != buf);
    memset(buf, 0, out_max_len); // todo security

    OpenSSL_BIO b64 = BIO_new(BIO_f_base64());

    if (b64) {
        if (!bLineBreaks) BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

        OpenSSL_BIO bmem = BIO_new_mem_buf(const_cast<char*>(strInput.data()),
                                           strInput.size());
        OT_ASSERT(nullptr != bmem);

        OpenSSL_BIO b64join = BIO_push(b64, bmem);
        b64.release();
        bmem.release();
        OT_ASSERT(nullptr != b64join);

        out_len = BIO_read(b64join, buf, out_max_len);

    }
    else {
        OT_FAIL_MSG("Failed creating new Bio in base64_decode.\n");
    }

    theOutput.resize(out_len);
    theOutput.assign(buf, buf + out_len);
    delete[] buf;

    return true;
}

// Decode formatted OT ID to the binary hash ID.
void OTCrypto_OpenSSL::SetIDFromEncoded(const String& strInput,
                                        Identifier& theOutput) const
{
    theOutput.clear();

    // If it's short, no validate.
    if (strInput.GetLength() < 4) return;

    ot_data_t decoded;
    bool success = IdentifierFormat::decode(strInput.Get(), decoded);

    if (success) {
        theOutput.resize(decoded.size());
        theOutput.assign(decoded.begin(), decoded.end());
    }
}

// Encode binary hash ID to the formatted OT ID.
void OTCrypto_OpenSSL::EncodeID(const Identifier& theInput,
                                String& strOutput) const
{
    strOutput.Release();

    if (theInput.empty()) return;

    auto inputPtr = static_cast<const unsigned char*>(theInput.data());
    strOutput.Set(IdentifierFormat::encode(inputPtr, theInput.size()).c_str());
}

bool OTCrypto_OpenSSL::RandomizeMemory(uint8_t* szDestination,
                                       uint32_t nNewSize) const
{
    OT_ASSERT(nullptr != szDestination);
    OT_ASSERT(nNewSize > 0);

    /*
     RAND_bytes() returns 1 on success, 0 otherwise. The error code can be
     obtained by ERR_get_error(3).
     RAND_pseudo_bytes() returns 1 if the bytes generated are cryptographically
     strong, 0 otherwise.
     Both functions return -1 if they are not supported by the current RAND
     method.
     */
    const int32_t nRAND_bytes =
        RAND_bytes(szDestination, static_cast<int32_t>(nNewSize));

    if ((-1) == nRAND_bytes) {
        otErr
            << __FUNCTION__
            << ": ERROR: RAND_bytes is apparently not supported by the current "
               "RAND method. OpenSSL: "
            << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return false;
    }
    else if (0 == nRAND_bytes) {
        otErr << __FUNCTION__
              << ": Failed: The PRNG is apparently not seeded. OpenSSL error: "
              << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return false;
    }

    return true;
}

OTPassword* OTCrypto_OpenSSL::DeriveNewKey(const OTPassword& userPassword,
                                           const ot_data_t& dataSalt,
                                           uint32_t uIterations,
                                           ot_data_t& dataCheckHash) const
{
    //  OT_ASSERT(userPassword.isPassword());
    OT_ASSERT(!dataSalt.empty());

    otInfo << __FUNCTION__
           << ": Using a text passphrase, salt, and iteration count, "
              "to make a derived key...\n";

    OTPassword* pDerivedKey(InstantiateBinarySecret()); // already asserts.

    //  pDerivedKey MUST be returned or cleaned-up, below this point.
    //
    // Key derivation in OpenSSL.
    //
    // int32_t PKCS5_PBKDF2_HMAC_SHA1(const char*, int32_t, const uint8_t*,
    // int32_t, int32_t, int32_t, uint8_t*)
    //
    PKCS5_PBKDF2_HMAC_SHA1(
        reinterpret_cast<const char*> // If is password... supply password,
                                      // otherwise supply memory.
        (userPassword.isPassword() ? userPassword.getPassword_uint8()
                                   : userPassword.getMemory_uint8()),
        static_cast<const int32_t>(
            userPassword.isPassword()
                ? userPassword.getPasswordSize()
                : userPassword.getMemorySize()), // Password Length
        dataSalt.data(),                         // Salt Data
        dataSalt.size(),                         // Salt Length
        static_cast<const int32_t>(uIterations), // Number Of Iterations
        static_cast<const int32_t>(
            pDerivedKey->getMemorySize()), // Output Length
        static_cast<uint8_t*>(
            pDerivedKey->getMemoryWritable()) // Output Key (not const!)
        );

    // For The HashCheck
    bool bHaveCheckHash = !dataCheckHash.empty();

    ot_data_t tmpHashCheck;
    tmpHashCheck.resize(OTCryptoConfig::SymmetricKeySize());

    // We take the DerivedKey, and hash it again, then get a 'hash-check'
    // Compare that with the supplied one, (if there is one).
    // If there isn't one, we return the

    PKCS5_PBKDF2_HMAC_SHA1(
        reinterpret_cast<const char*>(pDerivedKey->getMemory()), // Derived Key
        static_cast<const int32_t>(
            pDerivedKey->getMemorySize()),               // Password Length
        dataSalt.data(),                                 // Salt Data
        static_cast<const int32_t>(dataSalt.size()),     // Salt Length
        static_cast<const int32_t>(uIterations),         // Number Of Iterations
        static_cast<const int32_t>(tmpHashCheck.size()), // Output Length
        tmpHashCheck.data()) // Output Key (not const!)
        ;

    if (bHaveCheckHash) {
        String strDataCheck, strTestCheck;
        strDataCheck.Set(reinterpret_cast<const char*>(dataCheckHash.data()),
                         dataCheckHash.size());
        strTestCheck.Set(reinterpret_cast<const char*>(tmpHashCheck.data()),
                         tmpHashCheck.size());

        if (!strDataCheck.Compare(strTestCheck)) {
            dataCheckHash.clear();
            dataCheckHash = tmpHashCheck;
            return nullptr; // failure (but we will return the dataCheckHash we
                            // got
                            // anyway)
        }
    }
    else {
        dataCheckHash.clear();
        dataCheckHash = tmpHashCheck;
    }

    return pDerivedKey;
}

/*
 openssl dgst -sha1 \
 -sign clientkey.pem \
 -out cheesy2.sig \
 cheesy2.xml

 openssl dgst -sha1 \
 -verify clientcert.pem \
 -signature cheesy2.sig \
 cheesy2.xml


openssl x509 -in clientcert.pem -pubkey -noout > clientpub.pem

 Then verification using the public key works as expected:

openssl dgst -sha1 -verify clientpub.pem -signature cheesy2.sig  cheesy2.xml

 Verified OK


 openssl enc -base64 -out cheesy2.b64 cheesy2.sig

 */

// static
const EVP_MD* OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::GetOpenSSLDigestByName(
    const String& theName)
{
    if (theName.Compare("SHA1"))
        return EVP_sha1();
    else if (theName.Compare("SHA224"))
        return EVP_sha224();
    else if (theName.Compare("SHA256"))
        return EVP_sha256();
    else if (theName.Compare("SHA384"))
        return EVP_sha384();
    else if (theName.Compare("SHA512"))
        return EVP_sha512();
    return nullptr;
}

/*
 SHA256_CTX context;
 uint8_t md[SHA256_DIGEST_LENGTH];

 SHA256_Init(&context);
 SHA256_Update(&context, (uint8_t*)input, length);
 SHA256_Final(md, &context);
 */

// (To instantiate a text secret, just do this:  OTPassword thePassword;)

// Caller MUST delete!
// todo return a smartpointer here.
OTPassword* OTCrypto_OpenSSL::InstantiateBinarySecret() const
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

#ifndef _PASSWORD_LEN
#define _PASSWORD_LEN 128
#endif

bool OTCrypto_OpenSSL::GetPasswordFromConsoleLowLevel(
    OTPassword& theOutput, const char* szPrompt) const
{
    OT_ASSERT(nullptr != szPrompt);

#ifdef _WIN32
    {
        std::cout << szPrompt;

        {
            std::string strPassword = "";

#ifdef UNICODE

            const wchar_t enter[] = {L'\x000D', L'\x0000'}; // carrage return
            const std::wstring wstrENTER = enter;

            std::wstring wstrPass = L"";

            for (;;) {
                const wchar_t ch[] = {_getwch(), L'\x0000'};
                const std::wstring wstrCH = ch;
                if (wstrENTER == wstrCH) break;
                wstrPass.append(wstrCH);
            }
            strPassword = String::ws2s(wstrPass);

#else

            const char enter[] = {'\x0D', '\x00'}; // carrage return
            const std::string strENTER = enter;

            std::string strPass = "";

            for (;;) {
                const char ch[] = {_getch(), '\x00'};
                const std::string strCH = ch;
                if (strENTER == strCH) break;
                strPass.append(strCH);
            }
            strPassword = strPass;

#endif
            theOutput.setPassword(
                strPassword.c_str(),
                static_cast<int32_t>(strPassword.length() - 1));
        }

        std::cout << std::endl; // new line.
        return true;
    }
#elif defined(OT_CRYPTO_USING_OPENSSL)
    // todo security: might want to allow to set OTPassword's size and copy
    // directly into it,
    // so that we aren't using this temp buf in between, which, although we're
    // zeroing it, could
    // technically end up getting swapped to disk.
    //
    {
        char buf[_PASSWORD_LEN + 10] = "", buff[_PASSWORD_LEN + 10] = "";

        if (UI_UTIL_read_pw(buf, buff, _PASSWORD_LEN, szPrompt, 0) == 0) {
            size_t nPassLength = String::safe_strlen(buf, _PASSWORD_LEN);
            theOutput.setPassword_uint8(reinterpret_cast<uint8_t*>(buf),
                                        nPassLength);
            OTPassword::zeroMemory(buf, nPassLength);
            OTPassword::zeroMemory(buff, nPassLength);
            return true;
        }
        else
            return false;
    }
#else
    {
        otErr << "__FUNCTION__: Open-Transactions is not compiled to collect "
              << "the passphrase from the console!\n";
        return false;
    }
#endif
}

void OTCrypto_OpenSSL::thread_setup() const
{
    OTCrypto_OpenSSL::s_arrayMutex = new std::mutex[CRYPTO_num_locks()];

// NOTE: OpenSSL supposedly has some default implementation for the thread_id,
// so we're going to NOT set that callback here, and see what happens.
//
// UPDATE: Looks like this works "if and only if the local system provides
// errno"
// and since I already have a supposedly-reliable ID from tinythread++, I'm
// going
// to just use that one for now and see how it works.
//
#if OPENSSL_VERSION_NUMBER - 0 < 0x10000000L
    CRYPTO_set_id_callback(ot_openssl_thread_id);
#else
    int32_t nResult = CRYPTO_THREADID_set_callback(ot_openssl_thread_id);
    ++nResult;
    --nResult;
#endif

    // Here we set the locking callback function, which is the same for all
    // versions
    // of OpenSSL. (Unlike thread_id function above.)
    //
    CRYPTO_set_locking_callback(ot_openssl_locking_callback);
}

// done

void OTCrypto_OpenSSL::thread_cleanup() const
{
    CRYPTO_set_locking_callback(nullptr);

    if (nullptr != OTCrypto_OpenSSL::s_arrayMutex) {
        delete[] OTCrypto_OpenSSL::s_arrayMutex;
    }

    OTCrypto_OpenSSL::s_arrayMutex = nullptr;
}

void OTCrypto_OpenSSL::Init_Override() const
{
    const char* szFunc = "OTCrypto_OpenSSL::Init_Override";

    otWarn << szFunc << ": Setting up OpenSSL:  SSL_library_init, error "
                        "strings and algorithms, and OpenSSL config...\n";

/*
 OPENSSL_VERSION_NUMBER is a numeric release version identifier:

 MMNNFFPPS: major minor fix patch status
 The status nibble has one of the values 0 for development, 1 to e for betas 1
 to 14, and f for release.

 for example

 0x000906000 == 0.9.6 dev
 0x000906023 == 0.9.6b beta 3
 0x00090605f == 0.9.6e release
 Versions prior to 0.9.3 have identifiers < 0x0930. Versions between 0.9.3 and
 0.9.5 had a version identifier with this interpretation:

 MMNNFFRBB major minor fix final beta/patch
 for example

 0x000904100 == 0.9.4 release
 0x000905000 == 0.9.5 dev
 Version 0.9.5a had an interim interpretation that is like the current one,
 except the patch level got the highest bit set, to keep continuity. The number
 was therefore 0x0090581f.

 For backward compatibility, SSLEAY_VERSION_NUMBER is also defined.

 */
#if !defined(OPENSSL_VERSION_NUMBER) || OPENSSL_VERSION_NUMBER - 0 < 0x10000000L
    OT_FAIL_MSG("ASSERT: Must use OpenSSL version 1.0.0 or higher.\n");
#endif

/* Todo FYI:
 - One final comment about compiling applications linked to the OpenSSL library.
 - If you don't use the multithreaded DLL runtime library (/MD option) your
 - program will almost certainly crash because malloc gets confused -- the
 - OpenSSL DLLs are statically linked to one version, the application must
 - not use a different one.  You might be able to work around such problems
 - by adding CRYPTO_malloc_init() to your program before any calls to the
 - OpenSSL libraries: This tells the OpenSSL libraries to use the same
 - malloc(), free() and realloc() as the application.  However there are many
 - standard library functions used by OpenSSL that call malloc() internally
 - (e.g. fopen()), and OpenSSL cannot change these; so in general you cannot
 - rely on CRYPTO_malloc_init() solving your problem, and you should
 - consistently use the multithreaded library.
 */
#ifdef _WIN32
    CRYPTO_malloc_init(); //      # -1
// FYI: this call appeared in the client version, not the server version.
// but now it will obviously appear in both, since they both will just call this
// (OT_Init.)
// Therefore if any weird errors crop in the server, just be aware. This call
// might have been
// specifically for DLLs or something.
#endif
    // SSL_library_init() must be called before any other action takes place.
    // SSL_library_init() is not reentrant.
    //
    SSL_library_init(); //     #0

    /*
     We all owe a debt of gratitude to the OpenSSL team but fuck is their
     documentation
     difficult!! In this case I am trying to figure out whether I should call
     SSL_library_init()
     first, or SSL_load_error_strings() first.
     Docs say:

     EXAMPLES   (http://www.openssl.org/docs/ssl/SSL_library_init.html#)

     A typical TLS/SSL application will start with the library initialization,
     and provide readable error messages.

     SSL_load_error_strings();               // readable error messages
     SSL_library_init();                      // initialize library
     -----------
     ===> NOTICE it said "START" with library initialization, "AND" provide
     readable error messages... But then what order does it PUT them in?

     SSL_load_error_strings();        // readable error messages
     SSL_library_init();              // initialize library
     -------

     ON THE SAME PAGE, as if things weren't confusing enough, see THIS:

     NOTES
     SSL_library_init() must be called before any other action takes place.
     SSL_library_init() is not reentrant.
     -------------------
     Then, on http://www.openssl.org/docs/crypto/ERR_load_crypto_strings.html#,
     in
     reference to SSL_load_error_strings and ERR_load_crypto_strings, it says:

     One of these functions should be called BEFORE generating textual error
     messages.

     ====>  ?? Huh?? So which should I call first? Ben Laurie, if you are ever
     googling your
     own name on the Internet, please drop me a line and lemme know:
     fellowtraveler around rayservers cough net
     */

    // NOTE: the below sections are numbered #1, #2, #3, etc so that they can be
    // UNROLLED
    // IN THE OPPOSITE ORDER when we get to OT_Cleanup().

    /*
     - ERR_load_crypto_strings() registers the error strings for all libcrypto
     functions.
     - SSL_load_error_strings() does the same, but also registers the libssl
     error strings.
     One of these functions should be called before generating textual error
     messages.
     - ERR_free_strings() frees all previously loaded error strings.
     */

    SSL_load_error_strings(); // DONE -- corresponds to ERR_free_strings in
                              // OT_Cleanup()   #1

    //  ERR_load_crypto_strings();   // Redundant -- SSL_load_error_strings does
    // this already.
    //
    /*
     OpenSSL keeps an internal table of digest algorithms and ciphers.
     It uses this table to lookup ciphers via functions such as
     EVP_get_cipher_byname().

     OpenSSL_add_all_algorithms() adds all algorithms to the table (digests and
     ciphers).

     OpenSSL_add_all_digests() adds all digest algorithms to the table.
     OpenSSL_add_all_ciphers() adds all encryption algorithms to the table
     including password based encryption algorithms.

     TODO optimization:
     Calling OpenSSL_add_all_algorithms() links in all algorithms: as a result a
     statically linked executable
     can be quite large. If this is important it is possible to just add the
     required ciphers and digests.
     -- Thought: I will probably have different optimization options. Some
     things will be done no matter what, but
     other things will be compile-flags for optimizing specifically for speed,
     or size, or use of RAM, or CPU cycles,
     or security options, etc. This is one example of something where I would
     optimize it out, if possible, when trying
     to conserve RAM.
     Note: However it seems from the docs, that this table needs to be populated
     anyway due to problems in
     OpenSSL when it's not.
     */

    /*
    Try to activate OpenSSL debug memory procedure:
        OpenSSL_BIO pbio = BIO_new(BIO_s_file());
        BIO_set_fp(out,stdout,BIO_NOCLOSE);
        CRYPTO_malloc_debug_init();
        MemCheck_start();
        MemCheck_on();

        .
        .
        .
        MemCheck_off()
        MemCheck_stop()
        CRYPTO_mem_leaks(pbio);

     This will print out to stdout all memory that has been not deallocated.

     Put starting part before everything ( even before
    OpenSSL_add_all_algorithms() call)
     this way you will see everything.

     */

    OpenSSL_add_all_algorithms(); // DONE -- corresponds to EVP_cleanup() in
                                  // OT_Cleanup().    #2

//
//
// RAND
//
/*
 RAND_bytes() automatically calls RAND_poll() if it has not already been done at
 least once.
 So you do not have to call it yourself. RAND_poll() feeds on what the operating
 system provides:
 on Linux, Solaris, FreeBSD and similar Unix-like systems, it will use
 /dev/urandom (or /dev/random
 if there is no /dev/urandom) to obtain a cryptographically secure initial seed;
 on Windows, it will
 call CryptGenRandom() for the same effect.

 RAND_screen() is provided by OpenSSL only for backward compatibility with
 (much) older code which
 may call it (that was before OpenSSL used proper OS-based seed initialization).

 So the "normal" way of dealing with RAND_poll() and RAND_screen() is to call
 neither. Just use RAND_bytes()
 and be happy.

 RESPONSE: Thanks for the detailed answer. In regards to your suggestion to call
 neither, the problem
 under Windows is that RAND_poll can take some time and will block our UI. So we
 call it upon initialization,
 which works for us.
 */
// I guess Windows will seed the PRNG whenever someone tries to get
// some RAND_bytes() the first time...
//
//#ifdef _WIN32
// CORRESPONDS to RAND_cleanup in OT_Cleanup().
//      RAND_screen();
//#else
// note: optimization: might want to remove this, since supposedly it happens
// anyway
// when you use RAND_bytes. So the "lazy evaluation" rule would seem to imply,
// not bothering
// to slow things down NOW, since it's not really needed until THEN.
//

#if defined(USE_RAND_POLL)

    RAND_poll(); //                                   #3

#endif

    // OPENSSL_config()                                             #4
    //
    // OPENSSL_config configures OpenSSL using the standard openssl.cnf
    // configuration file name
    // using config_name. If config_name is nullptr then the default name
    // openssl_conf will be used.
    // Any errors are ignored. Further calls to OPENSSL_config() will have no
    // effect. The configuration
    // file format is documented in the conf(5) manual page.
    //

    OPENSSL_config(
        nullptr); // const char *config_name = nullptr: the default name
                  // openssl_conf will be used.

    //
    // Corresponds to CONF_modules_free() in OT_Cleanup().
    //

    //
    // Let's see 'em!
    //
    ERR_print_errors_fp(stderr);
//

//
//
// THREADS
//
//

#if defined(OPENSSL_THREADS)
    // thread support enabled

    otWarn << szFunc << ": OpenSSL WAS compiled with thread support, FYI. "
                        "Setting up mutexes...\n";

    thread_setup();

#else
    // no thread support

    otErr << __FUNCTION__
          << ": WARNING: OpenSSL was NOT compiled with thread support. "
          << "(Also: Master Key will not expire.)\n";

#endif
}

// RAND_status() and RAND_event() return 1 if the PRNG has been seeded with
// enough data, 0 otherwise.

/*
 13. I think I've detected a memory leak, is this a bug?

 In most cases the cause of an apparent memory leak is an OpenSSL internal
 table that is allocated when an application starts up. Since such tables do
 not grow in size over time they are harmless.

 These internal tables can be freed up when an application closes using
 various functions. Currently these include following:

 Thread-local cleanup functions:

 ERR_remove_state()

 Application-global cleanup functions that are aware of usage (and therefore
 thread-safe):

 ENGINE_cleanup() and CONF_modules_unload()

 "Brutal" (thread-unsafe) Application-global cleanup functions:

 ERR_free_strings(), EVP_cleanup() and CRYPTO_cleanup_all_ex_data().
 */

void OTCrypto_OpenSSL::Cleanup_Override() const
{
    const char* szFunc = "OTCrypto_OpenSSL::Cleanup_Override";

    otLog4 << szFunc << ": Cleaning up OpenSSL...\n";

// In the future if we start using ENGINEs, then do the cleanup here:
//#ifndef OPENSSL_NO_ENGINE
//  void ENGINE_cleanup(void);
//#endif
//

#if defined(OPENSSL_THREADS)
    // thread support enabled

    thread_cleanup();

#else
// no thread support

#endif

    /*
     CONF_modules_free()

     OpenSSL configuration cleanup function. CONF_modules_free() closes down and
     frees
     up all memory allocated by all configuration modules.
     Normally applications will only call CONF_modules_free() at application
     [shutdown]
     to tidy up any configuration performed.
     */
    CONF_modules_free(); // CORRESPONDS to: OPENSSL_config() in OT_Init().   #4

    RAND_cleanup(); // Corresponds to RAND_screen / RAND_poll in OT_Init()  #3

    EVP_cleanup(); // DONE (brutal) -- corresponds to OpenSSL_add_all_algorithms
                   // in OT_Init(). #2

    CRYPTO_cleanup_all_ex_data(); // (brutal)
                                  //    CRYPTO_mem_leaks(bio_err);

    ERR_free_strings(); // DONE (brutal) -- corresponds to
                        // SSL_load_error_strings in OT_Init().  #1

// ERR_remove_state - free a thread's error queue "prevents memory leaks..."
//
// ERR_remove_state() frees the error queue associated with thread pid. If
// pid == 0,
// the current thread will have its error queue removed.
//
// Since error queue data structures are allocated automatically for new
// threads,
// they must be freed when threads are terminated in order to avoid memory
// leaks.
//
#if OPENSSL_VERSION_NUMBER - 0 < 0x10000000L
    ERR_remove_state(0);
#else
    ERR_remove_thread_state(nullptr);
#endif

    /*
    +     Note that ERR_remove_state() is now deprecated, because it is tied
    +     to the assumption that thread IDs are numeric.  ERR_remove_state(0)
    +     to free the current thread's error state should be replaced by
    +     ERR_remove_thread_state(nullptr).
    */

    // NOTE: You must call SSL_shutdown() before you call SSL_free().
    // Update: these are for SSL sockets, they must be called per socket.
    // (IOW: Not needed here for app cleanup.)
}

// #define OTCryptoConfig::SymmetricBufferSize()   default: 4096

bool OTCrypto_OpenSSL::Encrypt(
    const OTPassword& theRawSymmetricKey, // The symmetric key, in clear form.
    const char* szInput,                  // This is the Plaintext.
    const uint32_t lInputLength, const ot_data_t& theIV, // (We assume this IV
    // is already generated
    // and passed in.)
    ot_data_t& theEncryptedOutput) const // OUTPUT. (Ciphertext.)
{
    const char* szFunc = "OTCrypto_OpenSSL::Encrypt";

    OT_ASSERT(OTCryptoConfig::SymmetricIvSize() == theIV.size());
    OT_ASSERT(OTCryptoConfig::SymmetricKeySize() ==
              theRawSymmetricKey.getMemorySize());
    OT_ASSERT(nullptr != szInput);
    OT_ASSERT(lInputLength > 0);

    EVP_CIPHER_CTX ctx;

    ot_data_t vBuffer = {}, vBuffer_out = {};

    vBuffer.resize(OTCryptoConfig::SymmetricBufferSize());
    vBuffer_out.resize(OTCryptoConfig::SymmetricBufferSize() +
                       EVP_MAX_IV_LENGTH);

    int32_t len_out = 0;

    //
    // This is where the envelope final contents will be placed.
    // including the size of the IV, the IV itself, and the ciphertext.
    //
    theEncryptedOutput.clear();

    class _OTEnv_Enc_stat
    {
    private:
        const char* m_szFunc;
        EVP_CIPHER_CTX& m_ctx;

    public:
        _OTEnv_Enc_stat(const char* param_szFunc, EVP_CIPHER_CTX& param_ctx)
            : m_szFunc(param_szFunc)
            , m_ctx(param_ctx)
        {
            OT_ASSERT(nullptr != param_szFunc);

            EVP_CIPHER_CTX_init(&m_ctx);
        }
        ~_OTEnv_Enc_stat()
        {
            // EVP_CIPHER_CTX_cleanup returns 1 for success and 0 for failure.
            //
            if (0 == EVP_CIPHER_CTX_cleanup(&m_ctx))
                otErr << m_szFunc << ": Failure in EVP_CIPHER_CTX_cleanup. (It "
                                     "returned 0.)\n";

            m_szFunc = nullptr; // keep the static analyzer happy
        }
    };
    _OTEnv_Enc_stat theInstance(szFunc, ctx);

    const EVP_CIPHER* cipher_type = EVP_aes_128_cbc(); // todo hardcoding.

    if (!EVP_EncryptInit(
            &ctx, cipher_type,
            const_cast<uint8_t*>(theRawSymmetricKey.getMemory_uint8()),
            theIV.data())) {
        otErr << szFunc << ": EVP_EncryptInit: failed.\n";
        return false;
    }

    // Now we process the input and write the encrypted data to
    // the output.
    //
    uint32_t lRemainingLength = lInputLength;
    uint32_t lCurrentIndex = 0;

    while (lRemainingLength > 0) {
        // If the remaining length is less than the default buffer size, then
        // set len to remaining length.
        // else if remaining length is larger than or equal to default buffer
        // size, then use the default buffer size.
        // Resulting value stored in len.
        //

        uint32_t len =
            ((lRemainingLength < OTCryptoConfig::SymmetricBufferSize())
                 ? lRemainingLength
                 : OTCryptoConfig::SymmetricBufferSize()); // 4096

        if (!EVP_EncryptUpdate(
                &ctx, vBuffer_out.data(), &len_out,
                const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(
                    &(szInput[lCurrentIndex]))),
                len)) {
            otErr << szFunc << ": EVP_EncryptUpdate: failed.\n";
            return false;
        }
        lRemainingLength -= len;
        lCurrentIndex += len;

        if (len_out > 0)
            theEncryptedOutput.insert(theEncryptedOutput.end(),
                                      vBuffer_out.begin(),
                                      vBuffer_out.begin() + len_out);
    }

    if (!EVP_EncryptFinal(&ctx, &vBuffer_out.at(0), &len_out)) {
        otErr << szFunc << ": EVP_EncryptFinal: failed.\n";
        return false;
    }

    // This is the "final" piece that is added from EncryptFinal just above.
    //
    if (len_out > 0)
        theEncryptedOutput.insert(theEncryptedOutput.end(), vBuffer_out.begin(),
                                  vBuffer_out.begin() + len_out);

    return true;
}

bool OTCrypto_OpenSSL::Decrypt(
    const OTPassword& theRawSymmetricKey, // The symmetric key, in clear form.
    const char* szInput,                  // This is the Ciphertext.
    const uint32_t lInputLength, const ot_data_t& theIV, // (We assume this IV
                                                         // is already generated
                                                         // and passed in.)
    OTCrypto_Decrypt_Output theDecryptedOutput) const    // OUTPUT. (Recovered
                                                         // plaintext.) You can
                                                         // pass OTPassword& OR
{
    const char* szFunc = "OTCrypto_OpenSSL::Decrypt";

    OT_ASSERT(OTCryptoConfig::SymmetricIvSize() == theIV.size());
    OT_ASSERT(OTCryptoConfig::SymmetricKeySize() ==
              theRawSymmetricKey.getMemorySize());
    OT_ASSERT(nullptr != szInput);
    OT_ASSERT(lInputLength > 0);

    EVP_CIPHER_CTX ctx;

    std::vector<uint8_t> vBuffer(OTCryptoConfig::SymmetricBufferSize()); // 4096
    std::vector<uint8_t> vBuffer_out(OTCryptoConfig::SymmetricBufferSize() +
                                     EVP_MAX_IV_LENGTH);
    int32_t len_out = 0;

    memset(&vBuffer.at(0), 0, OTCryptoConfig::SymmetricBufferSize());
    memset(&vBuffer_out.at(0), 0,
           OTCryptoConfig::SymmetricBufferSize() + EVP_MAX_IV_LENGTH);

    //
    // This is where the plaintext results will be placed.
    //
    theDecryptedOutput.Release();

    class _OTEnv_Dec_stat
    {
    private:
        const char* m_szFunc;
        EVP_CIPHER_CTX& m_ctx;

    public:
        _OTEnv_Dec_stat(const char* param_szFunc, EVP_CIPHER_CTX& param_ctx)
            : m_szFunc(param_szFunc)
            , m_ctx(param_ctx)
        {
            OT_ASSERT(nullptr != param_szFunc);

            EVP_CIPHER_CTX_init(&m_ctx);
        }
        ~_OTEnv_Dec_stat()
        {
            // EVP_CIPHER_CTX_cleanup returns 1 for success and 0 for failure.
            //
            if (0 == EVP_CIPHER_CTX_cleanup(&m_ctx))
                otErr << m_szFunc << ": Failure in EVP_CIPHER_CTX_cleanup. (It "
                                     "returned 0.)\n";
            m_szFunc = nullptr; // to keep the static analyzer happy.
        }
    };
    _OTEnv_Dec_stat theInstance(szFunc, ctx);

    const EVP_CIPHER* cipher_type = EVP_aes_128_cbc();

    if (!EVP_DecryptInit(
            &ctx, cipher_type,
            const_cast<uint8_t*>(theRawSymmetricKey.getMemory_uint8()),
            theIV.data())) {
        otErr << szFunc << ": EVP_DecryptInit: failed.\n";
        return false;
    }

    // Now we process the input and write the decrypted data to
    // the output.
    //
    uint32_t lRemainingLength = lInputLength;
    uint32_t lCurrentIndex = 0;

    while (lRemainingLength > 0) {
        // If the remaining length is less than the default buffer size, then
        // set len to remaining length.
        // else if remaining length is larger than or equal to default buffer
        // size, then use the default buffer size.
        // Resulting value stored in len.
        //
        uint32_t len =
            (lRemainingLength < OTCryptoConfig::SymmetricBufferSize())
                ? lRemainingLength
                : OTCryptoConfig::SymmetricBufferSize(); // 4096
        lRemainingLength -= len;

        if (!EVP_DecryptUpdate(
                &ctx, &vBuffer_out.at(0), &len_out,
                const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(
                    &(szInput[lCurrentIndex]))),
                len)) {
            otErr << szFunc << ": EVP_DecryptUpdate: failed.\n";
            return false;
        }
        lCurrentIndex += len;

        if (len_out > 0)
            if (false ==
                theDecryptedOutput.Concatenate(
                    reinterpret_cast<void*>(&vBuffer_out.at(0)),
                    static_cast<uint32_t>(len_out))) {
                otErr << szFunc << ": Failure: theDecryptedOutput isn't large "
                                   "enough for the decrypted output (1).\n";
                return false;
            }
    }

    if (!EVP_DecryptFinal(&ctx, &vBuffer_out.at(0), &len_out)) {
        otErr << szFunc << ": EVP_DecryptFinal: failed.\n";
        return false;
    }

    // This is the "final" piece that is added from DecryptFinal just above.
    //
    if (len_out > 0)
        if (false ==
            theDecryptedOutput.Concatenate(
                reinterpret_cast<void*>(&vBuffer_out.at(0)),
                static_cast<uint32_t>(len_out))) {
            otErr << szFunc << ": Failure: theDecryptedOutput isn't large "
                               "enough for the decrypted output (2).\n";
            return false;
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

void encode_envelope(const envelope_t&, ot_data_t&);
void decode_envelope(const ot_data_t&, envelope_t&);

void encode_envelope(const envelope_t& envelope, ot_data_t& data)
{
    data.clear();

    // Type
    OTData::appendData<uint16_t>(htons(envelope.type), data);

    // Number of Addressees
    OTData::appendData<uint32_t>(htonl(envelope.addresses.size()), data);

    for (auto addr : envelope.addresses) {
        // Nym ID
        OTData::appendData<uint32_t>(htonl(addr.id.size() + 1), data);
        data.insert(data.end(), addr.id.begin(), addr.id.end());
        data.push_back(0); // null terminator for string.

        // Encrypted Key
        OTData::appendData<uint32_t>(htonl(addr.ek.size()), data);
        data.insert(data.end(), addr.ek.begin(), addr.ek.end());
    }

    // IV
    OTData::appendData<uint32_t>(htonl(envelope.iv.size()), data);
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
        out.type = ntohs(OTData::readData<uint16_t>(&data_it, data.end()));

        if (out.type != 1) {
            throw std::invalid_argument(
                "Type 1 is the only supported envelope type");
        }
    }

    // Number of Addressees
    {
        out.addresses.resize(
            ntohl(OTData::readData<uint32_t>(&data_it, data.end())));

        if (out.addresses.empty()) {
            throw std::invalid_argument("Must have at least one addressee");
        }
    }

    for (auto& addr : out.addresses) {
        // Nym ID
        {
            ot_data_t nym(
                ntohl(OTData::readData<uint32_t>(&data_it, data.end())));

            OTData::readDataVector(&data_it, data.end(), nym);

            std::string nym_temp(nym.begin(), nym.end());
            addr.id.clear();
            addr.id = nym_temp.c_str();
        }

        // Encrypted Key
        {
            addr.ek.clear();
            addr.ek.resize(
                ntohl(OTData::readData<uint32_t>(&data_it, data.end())));
            OTData::readDataVector(&data_it, data.end(), addr.ek);

            if (addr.ek.empty()) {
                throw std::invalid_argument("Key must not be empty!");
            }
        }
    }

    // IV
    {
        out.iv.clear();
        out.iv.resize(ntohl(OTData::readData<uint32_t>(&data_it, data.end())));
        OTData::readDataVector(&data_it, data.end(), out.iv);

        if (out.iv.empty()) {
            throw std::invalid_argument("IV must not be empty!");
        }
    }

    // Ciphertext (the rest of the data).
    out.ciphertext.clear();
    out.ciphertext.assign(data_it, data.end());

    if (out.ciphertext.empty()) {
        throw std::invalid_argument("Ciphertext must not be empty!");
    }

    envelope = out;
    return; // success
}

// Seal up as envelope (Asymmetric, using public key and then AES key.)

bool OTCrypto_OpenSSL::Seal(mapOfAsymmetricKeys& RecipPubKeys,
                            const String& theInput, ot_data_t& dataOutput) const
{
    struct evp_ctx
    {
        EVP_CIPHER_CTX ctx;
        std::string szFunc;
        bool released;

        ~evp_ctx()
        {
            if (!released)
                if (0 == EVP_CIPHER_CTX_cleanup(&ctx))
                    otErr << szFunc << ": Failure in EVP_CIPHER_CTX_cleanup. "
                                       "(It returned 0.)\n";
        }
    };

    if (RecipPubKeys.empty()) {
        otErr << __FUNCTION__ << " cannot seal an envelope to noone!"
              << std::endl;
        return false;
    }

    envelope_t envelope = {};

    envelope.type = 1; // only one type for now.

    envelope.addresses.resize(RecipPubKeys.size());

    std::vector<EVP_PKEY*> pubkey_v = {};
    std::vector<std::vector<uint8_t>> ek_vv = {};
    std::vector<uint8_t*> ek_vp = {};
    std::vector<int32_t> eklen_v = {};

    // get data out of RecipPubKeys.
    {
        auto addr_it = envelope.addresses.begin();
        for (auto pubkey : RecipPubKeys) {
            (*addr_it++).id = pubkey.first;

            auto lowlevel_pubkey =
                dynamic_cast<OTAsymmetricKey_OpenSSL*>(pubkey.second);
            auto key = lowlevel_pubkey->dp->GetKey();
            pubkey_v.push_back(const_cast<EVP_PKEY*>(key));
        }

        for (auto pubkey : pubkey_v) {
            ek_vv.push_back(std::vector<uint8_t>(EVP_PKEY_size(pubkey)));
        }
    }

    // create array vector for openssl
    {
        for (auto& key : ek_vv) {
            ek_vp.push_back(key.data());
        }
    }

    eklen_v.resize(ek_vp.size());

    evp_ctx the_ctx = {};
    EVP_CIPHER_CTX_init(&the_ctx.ctx);

    const EVP_CIPHER* cipher_type = EVP_aes_128_cbc(); // todo hardcoding.

    envelope.iv.resize(EVP_CIPHER_iv_length(cipher_type));

    if (!EVP_SealInit(&the_ctx.ctx,       // in
                      cipher_type,        // in
                      ek_vp.data(),       // out (pre-allocated memory)
                      eklen_v.data(),     // out (pre-allocated memory)
                      envelope.iv.data(), // out (pre-allocated memory)
                      pubkey_v.data(),    // in
                      pubkey_v.size())    // in
        ) {
        otErr << __FUNCTION__ << ": EVP_SealInit: failed.\n";
        return false;
    }

    // resize and copy encrypted keys into envelope
    {
        auto addr_it = envelope.addresses.begin();
        auto eklen_v_it = eklen_v.begin();
        for (auto& ek : ek_vv) {
            auto s = static_cast<uint32_t>(*eklen_v_it++);
            ek.resize(s);

            (*addr_it++).ek = ek; // copy into envelope
        }
    }

    if (!theInput.Exists()) {
        otErr << __FUNCTION__ << " cannot seal no message!" << std::endl;
        return false;
    }

    // encrypt text and save to envelope;
    {
        std::string str_input(theInput.Get());
        ot_data_t input(str_input.begin(), str_input.end());

        auto input_it = input.begin();

        ot_data_t in_buf = {};
        ot_data_t out_buf = {};

        ot_data_t out = {};
        // Important! Must not have reallocations!
        out.reserve(input.size() + EVP_MAX_IV_LENGTH);
        auto out_it = out.begin();

        for (;;) {
            const bool end = input_it == input.end();

            if (end ||
                in_buf.size() == 128) // we have a block, lets process it.
            {
                ot_data_t out_buf(in_buf.size() + EVP_MAX_IV_LENGTH);
                {
                    int out_buf_len = 0;
                    if (!EVP_SealUpdate(&the_ctx.ctx, out_buf.data(),
                                        &out_buf_len, in_buf.data(),
                                        in_buf.size())) {
                        otErr << __FUNCTION__ << " seal update failed!"
                              << std::endl;
                        return false;
                    }
                    out_buf.resize(out_buf_len);
                }
                out.resize(out.size() + out_buf.size());
                std::copy(out_buf.begin(), out_buf.end(), out_it);
                out_it += out_buf.size();
                in_buf.clear();
            }

            if (end) {
                out_buf.resize(128 + EVP_MAX_IV_LENGTH);
                {
                    int out_len = 0;
                    the_ctx.released = true;
                    if (!EVP_SealFinal(&the_ctx.ctx, out_buf.data(),
                                       &out_len)) {
                        otErr << __FUNCTION__ << " seal final failed!"
                              << std::endl;
                        return false;
                    }
                    out_buf.resize(out_len);
                }
                out.resize(out.size() + out_buf.size());
                std::copy(out_buf.begin(), out_buf.end(), out_it);
                out_it += out_buf.size();
                out_buf.clear();
                break;
            }

            in_buf.push_back(*input_it++);
        }

        envelope.ciphertext = out;
    }

    dataOutput.clear();

    encode_envelope(envelope, dataOutput);

    return true;
}

bool OTCrypto_OpenSSL::Open(ot_data_t& dataInput,
                            const OTPseudonym& theRecipient, String& theOutput,
                            const OTPasswordData* pPWData) const
{
    struct private_key_ptr
    {
        OTAsymmetricKey_OpenSSL* pvtKey;

        ~private_key_ptr()
        {
            if (nullptr != pvtKey) pvtKey->ReleaseKey();
        }
    };

    struct evp_ctx
    {
        EVP_CIPHER_CTX ctx;
        std::string szFunc;
        bool released;

        ~evp_ctx()
        {
            if (!released)
                if (0 == EVP_CIPHER_CTX_cleanup(&ctx))
                    otErr << szFunc << ": Failure in EVP_CIPHER_CTX_cleanup. "
                                       "(It returned 0.)\n";
        }
    };

    // Grab the NymID of the recipient, so we can find his session
    // key (there might be symmetric keys for several Nyms, not just this
    // one, and we need to find the right one in order to perform this Open.)
    //
    std::string our_nym_id;
    {
        String nymID;
        theRecipient.GetIdentifier(nymID);
        our_nym_id = nymID.Get();
    }

    private_key_ptr ot_privateKey = {};
    EVP_PKEY* private_key = nullptr;

    {
        auto& theTempPrivateKey =
            const_cast<OTAsymmetricKey&>(theRecipient.GetPrivateEncrKey());

        ot_privateKey.pvtKey =
            dynamic_cast<OTAsymmetricKey_OpenSSL*>(&theTempPrivateKey);
        OT_ASSERT(nullptr != ot_privateKey.pvtKey);

        private_key =
            const_cast<EVP_PKEY*>(ot_privateKey.pvtKey->dp->GetKey(pPWData));

        if (nullptr == private_key) {
            otErr << __FUNCTION__
                  << ": Null private key on recipient. (Returning false.)\n";
            return false;
        }
    }

    envelope_t env = {};

    try {
        decode_envelope(dataInput, env);
    }
    catch (std::invalid_argument e) {
        otErr << __FUNCTION__ << ": " << e.what() << std::endl;
        return false;
    }

    // Cipher (for now, AES 128 CBC)
    const EVP_CIPHER* cipher_type = EVP_aes_128_cbc();

    ot_data_t text = {};

    for (auto addr : env.addresses) {
        if (!our_nym_id.empty() && !addr.id.empty()) {
            if (0 != our_nym_id.compare(addr.id)) {
                continue; // we have id's, but they don't match.
            }
        }

        bool good_out = true;

        // INSTANTIATE the clean-up object. (scoped per addressee)
        //
        evp_ctx the_ctx = {};
        EVP_CIPHER_CTX_init(&the_ctx.ctx);

        the_ctx.szFunc = __FUNCTION__;
        the_ctx.released = false;

        if (!EVP_OpenInit(&the_ctx.ctx, cipher_type, addr.ek.data(),
                          addr.ek.size(), env.iv.data(), private_key)) {

            // EVP_OpenInit() initializes a cipher context ctx for decryption
            // with
            // cipher type. It decrypts the encrypted
            //    symmetric key of length ekl bytes passed in the ek parameter
            //    using
            // the private key priv. The IV is supplied
            //    in the iv parameter.

            otErr << __FUNCTION__ << ": EVP_OpenInit: failed.\n";
            good_out = false;
            break;
        }

        auto cipher_it = env.ciphertext.begin();

        ot_data_t in_buf = {};
        ot_data_t out_buf = {};

        ot_data_t out = {};
        // Important! Must not have reallocations!
        out.reserve(env.ciphertext.size() + EVP_MAX_IV_LENGTH);
        auto out_it = out.begin();

        for (;;) {
            const bool end = cipher_it == env.ciphertext.end();

            if (end ||
                in_buf.size() == 128) // we have a block, lets process it.
            {
                ot_data_t out_buf(in_buf.size() + EVP_MAX_IV_LENGTH);
                {
                    int out_buf_len = 0;
                    if (!EVP_OpenUpdate(&the_ctx.ctx, out_buf.data(),
                                        &out_buf_len, in_buf.data(),
                                        in_buf.size())) {
                        good_out = false;
                        break;
                    }
                    out_buf.resize(out_buf_len);
                }
                out.resize(out.size() + out_buf.size());
                std::copy(out_buf.begin(), out_buf.end(), out_it);
                out_it += out_buf.size();
                in_buf.clear();
            }

            if (end) {
                out_buf.resize(128 + EVP_MAX_IV_LENGTH);
                {
                    int out_len = 0;
                    the_ctx.released = true;
                    if (!EVP_OpenFinal(&the_ctx.ctx, out_buf.data(),
                                       &out_len)) {
                        good_out = false;
                        break;
                    }
                    out_buf.resize(out_len);
                }
                out.resize(out.size() + out_buf.size());
                std::copy(out_buf.begin(), out_buf.end(), out_it);
                out_it += out_buf.size();
                out_buf.clear();
                break;
            }

            in_buf.push_back(*cipher_it++);
        }

        if (!good_out) {
            continue; // failed for this key (lets try another)
        }

        if (!out.empty()) {
            text = out;
            break; // success
        }
    }

    if (text.empty()) {
        return false; // error. (no data)
    }

    std::string text_str(text.begin(), text.end());

    theOutput.Release();
    theOutput = text_str.c_str();
    return theOutput.GetLength();
}

/*
 128 bytes * 8 bits == 1024 bits key.  (RSA)

 */
bool OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::SignContractDefaultHash(
    const String& strContractUnsigned, const EVP_PKEY* pkey,
    OTSignature& theSignature, const OTPasswordData*) const
{
    const char* szFunc = "OTCrypto_OpenSSL::SignContractDefaultHash";

    // 32 bytes, double sha256
    // This stores the message digest, pre-encrypted, but with the padding
    // added.
    unsigned char* vDigest =
        Hash(strContractUnsigned.Get(),
             strContractUnsigned.Get() + strContractUnsigned.GetLength());

    // This stores the final signature, when the EM value has been signed by RSA
    // private key.
    std::vector<uint8_t> vEM(OTCryptoConfig::PublicKeysizeMax());
    std::vector<uint8_t> vpSignature(OTCryptoConfig::PublicKeysizeMax());

    OTPassword::zeroMemory(&vEM.at(0), OTCryptoConfig::PublicKeysizeMax());
    OTPassword::zeroMemory(&vpSignature.at(0),
                           OTCryptoConfig::PublicKeysizeMax());

    // Here, we convert the EVP_PKEY that was passed in, to an RSA key for
    // signing.
    //
    RSA* pRsaKey = EVP_PKEY_get1_RSA(const_cast<EVP_PKEY*>(pkey));

    if (!pRsaKey) {
        otErr << szFunc << ": EVP_PKEY_get1_RSA failed with error "
              << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return false;
    }

    /*
     NOTE:
     RSA_sign only supports PKCS# 1 v1.5 padding which always gives the same
     output for the same input data.
     If you want to perfom a digital signature with PSS padding, you have to
     pad the data yourself by calling RSA_padding_add_PKCS1_PSS and then call
     RSA_private_encrypt on the padded output after setting its last
     parameter to RSA_NO_PADDING.

     I have written a small sample code that shows how to perform PSS
     signature and verification. You can get the code from the following link:
     http://www.idrix.fr/Root/Samples/openssl_pss_signature.c

     I hope this answers your questions.
     Cheers,
     --
     Mounir IDRASSI
     */
    // compute the PSS padded data
    // the result goes into EM.

    /*
     int32_t RSA_padding_add_PKCS1_PSS(RSA* rsa, uint8_t* EM, const uint8_t*
     mHash, const EVP_MD* Hash, int32_t sLen);
     */
    //    int32_t RSA_padding_add_xxx(uint8_t* to, int32_t tlen,
    //                            uint8_t *f, int32_t fl);
    // RSA_padding_add_xxx() encodes *fl* bytes from *f* so as to fit into
    // *tlen*
    // bytes and stores the result at *to*.
    // An error occurs if fl does not meet the size requirements of the encoding
    // method.
    // The RSA_padding_add_xxx() functions return 1 on success, 0 on error.
    // The RSA_padding_check_xxx() functions return the length of the recovered
    // data, -1 on error.

    //   rsa    EM    mHash      Hash      sLen
    //      in    OUT      IN        in        in
    const EVP_MD* md_sha256 = EVP_sha256();
    int32_t status =
        RSA_padding_add_PKCS1_PSS(pRsaKey, &vEM.at(0), vDigest, md_sha256,
                                  -2); // maximum salt length

    // Above, pDigest is the input, but its length is not needed, since it is
    // determined
    // by the digest algorithm (md_sha256.) In this case, that size is 32 bytes
    // ==
    // 256 bits.

    // More clearly: pDigest is 256 bits int64_t, aka 32 bytes. The call to
    // RSA_padding_add_PKCS1_PSS above
    // is transforming its contents based on digest1, into EM. Once this is
    // done, the new digest stored in
    // EM will be RSA_size(pRsaKey)-11 bytes in size, with the rest padded.
    // Therefore if this is sucessful, then we can call RSA_private_encrypt
    // without any further padding,
    // since it's already accomplished here. EM itself will be RSA_size(pRsaKey)
    // in size total (exactly.)

    if (!status) // 1 or 0.
    {
        otErr << __FILE__ << ": RSA_padding_add_PKCS1_PSS failure: "
              << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }

    // EM is now set up.
    // But how big is it? Answer: RSA_size(pRsaKey)
    // No size is returned because the whole point of RSA_padding_add_PKCS1_PSS
    // is to safely pad
    // pDigest into EM within a specific size based on the keysize.

    // RSA_padding_check_xxx() verifies that the fl bytes at f contain a valid
    // encoding for a rsa_len byte RSA key in the respective
    // encoding method and stores the recovered data of at most tlen bytes (for
    // RSA_NO_PADDING: of size tlen) at to.

    // RSA_private_encrypt
    //    int32_t RSA_private_encrypt(int32_t flen, uint8_t* from,
    //                            uint8_t *to, RSA* rsa, int32_t padding);
    // RSA_private_encrypt() signs the *flen* bytes at *from* (usually a message
    // digest with
    // an algorithm identifier) using the private key rsa and stores the
    // signature in *to*.
    // to must point to RSA_size(rsa) bytes of memory.
    // RSA_private_encrypt() returns the size of the signature (i.e.,
    // RSA_size(rsa)).
    //
    status = RSA_private_encrypt(
        RSA_size(pRsaKey),  // input
        &vEM.at(0),         // padded message digest (input)
        &vpSignature.at(0), // encrypted padded message digest (output)
        pRsaKey,            // private key (input )
        RSA_NO_PADDING); // why not RSA_PKCS1_PADDING ? (Custom padding above in
                         // PSS mode with two hashes.)

    if (status == -1) {
        otErr << szFunc << ": RSA_private_encrypt failure: "
              << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }
    // status contains size

    ot_data_t binSignature(vpSignature); // RSA_private_encrypt
                                         // actually returns the
                                         // right size.
    binSignature.resize(status);
    //    OTData binSignature(pSignature, 128);    // stop hardcoding this block
    // size.

    // theSignature that was passed in, now contains the final signature.
    // The contents were hashed twice, and the resulting hashes were
    // XOR'd together, and then padding was added, and then it was signed
    // with the private key.
    theSignature.SetData(binSignature, true); // true means, "yes, with newlines
                                              // in the b64-encoded output,
                                              // please."
    if (pRsaKey) RSA_free(pRsaKey);
    pRsaKey = nullptr;

    return true;
}

bool OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::VerifyContractDefaultHash(
    const String& strContractToVerify, const EVP_PKEY* pkey,
    const OTSignature& theSignature, const OTPasswordData*) const
{
    const char* szFunc = "OTCrypto_OpenSSL::VerifyContractDefaultHash";

    // 32 bytes, double sha256
    unsigned char* vDigest =
        Hash(strContractToVerify.Get(),
             strContractToVerify.Get() + strContractToVerify.GetLength());

    ot_data_t vDecrypted = {};
    vDecrypted.resize(OTCryptoConfig::PublicKeysizeMax());

    // Here, we convert the EVP_PKEY that was passed in, to an RSA key for
    // signing.
    RSA* pRsaKey = EVP_PKEY_get1_RSA(const_cast<EVP_PKEY*>(pkey));

    if (!pRsaKey) {
        otErr << szFunc << ": EVP_PKEY_get1_RSA failed with error "
              << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return false;
    }

    ot_data_t binSignature = {};

    // This will cause binSignature to contain the base64 decoded binary of the
    // signature that we're verifying. Unless the call fails of course...
    //
    if ((theSignature.GetLength() < 10) ||
        (false == theSignature.GetData(binSignature))) {
        otErr << szFunc << ": Error decoding base64 data for Signature.\n";
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }

    const int32_t nSignatureSize = static_cast<int32_t>(
        binSignature.size()); // converting from unsigned to signed (since
    // openssl wants it that way.)

    if ((binSignature.size() < static_cast<uint32_t>(RSA_size(pRsaKey))) ||
        (nSignatureSize < RSA_size(pRsaKey))) // this one probably unnecessary.
    {
        otErr << szFunc << ": Decoded base64-encoded data for signature, but "
                           "resulting size was < RSA_size(pRsaKey): "
                           "Signed: " << nSignatureSize
              << ". Unsigned: " << binSignature.size() << ".\n";
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }

    // now we will verify the signature
    // Start by a RAW decrypt of the signature
    // output goes to pDecrypted
    // FYI: const void * binSignature.GetPointer()

    // RSA_PKCS1_OAEP_PADDING
    // RSA_PKCS1_PADDING

    // the 128 in the below call was a BUG. The SIZE of the ciphertext
    // (signature) being decrypted is NOT 128 (modulus / cleartext size).
    // Rather, the size of the signature is RSA_size(pRsaKey).  Will have to
    // revisit this likely, elsewhere in the code.
    //    status = RSA_public_decrypt(128, static_cast<const
    // uint8_t*>(binSignature.GetPointer()), pDecrypted, pRsaKey,
    // RSA_NO_PADDING);
    int32_t status = RSA_public_decrypt(
        nSignatureSize,      // length of signature, aka RSA_size(rsa)
        binSignature.data(), // location of signature
        vDecrypted.data(), // Output--must be large enough to hold the md (which
        // is smaller than RSA_size(rsa) - 11)
        pRsaKey, // signer's public key
        RSA_NO_PADDING);

    // int32_t RSA_public_decrypt(int32_t flen, uint8_t* from,
    //                            uint8_t *to, RSA* rsa, int32_t padding);

    // RSA_public_decrypt() recovers the message digest from the *flen* bytes
    // int64_t signature at *from*,
    // using the signer's public key *rsa*.
    // padding is the padding mode that was used to sign the data.
    // *to* must point to a memory section large enough to hold the message
    // digest
    // (which is smaller than RSA_size(rsa) - 11).
    // RSA_public_decrypt() returns the size of the recovered message digest.
    /*
    message to be encrypted, an octet string of length at
    most k-2-2hLen, where k is the length in octets of the
    modulus n and hLen is the length in octets of the hash
    function output for EME-OAEP
    */

    if (status == -1) // Error
    {
        otErr << szFunc << ": RSA_public_decrypt failed with error "
              << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }
    // status contains size of recovered message digest after signature
    // decryption.

    // verify the data
    // Now it compares pDecrypted (the decrypted message digest from the
    // signature) with pDigest
    // (supposedly the same message digest, which we calculated above based on
    // the message itself.)
    // They SHOULD be the same.
    /*
	int32_t RSA_verify_PKCS1_PSS(RSA* rsa, const uint8_t* mHash, const EVP_MD* Hash, const uint8_t* EM, int32_t sLen)
	*/ // rsa        mHash    Hash alg.    EM         sLen

    const EVP_MD* md_sha256 = EVP_sha256();
    status =
        RSA_verify_PKCS1_PSS(pRsaKey, vDigest, md_sha256, vDecrypted.data(),
                             -2); // salt length recovered from signature

    if (!status) {
        otLog5 << szFunc << ": RSA_verify_PKCS1_PSS failed with error: "
               << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }

    otLog5 << "  *Signature verified*\n";

    /*

    NOTE:
    RSA_private_encrypt() signs the flen bytes at from (usually a message
    digest with an algorithm identifier)
    using the private key rsa and stores the signature in to. to must point to
    RSA_size(rsa) bytes of memory.

    From: http://linux.die.net/man/3/rsa_public_decrypt

    RSA_NO_PADDING
    Raw RSA signature. This mode should only be used to implement
    cryptographically sound padding modes in the application code.
    Signing user data directly with RSA is insecure.

    RSA_PKCS1_PADDING
    PKCS #1 v1.5 padding. This function does not handle the algorithmIdentifier
    specified in PKCS #1. When generating or verifying
    PKCS #1 signatures, rsa_sign(3) and rsa_verify(3) should be used.

    Need to research this and make sure it's being done right.

    Perhaps my use of the lower-level call here is related to my use of two
    message-digest algorithms.
    -------------------------------

    On Sun, Feb 25, 2001 at 08:04:55PM -0500, Greg Stark wrote:

    > It is not a bug, it is a known fact. As Joseph Ashwood notes, you end up
    > trying to encrypt values that are larger than the modulus. The
    documentation
    > and most literature do tend to refer to moduli as having a certain
    "length"
    > in bits or bytes. This is fine for most discussions, but if you are
    planning
    > to use RSA to directly encrypt/decrypt AND you are not willing or able to
    > use one of the padding schemes, then you'll have to understand *all* the
    > details. One of these details is that it is possible to supply
    > RSA_public_encrypt() with plaintext values that are greater than the
    modulus
    > N. It returns values that are always between 0 and N-1, which is the only
    > reasonable behavior. Similarly, RSA_public_decrypt() returns values
    between
    > 0 and N-1.

    I have to confess I totally overlooked that and just assumed that if
    RSA_size(key) would be 1024, then I would be able to encrypt messages of
    1024
    bits.

    > There are multiple solutions to this problem. A generally useful one
    > is to use the RSA PKCS#1 ver 1.5 padding
    > (http://www.rsalabs.com/pkcs/pkcs-1/index.html). If you don't like that
    > padding scheme, then you might want to read the PKCS#1 document for the
    > reasons behind that padding scheme and decide for yourself where you can
    > modify it. It sounds like it be easiest if you just follow Mr. Ashwood's
    > advice. Is there some problem with that?

    Yes well, upon reading the PKCS#1 v1.5 document I noticed that Mr. Ashwood
    solves this problem by not only making the most significant bit zero, but
    in
    fact the 6 most significant bits.

    I don't want to use one of the padding schemes because I already know the
    message size in advance, and so does a possible attacker. Using a padding
    scheme would therefore add known plaintext, which does not improve
    security.

    But thank you for the link! I think this solves my problem now :).
    */

    /*
    #include <openssl/rsa.h>

    int32_t RSA_sign(int32_t type, const uint8_t* m, uint32_t m_len, uint8_t*
    sigret, uint32_t* siglen, RSA* rsa);
    int32_t RSA_verify(int32_t type, const uint8_t* m, uint32_t m_len, uint8_t*
    sigbuf, uint32_t siglen, RSA* rsa);

    DESCRIPTION

    RSA_sign() signs the message digest m of size m_len using the private key
    rsa as specified in PKCS #1 v2.0.
    It stores the signature in sigret and the signature size in siglen. sigret
    must point to RSA_size(rsa) bytes of memory.

    type denotes the message digest algorithm that was used to generate m. It
    usually is one of NID_sha1, NID_ripemd160
    and NID_md5; see objects(3) for details. If type is NID_md5_sha1, an SSL
    signature (MD5 and SHA1 message digests with
    PKCS #1 padding and no algorithm identifier) is created.

    RSA_verify() verifies that the signature sigbuf of size siglen matches a
    given message digest m of size m_len. type
    denotes the message digest algorithm that was used to generate the
    signature. rsa is the signer's public key.

    RETURN VALUES

    RSA_sign() returns 1 on success, 0 otherwise. RSA_verify() returns 1 on
    successful verification, 0 otherwise.

    The error codes can be obtained by ERR_get_error(3).
    */

    /*
    Hello,
    > I am getting the following error in calling OCSP_basic_verify():
    >
    > error:04067084:rsa routines:RSA_EAY_PUBLIC_DECRYPT:data too large for
    modulus
    >
    > Could somebody advice what is going wrong?

    In RSA you can encrypt/decrypt only as much data as RSA key size
    (size of RSA key is the size of modulus n = p*q).
    In this situation, RSA routine checks size of data to decrypt
    (probably signature) and this size of bigger than RSA key size,
    this if of course error.
    I think that in this situation this is possible when OCSP was signed
    with (for example) 2048 bit key (private key) and you have some
    certificate with (maybe old) 1024 bit public key.
    In this case this error may happen.
    My suggestion is to check signer certificate.

    Best regards,
    --
    Marek Marcola <[EMAIL PROTECTED]>



    Daniel Stenberg | 16 Jul 19:57

    Re: SSL cert error with CURLOPT_SSL_VERIFYPEER

    On Thu, 16 Jul 2009, Stephen Collyer wrote:

    > error:04067084:rsa routines:RSA_EAY_PUBLIC_DECRYPT:data too large for
    > modulus

    This sounds like an OpenSSL problem to me.



    http://www.mail-archive.com/openssl-users@openssl.org/msg38183.html
    On Tue, Dec 07, 2004, Jesse Hammons wrote:

    >
    > > Jesse Hammons wrote:
    > >
    > >> So to clarify: If I generate a 65-bit key, will I be able to use that
    > >> 65-bit key to sign any 64-bit value?
    > >
    > > Yes, but
    >
    > Actually, I have found the answer to be "no" :-)
    >
    > > a 65 bit key won't be very secure AT ALL, it will be
    > > very easy to factor a modulus that small.
    >
    > Security is not my goal.  This is more of a theoretical exercise that
    > happens to have a practical application for me.
    >
    > >  Bottom line: asymmetrical
    > > (public-key) encryption has a fairly large "minimum block size" that
    > > actually increases as key size increases.
    >
    > Indeed.  I have found experimentally that:
    >  * The minimum signable data quantity in OpenSSL is 1 byte
    >  * The minimum size RSA key that can be used to sign 1 byte is 89 bits
    >  * A signature created using a 64-bit RSA key would create a number 64
    > bits int64_t, BUT:
    >    - This is not possible to do in OpenSSL because the maximum signable
    > quantity for a 64
    >       bit RSA key is only a few bits, and OpenSSL input/output is done on
    > byte boundaries
    >
    > Do those number sound right?

    It depends on the padding mode. These insert/delete padding bytes depending
    on
    the mode used. If you use the no padding mode you can "sign" data equal to
    the
    modulus length but less than its magnitude.

    Check the manual pages (e.g. RSA_private_encrypt()) for more info.





    http://www.mail-archive.com/openssl-users@openssl.org/msg29731.html
    Hmm, the error message "RSA_R_DATA_TOO_LARGE_FOR_MODULUS"
    is triggered by:

    ... (from RSA_eay_private_encrypt() in rsa_eay.c)
    if (BN_ucmp(&f, rsa->n) >= 0)
    {
    // usually the padding functions would catch this
    RSAerr(...,RSA_R_DATA_TOO_LARGE_FOR_MODULUS);
    goto err;
    }
    ...
    => the error message has nothing to do with PKCS#1. It should tell you
    that your plaintext (as a BIGNUM) is greater (or equal) than the modulus.
    The typical error message in case of PKCS#1 error (in your case) would
    be "RSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE".

    > I can arrange for the plaintext to be a little smaller: 14 octets is
    > definitely doable. (The 15 octet length for the ciphertext I can't
    exceed.)
    > If I arrange for the plaintext to be a zero followed by 14 octets of
    data,
    > can I make this work?

    it should work (, but what about a longer (== more secure) key ?)

    Regards,
    Nils




    For reasons that would be tedious to rehearse, the size of the encrypted
    block has to be not more than 15 octets.
    I was hoping for something a little more definitive than "should work."


    >
    > Would a good approach be perhaps to generate keys until I found one for
    > which n is greater than the bignum representation of the largest
    plaintext?
    > (Yeah, I know, this would restrict the key space, which might be a
    security
    > concern.)

    It would be sufficient is the highest bit of the plaintext is zero
    , because the highest bit of the modulus is certainly set
    (at least if the key is generated with OpenSSL).

    ...
    > > it should work (, but what about a longer (== more secure) key ?)
    >
    > For reasons that would be tedious to rehearse, the size of the encrypted
    > block has to be not more than 15 octets.
    >
    > I was hoping for something a little more definitive than "should work."

    Ok , unless something really strange happens: it will work :-)

    Regards,
    Nils


    Re: RSA_private_encrypt does not work with RSA_NO_PADDING option
    by Dr. Stephen Henson Jul 19, 2010; 10:31am :: Rate this Message:    - Use
    ratings to moderate (?)
    Reply | Print | View Threaded | Show Only this Message
    On Mon, Jul 19, 2010, anhpham wrote:

    >
    > Hi all :x
    > I encountered an error when using function RSA_private_encrypt with
    > RSA_NO_PADDING option.
    > I had an uint8_t array a with length = 20, RSA* r,
    > uint8_t* sig = (uint8_t*) malloc(RSA_size(r)) and then I invoked
    > function int32_t i = RSA_private_encrypt(20,a ,sign,r,RSA_NO_PADDING );
    The
    > returned value  i = -1 means that this function failed. However, when I
    > invoked int32_t i = RSA_private_encrypt(20,a,sig,r,RSA_PKCS1_PADDING ),
    it did
    > run smoothly. I'm confused whether it is an error of the library or not
    but
    > I don't know how to solve this problem.
    > Please help me :-<
    ... [show rest of quote]

    If you use RSA_NO_PADDING you have to supply a buffer of RSA_size(r) bytes
    and
    whose value is less than the modulus.

    With RSA_PKCS1_PADDING you can pass up to RSA_size(r) - 11.

    Steve.
    --
    Dr Stephen N. Henson. OpenSSL project core developer.
    Commercial tech support now available see: http://www.openssl.org



    Hello,

    I have a problem, I cannot really cover.

    I'm using public key encryption together with RSA_NO_PADDING. The
    Key-/Modulus-Size is 128Byte and the message to be encrypted are also
    128Byte sized.

    Now my problem:
    Using the same (!) binary code (running in a debugging environment or not)
    it sometimes work properly, sometimes it failes with the following
    message:

    "error:04068084:rsa routines:RSA_EAY_PUBLIC_ENCRYPT:data too large for
    modulus"

    Reply:
    It is *not* enough that the modulus and message are both 128 bytes. You
    need
    a stronger condition.

    Suppose your RSA modulus, as a BigNum, is n. Suppose the data you are
    trying
    to encrypt, as a BigNum, is x. You must ensure that x < n, or you get that
    error message. That is one of the reasons to use a padding scheme such as
    RSA_PKCS1 padding.


    knotwork
    is this a reason to use larger keys or something? 4096 instead of2048 or
    1024?

    4:41
    FellowTraveler
    larger keys is one solution, and that is why I've been looking at mkcert.c
    which, BTW *you* need to look at mkcert.c since there are default values
    hardcoded, and I need you to give me a better idea of what you would want
    in those places, as a server operator.
    First argument of encrypt should have been key.size() and first argument of
    decrypt should have been RSA_size(myKey).
    Padding scheme should have been used
    furthermore, RSA_Sign and RSA_Verify should have been used instead of
    RSA_Public_Decrypt and RSA_Private_Encrypt
    What you are seeing, your error, is a perfectly normal result of the fact
    that the message data being passed in is too large for the modulus of your
    key.
    .
    All of the above fixes need to be investigated and implemented at some
    point, and that will almost certainly change the data format inside the key
    enough to invalidate all existing signatures
    This is a real bug you found, in the crypto.

    4:43
    knotwork
    zmq got you thinking you could have large messages so you forgot the crypto
    had its own limits on message size?

    4:43
    FellowTraveler
    it's not message size per se
    it's message DIGEST size in relation to key modulus
    which must be smaller based on a bignum comparison of the two
    RSA_Size is supposed to be used in decrypt

    4:44
    knotwork
    a form of the resync should fix everything, it just needs to run throguh
    everything resigning it with new type of signature?

    4:44
    FellowTraveler
    not that simple
    I would have to code some kind of special "convert legacy data" thing into
    OT itself
    though there might be a stopgap measure now, good enough to keep data until
    all the above fixes are made
    ok see if this fixes it for you......
    knotwork, go into OTLib/OTContract.cpp
    Find the first line that begins with status = RSA_public_decrypt

    4:46
    knotwork
    vanalces would be enough maybe. jsut a way to set balances of all accoutns
    to whatever they actually are at the time

    4:46
    FellowTraveler
    the only other one is commented out, so it's not hard
    you will see a hardcoded size:    status = RSA_public_decrypt(128,
    CHANGE the 128 to this value:
    RSA_size(pRsaKey)
    for now you can change the entire line to this:
    status = RSA_public_decrypt(RSA_size(pRsaKey), static_cast<const
    uint8_t*>(binSignature.GetPointer()), pDecrypted, pRsaKey, RSA_NO_PADDING);
    Then see if your bug goes away
    I will still need to make fixes someday though, even if this works, and
    will have to lose or convert data.
    4:48
    otherwise there could be security issues down the road.


    TODO SECURITY ^  sign/verify needs revamping!

    UPDATE: Okay I may have it fixed now, though need to test still.

    http://www.bmt-online.org/geekisms/RSA_verify

    Also see: ~/Projects/openssl/demos/sign
    */

    if (pRsaKey) RSA_free(pRsaKey);
    pRsaKey = nullptr;

    return true;
}

// All the other various versions eventually call this one, where the actual
// work is done.
bool OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::SignContract(
    const String& strContractUnsigned, const EVP_PKEY* pkey,
    OTSignature& theSignature, const String& strHashType,
    const OTPasswordData* pPWData) const
{
    OT_ASSERT_MSG(nullptr != pkey,
                  "Null private key sent to OTCrypto_OpenSSL::SignContract.\n");

    const char* szFunc = "OTCrypto_OpenSSL::SignContract";

    class _OTCont_SignCont1
    {
    private:
        const char* m_szFunc;
        EVP_MD_CTX& m_ctx;

    public:
        _OTCont_SignCont1(const char* param_szFunc, EVP_MD_CTX& param_ctx)
            : m_szFunc(param_szFunc)
            , m_ctx(param_ctx)
        {
            OT_ASSERT(nullptr != m_szFunc);

            EVP_MD_CTX_init(&m_ctx);
        }
        ~_OTCont_SignCont1()
        {
            if (0 == EVP_MD_CTX_cleanup(&m_ctx))
                otErr << m_szFunc << ": Failure in cleanup. (It returned 0.)\n";
        }
    };

    const bool bUsesDefaultHashAlgorithm =
        strHashType.Compare(Identifier::DefaultHashAlgorithm);
    EVP_MD* md = nullptr;

    if (bUsesDefaultHashAlgorithm) {
        return SignContractDefaultHash(strContractUnsigned, pkey, theSignature,
                                       pPWData);
    }

    //    else
    {
        md = const_cast<EVP_MD*>(
            OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::GetOpenSSLDigestByName(
                strHashType));
    }

    // If it's not the default hash, then it's just a normal hash.
    // Either way then we process it, first by getting the message digest
    // pointer for signing.

    if (nullptr == md) {
        otErr << szFunc
              << ": Unable to decipher Hash algorithm: " << strHashType << "\n";
        return false;
    }

    // RE: EVP_SignInit() or EVP_MD_CTX_init()...
    //
    // Since only a copy of the digest context is ever finalized the
    // context MUST be cleaned up after use by calling EVP_MD_CTX_cleanup()
    // or a memory leak will occur.
    //
    EVP_MD_CTX md_ctx;

    _OTCont_SignCont1 theInstance(szFunc, md_ctx);

    // Do the signature
    // Note: I just changed this to the _ex version (in case I'm debugging later
    // and find a problem here.)
    //
    EVP_SignInit_ex(&md_ctx, md, nullptr);

    EVP_SignUpdate(&md_ctx, strContractUnsigned.Get(),
                   strContractUnsigned.GetLength());

    uint8_t sig_buf[4096]; // Safe since we pass the size when we use it.

    int32_t sig_len = sizeof(sig_buf);
    int32_t err =
        EVP_SignFinal(&md_ctx, sig_buf, reinterpret_cast<uint32_t*>(&sig_len),
                      const_cast<EVP_PKEY*>(pkey));

    if (err != 1) {
        otErr << szFunc << ": Error signing xml contents.\n";
        return false;
    }
    else {
        otLog3 << szFunc << ": Successfully signed xml contents.\n";

        // We put the signature data into the signature object that
        // was passed in for that purpose.
        ot_data_t tempData = {};
        tempData.assign(sig_buf, sig_buf + sig_len);
        theSignature.SetData(tempData);

        return true;
    }
}

bool OTCrypto_OpenSSL::SignContract(const String& strContractUnsigned,
                                    const OTAsymmetricKey& theKey,
                                    OTSignature& theSignature, // output
                                    const String& strHashType,
                                    const OTPasswordData* pPWData)
{

    OTAsymmetricKey& theTempKey = const_cast<OTAsymmetricKey&>(theKey);
    OTAsymmetricKey_OpenSSL* pTempOpenSSLKey =
        dynamic_cast<OTAsymmetricKey_OpenSSL*>(&theTempKey);
    OT_ASSERT(nullptr != pTempOpenSSLKey);

    const EVP_PKEY* pkey = pTempOpenSSLKey->dp->GetKey(pPWData);
    OT_ASSERT(nullptr != pkey);

    if (false ==
        dp->SignContract(strContractUnsigned, pkey, theSignature, strHashType,
                         pPWData)) {
        otErr << "OTCrypto_OpenSSL::SignContract: "
              << "SignContract returned false.\n";
        return false;
    }

    return true;
}

bool OTCrypto_OpenSSL::VerifySignature(const String& strContractToVerify,
                                       const OTAsymmetricKey& theKey,
                                       const OTSignature& theSignature,
                                       const String& strHashType,
                                       const OTPasswordData* pPWData) const
{
    OTAsymmetricKey& theTempKey = const_cast<OTAsymmetricKey&>(theKey);
    OTAsymmetricKey_OpenSSL* pTempOpenSSLKey =
        dynamic_cast<OTAsymmetricKey_OpenSSL*>(&theTempKey);
    OT_ASSERT(nullptr != pTempOpenSSLKey);

    const EVP_PKEY* pkey = pTempOpenSSLKey->dp->GetKey(pPWData);
    OT_ASSERT(nullptr != pkey);

    if (false ==
        dp->VerifySignature(strContractToVerify, pkey, theSignature,
                            strHashType, pPWData)) {
        otLog3 << "OTCrypto_OpenSSL::VerifySignature: "
               << "VerifySignature returned false.\n";
        return false;
    }

    return true;
}

// All the other various versions eventually call this one, where the actual
// work is done.
bool OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::VerifySignature(
    const String& strContractToVerify, const EVP_PKEY* pkey,
    const OTSignature& theSignature, const String& strHashType,
    const OTPasswordData* pPWData) const
{
    OT_ASSERT_MSG(strContractToVerify.Exists(),
                  "OTCrypto_OpenSSL::VerifySignature: ASSERT FAILURE: "
                  "strContractToVerify.Exists()");
    OT_ASSERT_MSG(nullptr != pkey,
                  "Null pkey in OTCrypto_OpenSSL::VerifySignature.\n");

    const char* szFunc = "OTCrypto_OpenSSL::VerifySignature";

    const bool bUsesDefaultHashAlgorithm =
        strHashType.Compare(Identifier::DefaultHashAlgorithm);
    EVP_MD* md = nullptr;

    if (bUsesDefaultHashAlgorithm) {
        return VerifyContractDefaultHash(strContractToVerify, pkey,
                                         theSignature, pPWData);
    }

    //    else
    {
        md = const_cast<EVP_MD*>(
            OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::GetOpenSSLDigestByName(
                strHashType));
    }

    if (!md) {
        otWarn << szFunc
               << ": Unknown message digest algorithm: " << strHashType << "\n";
        return false;
    }

    ot_data_t binSignature;

    // now binSignature contains the base64 decoded binary of the signature.
    // Unless the call failed of course...
    if (!theSignature.GetData(binSignature)) {
        otErr << szFunc << ": Error decoding base64 data for Signature.\n";
        return false;
    }

    EVP_MD_CTX ctx;
    EVP_MD_CTX_init(&ctx);

    EVP_VerifyInit(&ctx, md);

    // Here I'm adding the actual XML portion of the contract (the portion that
    // gets signed.)
    // Basically we are repeating similarly to the signing process in order to
    // verify.

    EVP_VerifyUpdate(&ctx, strContractToVerify.Get(),
                     strContractToVerify.GetLength());

    // Now we pass in the Signature
    // EVP_VerifyFinal() returns 1 for a correct signature,
    // 0 for failure and -1 if some other error occurred.
    //
    int32_t nErr =
        EVP_VerifyFinal(&ctx, binSignature.data(), binSignature.size(),
                        const_cast<EVP_PKEY*>(pkey));

    EVP_MD_CTX_cleanup(&ctx);

    // the moment of true. 1 means the signature verified.
    if (1 == nErr)
        return true;
    else
        return false;
}

// Sign the Contract using a private key from a file.
// theSignature will contain the output.
bool OTCrypto_OpenSSL::SignContract(const String& strContractUnsigned,
                                    const String& strSigHashType,
                                    const std::string& strCertFileContents,
                                    OTSignature& theSignature,
                                    const OTPasswordData* pPWData)
{
    OT_ASSERT_MSG(strContractUnsigned.Exists(), "OTCrypto_OpenSSL::"
                                                "SignContract: ASSERT FAILURE: "
                                                "strContractUnsigned.Exists()");
    OT_ASSERT_MSG(
        strCertFileContents.size() > 2,
        "Empty strCertFileContents passed to OTCrypto_OpenSSL::SignContract");

    // Create a new memory buffer on the OpenSSL side
    //
    OpenSSL_BIO bio = BIO_new_mem_buf(
        reinterpret_cast<void*>(const_cast<char*>(strCertFileContents.c_str())),
        -1);
    OT_ASSERT(nullptr != bio);

    // TODO security:
    /* The old PrivateKey write routines are retained for compatibility.
     New applications should write private keys using the
     PEM_write_bio_PKCS8PrivateKey() or PEM_write_PKCS8PrivateKey()
     routines because they are more secure (they use an iteration count of 2048
     whereas the traditional routines use a
     count of 1) unless compatibility with older versions of OpenSSL is
     important.
     NOTE: The PrivateKey read routines can be used in all applications because
     they handle all formats transparently.
     */
    OTPasswordData thePWData("(OTCrypto_OpenSSL::SignContract is trying to "
                             "read the private key...)");

    if (nullptr == pPWData) pPWData = &thePWData;

    bool bSigned = false;
    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(
        bio, nullptr, OTAsymmetricKey::GetPasswordCallback(),
        const_cast<OTPasswordData*>(pPWData));

    if (nullptr == pkey) {
        otErr << "OTCrypto_OpenSSL::SignContract: "
              << "Error reading private key from BIO.\n";
    }
    else {
        bSigned = dp->SignContract(strContractUnsigned, pkey, theSignature,
                                   strSigHashType, pPWData);

        EVP_PKEY_free(pkey);
        pkey = nullptr;
    }

    return bSigned;
}

// Presumably the Signature passed in here was just loaded as part of this
// contract and is
// somewhere in m_listSignatures. Now it is being verified.
//
bool OTCrypto_OpenSSL::VerifySignature(const String& strContractToVerify,
                                       const String& strSigHashType,
                                       const std::string& strCertFileContents,
                                       const OTSignature& theSignature,
                                       const OTPasswordData* pPWData) const
{
    OT_ASSERT_MSG(strContractToVerify.Exists(),
                  "OTCrypto_OpenSSL::VerifySignature: ASSERT FAILURE: "
                  "strContractToVerify.Exists()");
    OT_ASSERT_MSG(strCertFileContents.size() > 2,
                  "Empty strCertFileContents passed to "
                  "OTCrypto_OpenSSL::VerifySignature");

    const char* szFunc = "OTCrypto_OpenSSL::VerifySignature";

    // Create a new memory buffer on the OpenSSL side
    //
    OpenSSL_BIO bio = BIO_new_mem_buf(
        static_cast<void*>(const_cast<char*>(strCertFileContents.c_str())), -1);
    OT_ASSERT(nullptr != bio);

    OTPasswordData thePWData("(OTCrypto_OpenSSL::VerifySignature is trying to "
                             "read the public key...)");

    if (nullptr == pPWData) pPWData = &thePWData;

    X509* x509 =
        PEM_read_bio_X509(bio, nullptr, OTAsymmetricKey::GetPasswordCallback(),
                          const_cast<OTPasswordData*>(pPWData));

    if (nullptr == x509) {
        otErr << szFunc << ": Failed reading x509 out of cert file...\n";
        return false;
    }

    bool bVerifySig = false;
    EVP_PKEY* pkey = X509_get_pubkey(x509);

    if (nullptr == pkey) {
        otErr << szFunc
              << ": Failed reading public key from x509 from certfile...\n";
    }
    else {
        bVerifySig = dp->VerifySignature(strContractToVerify, pkey,
                                         theSignature, strSigHashType, pPWData);

        EVP_PKEY_free(pkey);
        pkey = nullptr;
    }

    // At some point have to call this.
    //
    X509_free(x509);
    x509 = nullptr;

    return bVerifySig;
}

// OpenSSL_BIO

// static
BIO* OpenSSL_BIO::assertBioNotNull(BIO* pBIO)
{
    if (nullptr == pBIO) OT_FAIL;
    return pBIO;
}

OpenSSL_BIO::OpenSSL_BIO(BIO* pBIO)
    : m_refBIO(*assertBioNotNull(pBIO))
    , bCleanup(true)
    , bFreeOnly(false)
{
}

OpenSSL_BIO::~OpenSSL_BIO()
{
    if (bCleanup) {
        if (bFreeOnly) {
            BIO_free(&m_refBIO);
        }
        else {
            BIO_free_all(&m_refBIO);
        }
    }
}

OpenSSL_BIO::operator BIO*() const
{
    return (&m_refBIO);
}

void OpenSSL_BIO::release()
{
    bCleanup = false;
}

void OpenSSL_BIO::setFreeOnly()
{
    bFreeOnly = true;
}

#elif defined(OT_CRYPTO_USING_GPG)

// Someday    }:-)

#else // Apparently NO crypto engine is defined!

// Perhaps error out here...

#endif // if defined (OT_CRYPTO_USING_OPENSSL), elif defined
       // (OT_CRYPTO_USING_GPG), else, endif.

} // namespace opentxs
