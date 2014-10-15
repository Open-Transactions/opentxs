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

#include <opentxs/core/crypto/OTCryptoOpenSSL.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/crypto/OTPassword.hpp>
#include <opentxs/core/crypto/OTPasswordData.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/crypto/OTSignature.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/util/stacktrace.h>

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

namespace opentxs
{

class OTCrypto_OpenSSL::OTCrypto_OpenSSLdp
{
public:
    // These are protected because they contain OpenSSL-specific parameters.

    bool SignContractDefaultHash(const OTString& strContractUnsigned,
                                 const EVP_PKEY* pkey,
                                 OTSignature& theSignature, // output
                                 const OTPasswordData* pPWData = nullptr) const;

    bool VerifyContractDefaultHash(
        const OTString& strContractToVerify, const EVP_PKEY* pkey,
        const OTSignature& theSignature,
        const OTPasswordData* pPWData = nullptr) const;

    // Sign or verify using the actual OpenSSL EVP_PKEY
    //
    bool SignContract(const OTString& strContractUnsigned, const EVP_PKEY* pkey,
                      OTSignature& theSignature, // output
                      const OTString& strHashType,
                      const OTPasswordData* pPWData = nullptr) const;

    bool VerifySignature(const OTString& strContractToVerify,
                         const EVP_PKEY* pkey, const OTSignature& theSignature,
                         const OTString& strHashType,
                         const OTPasswordData* pPWData = nullptr) const;

    static const EVP_MD* GetOpenSSLDigestByName(const OTString& theName);
};

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

//#ifndef ANDROID // Android thus far only supports OpenSSL 0.9.8k
#include <openssl/whrlpool.h>

//    // Just trying to get Whirlpool working since they added it to OpenSSL
//    //
//    static int32_t init(EVP_MD_CTX* ctx)
//    { return WHIRLPOOL_Init((WHIRLPOOL_CTX*)ctx->md_data); }
//
//    static int32_t update(EVP_MD_CTX* ctx, const void* data,size_t count)
//    { return WHIRLPOOL_Update((WHIRLPOOL_CTX*)ctx->md_data,data,count); }
//
//    static int32_t final(EVP_MD_CTX* ctx, uint8_t* md)
//    { return WHIRLPOOL_Final(md,(WHIRLPOOL_CTX*)ctx->md_data); }
//
//
//    static const EVP_MD whirlpool_md =
//    {
//        NID_whirlpool,
//        0,
//        WHIRLPOOL_DIGEST_LENGTH,
//        0,
//        init,
//        update,
//        final,
//        nullptr,
//        nullptr,
//        EVP_PKEY_nullptr_method,
//        WHIRLPOOL_BBLOCK/8,
//        sizeof(EVP_MD *)+sizeof(WHIRLPOOL_CTX),
//    };
//#endif // !ANDROID

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
unsigned int64_t ot_openssl_thread_id(void);
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
uint64_t ot_openssl_thread_id()
{
    uint64_t ret = this_thread::get_raw_id();

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

// Caller responsible to delete.
char* OTCrypto_OpenSSL::Base64Encode(const uint8_t* input, int32_t in_len,
                                     bool bLineBreaks) const
{
    char* buf = nullptr;
    BUF_MEM* bptr = nullptr;

    OT_ASSERT_MSG(in_len >= 0,
                  "OT_base64_encode: Abort: in_len is a negative number!");

    OpenSSL_BIO b64 = BIO_new(BIO_f_base64());

    if (!b64) return buf;

    if (!bLineBreaks) BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    OpenSSL_BIO bmem = BIO_new(BIO_s_mem());

    if (bmem) {
        OpenSSL_BIO b64join = BIO_push(b64, bmem);
        b64.release();
        bmem.release();

        if (BIO_write(b64join, input, in_len) == in_len) {
            (void)BIO_flush(b64join);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
            BIO_get_mem_ptr(b64join, &bptr);
#pragma GCC diagnostic pop
            //            otLog5 << "DEBUG base64_encode size: %lld,  in_len:
            // %lld\n", bptr->length+1, in_len);
            buf = new char[bptr->length + 1];
            OT_ASSERT(nullptr != buf);
            memcpy(buf, bptr->data, bptr->length); // Safe.
            buf[bptr->length] = '\0';              // Forcing null terminator.
        }
    }
    else {
        OT_FAIL_MSG("Failed creating new Bio in base64_encode.\n");
    }

    return buf;
}

// Caller responsible to delete.
uint8_t* OTCrypto_OpenSSL::Base64Decode(const char* input, size_t* out_len,
                                        bool bLineBreaks) const
{

    OT_ASSERT(nullptr != input);

    int32_t in_len =
        static_cast<int32_t>(strlen(input)); // todo security (strlen)
    int32_t out_max_len = (in_len * 6 + 7) / 8;
    uint8_t* buf = new uint8_t[out_max_len];
    OT_ASSERT(nullptr != buf);
    memset(buf, 0, out_max_len); // todo security

    OpenSSL_BIO b64 = BIO_new(BIO_f_base64());

    if (b64) {
        if (!bLineBreaks) BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

        OpenSSL_BIO bmem = BIO_new_mem_buf(const_cast<char*>(input), in_len);
        OT_ASSERT(nullptr != bmem);

        OpenSSL_BIO b64join = BIO_push(b64, bmem);
        b64.release();
        bmem.release();
        OT_ASSERT(nullptr != b64join);

        *out_len = BIO_read(b64join, buf, out_max_len);

    }
    else {
        OT_FAIL_MSG("Failed creating new Bio in base64_decode.\n");
    }

    return buf;
}

// SET (binary id) FROM BASE62-ENCODED STRING
//
// Using a BigInteger lib I just added.
//
// Hopefully use something like this to replace some of the internals for
// OTIdentifier.
// I need to get the author to add a "back into data again" function though.
//
void OTCrypto_OpenSSL::SetIDFromBase62String(const OTString& strInput,
                                             OTIdentifier& theOutput) const
{
    theOutput.Release();

    // If it's short, no validate.
    //
    if (strInput.GetLength() < 3) return;

    // If it's not base62-encoded, then it doesn't validate.
    //
    const std::string strINPUT = strInput.Get();
    if (!IsBase62(strINPUT)) return;

    // Todo there are try/catches in here, so need to handle those at some
    // point.
    BigInteger bigIntFromBase62 = stringToBigIntegerBase62(strINPUT);

    // Now theBaseConverter contains a BigInteger that it read in as base62.
    //
    // Next step is to output it from that to Hex so I can convert to Binary.
    //
    // Why not convert it DIRECTLY to binary, you might ask?  TODO.
    // In fact this is what we SHOULD be doing. But the BigInteger lib
    // I'm using doesn't have a damned output to binary!  I'm emailing the
    // author now.
    //
    // In the meantime, I had old code from before, that converted hex string to
    // binary, which still needs to be removed. But for now, I'll just convert
    // the
    // BigInteger to hex, and then call my old code (below) just to get things
    // running.

    // You can convert the other way too.
    std::string strHEX_VERSION = bigIntegerToStringBase16(bigIntFromBase62);

    // I would rather use stringToBigUnsigned and then convert that to data.
    // But apparently this class has no conversion back to data, I will contact
    // the author.
    BIGNUM* pBigNum = BN_new();
    OT_ASSERT(nullptr != pBigNum);

    // Convert from Hex String to BIGNUM.
    const int32_t nToHex = BN_hex2bn(&pBigNum, strHEX_VERSION.c_str());
    OT_ASSERT(0 < nToHex);

    // Convert from Hex String to BigInteger (unwieldy, I know. Future versions
    // will improve.)
    //
    uint32_t nBigNumBytes = BN_num_bytes(pBigNum);
    theOutput.SetSize(nBigNumBytes);

    const int32_t nConverted = BN_bn2bin(
        pBigNum,
        static_cast<uint8_t*>(const_cast<void*>(theOutput.GetPointer())));
    OT_ASSERT(nConverted);

    // BN_bn2bin() converts the absolute value of param 1 into big-endian form
    // and stores it at param2.
    // param2 must point to BN_num_bytes(pBigNum) bytes of memory.

    BN_free(pBigNum);
}

// GET (binary id) AS BASE62-ENCODED STRING
//
// This Identifier is stored in binary form.
// But what if you want a pretty hex string version of it?
// Just call this function.
// UPDATE: Now Base62 instead of Hex. (More compact.)
// Easy double-click the ID and the entire thing highlights at once.
//
void OTCrypto_OpenSSL::SetBase62StringFromID(const OTIdentifier& theInput,
                                             OTString& strOutput) const
{
    strOutput.Release();

    if (theInput.IsEmpty()) return;

    // Convert from internal binary format to BIGNUM format.
    //
    BIGNUM* pBigNum = BN_new();
    OT_ASSERT(nullptr != pBigNum);

    BN_bin2bn(static_cast<uint8_t*>(const_cast<void*>(theInput.GetPointer())),
              theInput.GetSize(), pBigNum);

    // Convert from BIGNUM to Hex String.
    //
    char* szBigNumInHex = BN_bn2hex(pBigNum);
    OT_ASSERT(szBigNumInHex != nullptr);

    // Convert from Hex String to BigInteger (unwieldy, I know. Future versions
    // will improve.)
    //
    BigInteger theBigInt = stringToBigIntegerBase16(szBigNumInHex);
    OPENSSL_free(szBigNumInHex);
    szBigNumInHex = nullptr;
    BN_free(pBigNum);

    // Convert from BigInteger to std::string in Base62 format.
    //
    std::string strBigInt = bigIntegerToStringBase62(theBigInt);

    strOutput.Set(strBigInt.c_str());
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
        RAND_bytes(reinterpret_cast<uint8_t*>(szDestination),
                   static_cast<int32_t>(nNewSize));

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
                                           const OTData& dataSalt,
                                           uint32_t uIterations,
                                           OTData& dataCheckHash) const
{
    //  OT_ASSERT(userPassword.isPassword());
    OT_ASSERT(!dataSalt.IsEmpty());

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
                : userPassword.getMemorySize()),            // Password Length
        static_cast<const uint8_t*>(dataSalt.GetPointer()), // Salt Data
        static_cast<const int32_t>(dataSalt.GetSize()),     // Salt Length
        static_cast<const int32_t>(uIterations), // Number Of Iterations
        static_cast<const int32_t>(
            pDerivedKey->getMemorySize()), // Output Length
        static_cast<uint8_t*>(
            pDerivedKey->getMemoryWritable()) // Output Key (not const!)
        );

    // For The HashCheck
    bool bHaveCheckHash = !dataCheckHash.IsEmpty();

    OTData tmpHashCheck;
    tmpHashCheck.SetSize(OTCryptoConfig::SymmetricKeySize());

    // We take the DerivedKey, and hash it again, then get a 'hash-check'
    // Compare that with the supplied one, (if there is one).
    // If there isn't one, we return the

    PKCS5_PBKDF2_HMAC_SHA1(
        reinterpret_cast<const char*>(pDerivedKey->getMemory()), // Derived Key
        static_cast<const int32_t>(
            pDerivedKey->getMemorySize()),                  // Password Length
        static_cast<const uint8_t*>(dataSalt.GetPointer()), // Salt Data
        static_cast<const int32_t>(dataSalt.GetSize()),     // Salt Length
        static_cast<const int32_t>(uIterations), // Number Of Iterations
        static_cast<const int32_t>(tmpHashCheck.GetSize()), // Output Length
        const_cast<uint8_t*>(static_cast<const uint8_t*>(
            tmpHashCheck.GetPointer()))) // Output Key (not const!)
        ;

    if (bHaveCheckHash) {
        OTString strDataCheck, strTestCheck;
        strDataCheck.Set(static_cast<const char*>(dataCheckHash.GetPointer()),
                         dataCheckHash.GetSize());
        strTestCheck.Set(static_cast<const char*>(tmpHashCheck.GetPointer()),
                         tmpHashCheck.GetSize());

        if (!strDataCheck.Compare(strTestCheck)) {
            dataCheckHash.reset();
            dataCheckHash = tmpHashCheck;
            return nullptr; // failure (but we will return the dataCheckHash we
                            // got
                            // anyway)
        }
    }
    else {
        dataCheckHash.reset();
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
    const OTString& theName)
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
    //#ifndef ANDROID
    else if (theName.Compare("WHIRLPOOL")) // Todo: follow up on any cleanup
                                           // issues related to this. (Are the
                                           // others dynamically allocated? This
                                           // one isn't.)
        return EVP_whirlpool();
    //#endif
    return nullptr;
}

bool OTCrypto_OpenSSL::CalculateDigest(const OTString& strInput,
                                       const OTString& strHashAlgorithm,
                                       OTIdentifier& theOutput) const
{
    theOutput.Release();

    // Some hash algorithms are handled by other methods.
    // If those don't handle it, then we'll come back here and use OpenSSL.
    if (theOutput.CalculateDigestInternal(strInput, strHashAlgorithm)) {
        return true;
    }

    EVP_MD_CTX mdctx;
    const EVP_MD* md = nullptr;

    uint32_t md_len = 0;
    uint8_t md_value[EVP_MAX_MD_SIZE]; // I believe this is safe, having just
                                       // analyzed this function.

    // Okay, it wasn't any internal hash algorithm, so then which one was it?
    //
    md = OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::GetOpenSSLDigestByName(
        strHashAlgorithm); // todo cleanup?

    if (!md) {
        otErr << "OTCrypto_OpenSSL::CalculateDigest"
              << ": Unknown message digest algorithm: " << strHashAlgorithm
              << "\n";
        return false;
    }

    EVP_MD_CTX_init(&mdctx);
    EVP_DigestInit_ex(&mdctx, md, nullptr);
    EVP_DigestUpdate(&mdctx, strInput.Get(), strInput.GetLength());
    EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
    EVP_MD_CTX_cleanup(&mdctx);

    theOutput.Assign(md_value, md_len);

    return true;
}

bool OTCrypto_OpenSSL::CalculateDigest(const OTData& dataInput,
                                       const OTString& strHashAlgorithm,
                                       OTIdentifier& theOutput) const
{
    theOutput.Release();

    // Some hash algorithms are handled by other methods.
    // If those don't handle it, then we'll come back here and use OpenSSL.
    if (theOutput.CalculateDigestInternal(dataInput, strHashAlgorithm)) {
        return true;
    }

    EVP_MD_CTX mdctx;
    const EVP_MD* md = nullptr;

    uint32_t md_len = 0;
    uint8_t md_value[EVP_MAX_MD_SIZE]; // I believe this is safe, shouldn't ever
                                       // be larger than MAX SIZE.

    // Okay, it wasn't any internal hash algorithm, so then which one was it?
    //
    md = OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::GetOpenSSLDigestByName(
        strHashAlgorithm); // todo cleanup ?

    if (!md) {
        otErr << "OTCrypto_OpenSSL::CalculateDigest"
              << ": Unknown message digest algorithm: " << strHashAlgorithm
              << "\n";
        return false;
    }

    EVP_MD_CTX_init(&mdctx);
    EVP_DigestInit_ex(&mdctx, md, nullptr);
    EVP_DigestUpdate(&mdctx, dataInput.GetPointer(), dataInput.GetSize());
    EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
    EVP_MD_CTX_cleanup(&mdctx);

    theOutput.Assign(md_value, md_len);

    return true;
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
            strPassword = OTString::ws2s(wstrPass);

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
#else
    // todo security: might want to allow to set OTPassword's size and copy
    // directly into it,
    // so that we aren't using this temp buf in between, which, although we're
    // zeroing it, could
    // technically end up getting swapped to disk.
    //
    {
        char buf[_PASSWORD_LEN + 10] = "", buff[_PASSWORD_LEN + 10] = "";

        if (UI_UTIL_read_pw(buf, buff, _PASSWORD_LEN, szPrompt, 0) == 0) {
            size_t nPassLength = OTString::safe_strlen(buf, _PASSWORD_LEN);
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
    //  ERR_remove_state(0);
    ERR_remove_thread_state(nullptr);

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
    const uint32_t lInputLength, const OTData& theIV, // (We assume this IV
                                                      // is already generated
                                                      // and passed in.)
    OTData& theEncryptedOutput) const                 // OUTPUT. (Ciphertext.)
{
    const char* szFunc = "OTCrypto_OpenSSL::Encrypt";

    OT_ASSERT(OTCryptoConfig::SymmetricIvSize() == theIV.GetSize());
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
    // This is where the envelope final contents will be placed.
    // including the size of the IV, the IV itself, and the ciphertext.
    //
    theEncryptedOutput.Release();

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
            static_cast<uint8_t*>(const_cast<void*>(theIV.GetPointer())))) {
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

        uint32_t len = static_cast<uint32_t>(
            (lRemainingLength < OTCryptoConfig::SymmetricBufferSize())
                ? lRemainingLength
                : OTCryptoConfig::SymmetricBufferSize()); // 4096

        if (!EVP_EncryptUpdate(
                &ctx, &vBuffer_out.at(0), &len_out,
                const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(
                    &(szInput[lCurrentIndex]))),
                len)) {
            otErr << szFunc << ": EVP_EncryptUpdate: failed.\n";
            return false;
        }
        lRemainingLength -= len;
        lCurrentIndex += len;

        if (len_out > 0)
            theEncryptedOutput.Concatenate(
                reinterpret_cast<void*>(&vBuffer_out.at(0)),
                static_cast<uint32_t>(len_out));
    }

    if (!EVP_EncryptFinal(&ctx, &vBuffer_out.at(0), &len_out)) {
        otErr << szFunc << ": EVP_EncryptFinal: failed.\n";
        return false;
    }

    // This is the "final" piece that is added from EncryptFinal just above.
    //
    if (len_out > 0)
        theEncryptedOutput.Concatenate(
            reinterpret_cast<void*>(&vBuffer_out.at(0)),
            static_cast<uint32_t>(len_out));

    return true;
}

bool OTCrypto_OpenSSL::Decrypt(
    const OTPassword& theRawSymmetricKey, // The symmetric key, in clear form.
    const char* szInput,                  // This is the Ciphertext.
    const uint32_t lInputLength, const OTData& theIV, // (We assume this IV
                                                      // is already generated
                                                      // and passed in.)
    OTCrypto_Decrypt_Output theDecryptedOutput) const // OUTPUT. (Recovered
                                                      // plaintext.) You can
                                                      // pass OTPassword& OR
// OTData& here (either
// will work.)
{
    const char* szFunc = "OTCrypto_OpenSSL::Decrypt";

    OT_ASSERT(OTCryptoConfig::SymmetricIvSize() == theIV.GetSize());
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
            static_cast<uint8_t*>(const_cast<void*>(theIV.GetPointer())))) {
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

// Seal up as envelope (Asymmetric, using public key and then AES key.)

bool OTCrypto_OpenSSL::Seal(mapOfAsymmetricKeys& RecipPubKeys,
                            const OTString& theInput, OTData& dataOutput) const
{
    OT_ASSERT_MSG(!RecipPubKeys.empty(),
                  "OTCrypto_OpenSSL::Seal: ASSERT: RecipPubKeys.size() > 0");

    const char* szFunc = "OTCrypto_OpenSSL::Seal";

    EVP_CIPHER_CTX ctx;

    uint8_t buffer[4096];
    uint8_t buffer_out[4096 + EVP_MAX_IV_LENGTH];
    uint8_t iv[EVP_MAX_IV_LENGTH];

    uint32_t len = 0;
    int32_t len_out = 0;

    memset(buffer, 0, 4096);
    memset(buffer_out, 0, 4096 + EVP_MAX_IV_LENGTH);
    memset(iv, 0, EVP_MAX_IV_LENGTH);

    // The below three arrays are ALL allocated and then cleaned-up inside this
    // fuction
    // (Using the below nested class, _OTEnv_Seal.) The first array will contain
    // useful pointers, but we do NOT delete those.
    // The next array contains pointers that we DO need to cleanup.
    // The final array contains integers (sizes.)
    //
    EVP_PKEY** array_pubkey =
        nullptr; // These will be pointers we use, but do NOT need to clean-up.
    uint8_t** ek = nullptr;   // These we DO need to cleanup...
    int32_t* eklen = nullptr; // This will just be an array of integers.

    bool bFinalized = false; // If this is set true, then we don't bother to
                             // cleanup the ctx. (See the destructor below.)

    // This class is used as a nested function, for easier cleanup. (C++ doesn't
    // directly support nested functions.)
    // Basically it translates the incoming RecipPubKeys into the low-level
    // arrays
    // ek and eklen (that OpenSSL needs.) This also cleans up those same arrays,
    // once
    // this object destructs (when we leave scope of this function.)
    //
    class _OTEnv_Seal
    {
    private:
        _OTEnv_Seal(const _OTEnv_Seal&);
        _OTEnv_Seal& operator=(const _OTEnv_Seal&);
        const char* m_szFunc;
        EVP_CIPHER_CTX& m_ctx;      // reference to openssl cipher context.
        EVP_PKEY*** m_array_pubkey; // pointer to array of public key pointers.
        uint8_t*** m_ek;   // pointer to array of encrypted symmetric keys.
        int32_t** m_eklen; // pointer to array of lengths for each encrypted
                           // symmetric key
        mapOfAsymmetricKeys& m_RecipPubKeys; // array of public keys (to
                                             // initialize the above members
                                             // with.)
        int32_t m_nLastPopulatedIndex; // We store the highest-populated index
                                       // (so we can free() up 'til the same
                                       // index, in destructor.)
        bool& m_bFinalized;

    public:
        _OTEnv_Seal(const char* param_szFunc, EVP_CIPHER_CTX& theCTX,
                    EVP_PKEY*** param_array_pubkey, uint8_t*** param_ek,
                    int32_t** param_eklen,
                    mapOfAsymmetricKeys& param_RecipPubKeys,
                    bool& param_Finalized)
            : m_szFunc(param_szFunc)
            , m_ctx(theCTX)
            , m_array_pubkey(nullptr)
            , m_ek(nullptr)
            , m_eklen(nullptr)
            , m_RecipPubKeys(param_RecipPubKeys)
            , m_nLastPopulatedIndex(-1)
            , m_bFinalized(param_Finalized)
        {
            if (nullptr == param_szFunc) OT_FAIL;
            if (nullptr == param_array_pubkey) OT_FAIL;
            if (nullptr == param_ek) OT_FAIL;
            if (nullptr == param_eklen) OT_FAIL;
            OT_ASSERT(!m_RecipPubKeys.empty());

            // Notice that each variable is a "pointer to" the actual array that
            // was passed in.
            // (So use them that way, inside this class,
            //  like this:    *m_ek   and   *m_eklen )
            //
            m_array_pubkey = param_array_pubkey;
            m_ek = param_ek;
            m_eklen = param_eklen;

            // EVP_CIPHER_CTX_init() corresponds to: EVP_CIPHER_CTX_cleanup()
            // EVP_CIPHER_CTX_cleanup clears all information from a cipher
            // context and free up any allocated
            // memory associate with it. It should be called after all
            // operations using a cipher are complete
            // so sensitive information does not remain in memory.
            //
            EVP_CIPHER_CTX_init(&m_ctx);

            // (*m_array_pubkey)[] array must have m_RecipPubKeys.size() no. of
            // elements (each containing a pointer
            // to an EVP_PKEY that we must NOT clean up.)
            //
            *m_array_pubkey = static_cast<EVP_PKEY**>(
                malloc(m_RecipPubKeys.size() * sizeof(EVP_PKEY*)));
            OT_ASSERT(nullptr != *m_array_pubkey);
            memset(*m_array_pubkey, 0,
                   m_RecipPubKeys.size() *
                       sizeof(EVP_PKEY*)); // size of array length *
                                           // sizeof(pointer)

            // (*m_ek)[] array must have m_RecipPubKeys.size() no. of elements
            // (each will contain a pointer from OpenSSL that we must clean up.)
            //
            *m_ek = static_cast<uint8_t**>(
                malloc(m_RecipPubKeys.size() * sizeof(uint8_t*)));
            if (nullptr == *m_ek) OT_FAIL;
            memset(*m_ek, 0, m_RecipPubKeys.size() *
                                 sizeof(uint8_t*)); // size of array length *
                                                    // sizeof(pointer)

            // (*m_eklen)[] array must also have m_RecipPubKeys.size() no. of
            // elements (each containing a size as integer.)
            //
            *m_eklen = static_cast<int32_t*>(
                malloc(m_RecipPubKeys.size() * sizeof(int32_t)));
            OT_ASSERT(nullptr != *m_eklen);
            memset(*m_eklen, 0, m_RecipPubKeys.size() *
                                    sizeof(int32_t)); // size of array length *
                                                      // sizeof(int32_t)

            //
            // ABOVE is all just above allocating the memory and setting it to 0
            // / nullptr.
            //
            // Whereas BELOW is about populating that memory, so the actual
            // OTEnvelope::Seal() function can use it.
            //

            int32_t nKeyIndex = -1; // it will be 0 upon first iteration.

            for (auto& it : m_RecipPubKeys) {
                ++nKeyIndex; // 0 on first iteration.
                m_nLastPopulatedIndex = nKeyIndex;

                OTAsymmetricKey* pTempPublicKey =
                    it.second; // first is the NymID
                OT_ASSERT(nullptr != pTempPublicKey);

                OTAsymmetricKey_OpenSSL* pPublicKey =
                    dynamic_cast<OTAsymmetricKey_OpenSSL*>(pTempPublicKey);
                OT_ASSERT(nullptr != pPublicKey);

                EVP_PKEY* public_key =
                    const_cast<EVP_PKEY*>(pPublicKey->dp->GetKey());
                OT_ASSERT(nullptr != public_key);

                // Copy the public key pointer to an array of public key
                // pointers...
                //
                (*m_array_pubkey)[nKeyIndex] =
                    public_key; // For OpenSSL, it needs an array of ALL the
                                // public keys.

                // We allocate enough space for the encrypted symmetric key to
                // be placed
                // at this index (the space determined based on size of the
                // public key that
                // the symmetric key will be encrypted to.) The space is left
                // empty, for OpenSSL
                // to populate.

                // (*m_ek)[i] must have room for EVP_PKEY_size(pubk[i]) bytes.
                (*m_ek)[nKeyIndex] =
                    static_cast<uint8_t*>(malloc(EVP_PKEY_size(public_key)));
                OT_ASSERT(nullptr != (*m_ek)[nKeyIndex]);
                memset((*m_ek)[nKeyIndex], 0, EVP_PKEY_size(public_key));
            }
        }

        ~_OTEnv_Seal()
        {
            OT_ASSERT(nullptr != m_array_pubkey); // 1. pointer to an array of
                                                  // pointers to EVP_PKEY,
            OT_ASSERT(nullptr != m_ek); // 2. pointer to an array of pointers to
                                        // encrypted symmetric keys
            OT_ASSERT(nullptr != m_eklen); // 3. pointer to an array storing the
                                           // lengths of those keys.

            // Iterate the array of encrypted symmetric keys, and free the key
            // at each index...
            //
            // We know how many there are, because each pointer will otherwise
            // be nullptr.
            // Plus we have m_nLastPopulatedIndex, which is obviously as far as
            // we will go.
            //

            int32_t nKeyIndex = -1; // it will be 0 upon first iteration.
            while (nKeyIndex < m_nLastPopulatedIndex) // if
                                                      // m_nLastPopulatedIndex
                                                      // is 0, then this loop
                                                      // will iterate ONCE, with
                                                      // nKeyIndex incrementing
                                                      // to 0 on the first line.
            {
                ++nKeyIndex; // 0 on first iteration.

                OT_ASSERT(nullptr != (*m_ek)[nKeyIndex]);

                free((*m_ek)[nKeyIndex]);
                (*m_ek)[nKeyIndex] = nullptr;
            }

            //
            // Now free all of the arrays:
            // 1. an array of pointers to EVP_PKEY,
            // 2. an array of pointers to encrypted symmetric keys
            //    (those all being nullptr pointers due to the above
            // while-loop),
            //    and
            // 3. an array storing the lengths of those keys.
            //

            if (nullptr !=
                *m_array_pubkey) // NOTE: The individual pubkeys are NOT
                                 // to be cleaned up, but this array,
                                 // containing pointers to those
                                 // pubkeys, IS cleaned up.
                free(*m_array_pubkey);
            *m_array_pubkey = nullptr;
            m_array_pubkey = nullptr;
            if (nullptr != *m_ek) free(*m_ek);
            *m_ek = nullptr;
            m_ek = nullptr;
            if (nullptr != *m_eklen) free(*m_eklen);
            *m_eklen = nullptr;
            m_eklen = nullptr;

            // EVP_CIPHER_CTX_cleanup returns 1 for success and 0 for failure.
            // EVP_EncryptFinal(), EVP_DecryptFinal() and EVP_CipherFinal()
            // behave in a similar way to EVP_EncryptFinal_ex(),
            // EVP_DecryptFinal_ex() and EVP_CipherFinal_ex() except ctx is
            // automatically cleaned up after the call.
            //
            if (!m_bFinalized) {
                // We only clean this up here, if the "Final" Seal function
                // didn't get called. (It normally
                // would have done this for us.)

                if (0 == EVP_CIPHER_CTX_cleanup(&m_ctx))
                    otErr << m_szFunc << ": Failure in EVP_CIPHER_CTX_cleanup. "
                                         "(It returned 0.)\n";
            }
        }
    }; // class _OTEnv_Seal

    // INSTANTIATE IT (This does all our setup on construction here, AND cleanup
    // on destruction, whenever exiting this function.)

    _OTEnv_Seal local_RAII(szFunc, ctx, &array_pubkey, &ek, &eklen,
                           RecipPubKeys, bFinalized);

    // This is where the envelope final contents will be placed.
    // including the size of the encrypted symmetric key, the symmetric key
    // itself, the initialization vector, and the ciphertext.
    //
    dataOutput.Release();

    const EVP_CIPHER* cipher_type = EVP_aes_128_cbc(); // todo hardcoding.

    /*
    int32_t EVP_SealInit(EVP_CIPHER_CTX* ctx, const EVP_CIPHER* type,
                     uint8_t **ek, int32_t* ekl, uint8_t* iv,
                     EVP_PKEY **pubk,     int32_t npubk);

     -- ek is an array of buffers where the public-key-encrypted secret key will
    be written (for each recipient.)
     -- Each buffer must contain enough room for the corresponding encrypted
    key: that is,
            ek[i] must have room for EVP_PKEY_size(pubk[i]) bytes.
     -- The actual size of each encrypted secret key is written to the array
    ekl.
     -- pubk is an array of npubk public keys.
     */

    //    EVP_PKEY      ** array_pubkey = nullptr;  // These will be pointers we
    // use, but do NOT need to clean-up.
    //    uint8_t ** ek           = nullptr;  // These we DO need to cleanup...
    //    int32_t           *  eklen        = nullptr;  // This will just be an
    // array of integers.

    if (!EVP_SealInit(
            &ctx, cipher_type, ek,
            eklen, // array of buffers for output of encrypted copies of the
                   // symmetric "session key". (Plus array of ints, to receive
                   // the size of each key.)
            iv,    // A buffer where the generated IV is written. Must contain
                   // room for the corresponding cipher's IV, as determined by
                   // (for example) EVP_CIPHER_iv_length(type).
            array_pubkey,
            static_cast<int32_t>(RecipPubKeys.size()))) // array of public keys
                                                        // we are addressing
                                                        // this envelope to.
    {
        otErr << szFunc << ": EVP_SealInit: failed.\n";
        return false;
    }

    // Write the ENVELOPE TYPE (network order version.)
    //
    // 0 == Error
    // 1 == Asymmetric Key  (this function -- Seal / Open)
    // 2 == Symmetric Key   (other functions -- Encrypt / Decrypt use this.)
    // Anything else: error.

    uint16_t temp_env_type = 1; // todo hardcoding.
    uint16_t env_type_n = static_cast<uint16_t>(
        htons(static_cast<uint16_t>(temp_env_type))); // Calculate
                                                      // "network-order" version
                                                      // of envelope type 1.

    dataOutput.Concatenate(reinterpret_cast<void*>(&env_type_n),
                           static_cast<uint32_t>(sizeof(env_type_n)));

    // Write the ARRAY SIZE (network order version.)

    uint32_t array_size_n = static_cast<uint32_t>(
        htonl(static_cast<uint32_t>(RecipPubKeys.size()))); // Calculate
                                                            // "network-order"
                                                            // version of array
                                                            // size.

    dataOutput.Concatenate(reinterpret_cast<void*>(&array_size_n),
                           static_cast<uint32_t>(sizeof(array_size_n)));

    otLog5 << __FUNCTION__
           << ": Envelope type:  " << static_cast<int32_t>(ntohs(env_type_n))
           << "    Array size: " << static_cast<int64_t>(ntohl(array_size_n))
           << "\n";

    OT_ASSERT(nullptr != ek);
    OT_ASSERT(nullptr != eklen);

    // Loop through the encrypted symmetric keys, and for each, write its
    // network-order NymID size, and its NymID, and its network-order content
    // size,
    // and its content, to the envelope data contents
    // (that we are currently building...)
    //
    int32_t ii = -1; // it will be 0 upon first iteration.

    for (auto& it : RecipPubKeys) {
        ++ii; // 0 on first iteration.

        std::string str_nym_id = it.first;
        //        OTAsymmetricKey * pTempPublicKey = it->second;
        //        OT_ASSERT(nullptr != pTempPublicKey);

        //        OTAsymmetricKey_OpenSSL * pPublicKey =
        // dynamic_cast<OTAsymmetricKey_OpenSSL*>(pTempPublicKey);
        //        OT_ASSERT(nullptr != pPublicKey);

        //        OTIdentifier theNymID;
        //        bool bCalculatedID = pPublicKey->CalculateID(theNymID); //
        // Only works for public keys.
        //
        //        if (!bCalculatedID)
        //        {
        //            otErr << "%s: Error trying to calculate ID of
        // recipient.\n", szFunc);
        //            return false;
        //        }

        const OTString strNymID(str_nym_id.c_str());

        uint32_t nymid_len = static_cast<uint32_t>(
            strNymID.GetLength() + 1); // +1 for null terminator.
        uint32_t nymid_len_n = static_cast<uint32_t>(
            htonl(nymid_len)); // Calculate "network-order" version of length
                               // (+1 for null terminator)

        // Write nymid_len_n and strNymID for EACH encrypted symmetric key.
        //
        dataOutput.Concatenate(reinterpret_cast<void*>(&nymid_len_n),
                               static_cast<uint32_t>(sizeof(nymid_len_n)));

        dataOutput.Concatenate(
            reinterpret_cast<const void*>(strNymID.Get()),
            static_cast<uint32_t>(nymid_len)); // (+1 for null terminator is
                                               // included here already, from
                                               // above.)

        otLog5 << __FUNCTION__ << ": INDEX: " << static_cast<int64_t>(ii)
               << "  NymID length:  "
               << static_cast<int64_t>(ntohl(nymid_len_n))
               << "   Nym ID: " << strNymID
               << "   Strlen (should be a byte shorter): "
               << static_cast<int64_t>(strNymID.GetLength()) << "\n";

        //      Write eklen_n and ek for EACH encrypted symmetric key,
        //
        //        EVP_PKEY      ** array_pubkey = nullptr;  // These will be
        // pointers we use, but do NOT need to clean-up.
        //        uint8_t ** ek           = nullptr;  // These we DO need to
        // cleanup...
        //        int32_t           *  eklen        = nullptr;  // This will
        // just
        // be an array of integers.

        OT_ASSERT(nullptr != ek[ii]); // assert key pointer not null.
        OT_ASSERT(eklen[ii] > 0);     // assert key length larger than 0.

        uint32_t eklen_n = static_cast<uint32_t>(htonl(static_cast<uint32_t>(
            eklen[ii]))); // Calculate "network-order" version of length.

        dataOutput.Concatenate(reinterpret_cast<void*>(&eklen_n),
                               static_cast<uint32_t>(sizeof(eklen_n)));

        dataOutput.Concatenate(reinterpret_cast<void*>(ek[ii]),
                               static_cast<uint32_t>(eklen[ii]));

        otLog5 << __FUNCTION__
               << ": EK length:  " << static_cast<int64_t>(ntohl(eklen_n))
               << "     First byte: " << static_cast<int32_t>((ek[ii])[0])
               << "      Last byte: "
               << static_cast<int32_t>((ek[ii])[eklen[ii] - 1]) << "\n";
    }

    // Write IV size before then writing IV itself.
    //
    uint32_t ivlen = static_cast<uint32_t>(
        EVP_CIPHER_iv_length(cipher_type)); // Length of IV for this cipher...
                                            // (TODO: add cipher name to output,
                                            // and use it for looking up cipher
                                            // upon Open.)
                                            //  OT_ASSERT(ivlen > 0);
    uint32_t ivlen_n = static_cast<uint32_t>(htonl(static_cast<uint32_t>(
        ivlen))); // Calculate "network-order" version of iv length.

    dataOutput.Concatenate(reinterpret_cast<void*>(&ivlen_n),
                           static_cast<uint32_t>(sizeof(ivlen_n)));

    dataOutput.Concatenate(reinterpret_cast<void*>(iv),
                           static_cast<uint32_t>(ivlen));

    otLog5 << __FUNCTION__
           << ": iv_size: " << static_cast<int64_t>(ntohl(ivlen_n))
           << "   IV first byte: " << static_cast<int32_t>(iv[0])
           << "    IV last byte: " << static_cast<int32_t>(iv[ivlen - 1])
           << "   \n";

    // Next we put the plaintext into a data object so we can process it via
    // EVP_SealUpdate,
    // in blocks, into encrypted form in dataOutput. Each iteration of the loop
    // processes
    // one block.
    //
    OTData plaintext(static_cast<const void*>(theInput.Get()),
                     theInput.GetLength() + 1); // +1 for null terminator

    // Now we process the input and write the encrypted data to the
    // output.
    //
    while (0 <
           (len = plaintext.OTfread(reinterpret_cast<uint8_t*>(buffer),
                                    static_cast<uint32_t>(sizeof(buffer))))) {
        if (!EVP_SealUpdate(&ctx, buffer_out, &len_out, buffer,
                            static_cast<int32_t>(len))) {
            otErr << szFunc << ": EVP_SealUpdate failed.\n";
            return false;
        }
        else if (len_out > 0)
            dataOutput.Concatenate(reinterpret_cast<void*>(buffer_out),
                                   static_cast<uint32_t>(len_out));
        else
            break;
    }

    if (!EVP_SealFinal(&ctx, buffer_out, &len_out)) {
        otErr << szFunc << ": EVP_SealFinal failed.\n";
        return false;
    }
    // This is the "final" piece that is added from SealFinal just above.
    //
    else if (len_out > 0) {
        bFinalized = true;
        dataOutput.Concatenate(reinterpret_cast<void*>(buffer_out),
                               static_cast<uint32_t>(len_out));
    }
    else {
        // cppcheck-suppress unreadVariable
        bFinalized = true;
    }

    return true;
}

/*
#include <openssl/evp.h>
int32_t EVP_OpenInit(EVP_CIPHER_CTX* ctx, EVP_CIPHER* type, uint8_t* ek,
                 int32_t ekl, uint8_t* iv, EVP_PKEY* priv);
int32_t EVP_OpenUpdate(EVP_CIPHER_CTX* ctx, uint8_t* out, int32_t* outl,
uint8_t* in, int32_t inl);
int32_t EVP_OpenFinal(EVP_CIPHER_CTX* ctx, uint8_t* out, int32_t* outl);
DESCRIPTION

The EVP envelope routines are a high level interface to envelope decryption.
They decrypt a public key
 encrypted symmetric key and then decrypt data using it.

 int32_t EVP_OpenInit(EVP_CIPHER_CTX* ctx, EVP_CIPHER* type, uint8_t* ek,
int32_t
ekl, uint8_t* iv, EVP_PKEY* priv);
EVP_OpenInit() initializes a cipher context ctx for decryption with cipher type.
It decrypts the encrypted
 symmetric key of length ekl bytes passed in the ek parameter using the private
key priv. The IV is supplied
 in the iv parameter.

EVP_OpenUpdate() and EVP_OpenFinal() have exactly the same properties as the
EVP_DecryptUpdate() and
 EVP_DecryptFinal() routines, as documented on the EVP_EncryptInit(3) manual
page.

NOTES

It is possible to call EVP_OpenInit() twice in the same way as
EVP_DecryptInit(). The first call should have
 priv set to nullptr and (after setting any cipher parameters) it should be
called
again with type set to nullptr.

If the cipher passed in the type parameter is a variable length cipher then the
key length will be set to the
value of the recovered key length. If the cipher is a fixed length cipher then
the recovered key length must
match the fixed cipher length.

RETURN VALUES

EVP_OpenInit() returns 0 on error or a non zero integer (actually the recovered
secret key size) if successful.

EVP_OpenUpdate() returns 1 for success or 0 for failure.

EVP_OpenFinal() returns 0 if the decrypt failed or 1 for success.
*/

// RSA / AES

bool OTCrypto_OpenSSL::Open(OTData& dataInput, const OTPseudonym& theRecipient,
                            OTString& theOutput,
                            const OTPasswordData* pPWData) const
{
    const char* szFunc = "OTCrypto_OpenSSL::Open";

    uint8_t buffer[4096];
    uint8_t buffer_out[4096 + EVP_MAX_IV_LENGTH];
    uint8_t iv[EVP_MAX_IV_LENGTH];

    uint32_t len = 0;
    int32_t len_out = 0;
    bool bFinalized = false; // We only clean up the ctx if the Open "Final"
                             // function hasn't been called, since it does that
                             // automatically already.

    memset(buffer, 0, 4096);
    memset(buffer_out, 0, 4096 + EVP_MAX_IV_LENGTH);
    memset(iv, 0, EVP_MAX_IV_LENGTH);

    // theOutput is where we'll put the decrypted result.
    //
    theOutput.Release();

    // Grab the NymID of the recipient, so we can find his session
    // key (there might be symmetric keys for several Nyms, not just this
    // one, and we need to find the right one in order to perform this Open.)
    //
    OTString strNymID;
    theRecipient.GetIdentifier(strNymID);

    OTAsymmetricKey& theTempPrivateKey =
        const_cast<OTAsymmetricKey&>(theRecipient.GetPrivateEncrKey());

    OTAsymmetricKey_OpenSSL* pPrivateKey =
        dynamic_cast<OTAsymmetricKey_OpenSSL*>(&theTempPrivateKey);
    OT_ASSERT(nullptr != pPrivateKey);

    EVP_PKEY* private_key =
        const_cast<EVP_PKEY*>(pPrivateKey->dp->GetKey(pPWData));

    if (nullptr == private_key) {
        otErr << szFunc
              << ": Null private key on recipient. (Returning false.)\n";
        return false;
    }
    else
        otLog5 << __FUNCTION__
               << ": Private key is available for NymID: " << strNymID << " \n";

    EVP_CIPHER_CTX ctx;

    class _OTEnv_Open
    {
    private:
        const char* m_szFunc;
        EVP_CIPHER_CTX& m_ctx;         // reference to openssl cipher context.
        OTAsymmetricKey& m_privateKey; // reference to OTAsymmetricKey object.
        bool& m_bFinalized;

    public:
        _OTEnv_Open(const char* param_szFunc, EVP_CIPHER_CTX& theCTX,
                    OTAsymmetricKey& param_privateKey, bool& param_Finalized)
            : m_szFunc(param_szFunc)
            , m_ctx(theCTX)
            , m_privateKey(param_privateKey)
            , m_bFinalized(param_Finalized)
        {
            OT_ASSERT(nullptr != param_szFunc);

            EVP_CIPHER_CTX_init(&m_ctx);
        }

        ~_OTEnv_Open() // DESTRUCTOR
        {
            m_privateKey.ReleaseKey();
            //
            // BELOW this point, private_key (which is a member of m_privateKey
            // is either
            // cleaned up, or kept based on a timer value. (It MAY not be
            // cleaned up,
            // depending on its state.)

            // EVP_CIPHER_CTX_cleanup returns 1 for success and 0 for failure.
            //
            if (!m_bFinalized) {
                if (0 == EVP_CIPHER_CTX_cleanup(&m_ctx))
                    otErr << m_szFunc << ": Failure in EVP_CIPHER_CTX_cleanup. "
                                         "(It returned 0.)\n";
            }

            m_szFunc = nullptr;
        }
    };

    // INSTANTIATE the clean-up object.
    //
    _OTEnv_Open theNestedInstance(szFunc, ctx, *pPrivateKey, bFinalized);

    dataInput.reset(); // Reset the fread position on this object to 0.

    uint32_t nRunningTotal =
        0; // Everytime we read something, we add the length to this variable.

    uint32_t nReadEnvType = 0;
    uint32_t nReadArraySize = 0;
    uint32_t nReadIV = 0;

    // Read the ARRAY SIZE (network order version -- convert to host version.)

    // Loop through the array of encrypted symmetric keys, and for each:
    //      read its network-order NymID size (convert to host version), and
    // then read its NymID,
    //      read its network-order key content size (convert to host), and then
    // read its key content,

    //
    // Read network-order IV size (convert to host version) before then reading
    // IV itself.
    // (Then update encrypted blocks until evp open final...)
    //

    // So here we go...

    //
    // Read the ENVELOPE TYPE (as network order version -- and convert to host
    // version.)
    //
    // 0 == Error
    // 1 == Asymmetric Key  (this function -- Seal / Open)
    // 2 == Symmetric Key   (other functions -- Encrypt / Decrypt use this.)
    // Anything else: error.
    //
    uint16_t env_type_n = 0;

    if (0 == (nReadEnvType = dataInput.OTfread(
                  reinterpret_cast<uint8_t*>(&env_type_n),
                  static_cast<uint32_t>(sizeof(env_type_n))))) {
        otErr << szFunc << ": Error reading Envelope Type. Expected "
                           "asymmetric(1) or symmetric (2).\n";
        return false;
    }
    nRunningTotal += nReadEnvType;
    OT_ASSERT(nReadEnvType == static_cast<uint32_t>(sizeof(env_type_n)));

    // convert that envelope type from network to HOST endian.
    //
    const uint16_t env_type =
        static_cast<uint16_t>(ntohs(static_cast<uint16_t>(env_type_n)));
    //  nRunningTotal += env_type;    // NOPE! Just because envelope type is 1
    // or 2, doesn't mean we add 1 or 2 extra bytes to the length here. Nope!

    if (1 != env_type) {
        otErr << szFunc << ": Error : Expected Envelope for Asymmetric "
                           "key(type 1) but instead found type "
              << static_cast<int32_t>(env_type) << ".\n";
        print_stacktrace();
        return false;
    }
    else
        otLog5 << __FUNCTION__
               << ": Envelope type: " << static_cast<int32_t>(env_type) << "\n";

    // Read the ARRAY SIZE (network order version -- convert to host version.)
    //
    uint32_t array_size_n = 0;

    if (0 == (nReadArraySize = dataInput.OTfread(
                  reinterpret_cast<uint8_t*>(&array_size_n),
                  static_cast<uint32_t>(sizeof(array_size_n))))) {
        otErr << szFunc
              << ": Error reading Array Size for encrypted symmetric keys.\n";
        return false;
    }
    nRunningTotal += nReadArraySize;
    OT_ASSERT(nReadArraySize == static_cast<uint32_t>(sizeof(array_size_n)));

    // convert that array size from network to HOST endian.
    //
    const uint32_t array_size = ntohl(array_size_n);

    otLog5 << __FUNCTION__
           << ": Array size: " << static_cast<int64_t>(array_size) << "\n";

    //  nRunningTotal += array_size;    // NOPE! Just because there are 10 array
    // elements doesn't mean I want to add "10" here to the running total!! Not
    // logical.

    // We are going to loop through all the keys and load each up (then delete.)
    // Each one is proceeded by its length.
    // IF we find the one we are looking for, then we set it onto this variable,
    // theRawEncryptedKey, so we have it available below this loop.
    //
    OTData theRawEncryptedKey;
    bool bFoundKeyAlready =
        false; // If we find it during the loop below, we'll set this to true.

    // Loop through as we read the encrypted symmetric keys, and for each:
    //      read its network-order NymID size (convert to host version), and
    // then read its NymID,
    //      read its network-order key content size (convert to host), and then
    // read its key content,
    //
    for (uint32_t ii = 0; ii < array_size; ++ii) {

        // Loop through the encrypted symmetric keys, and for each:
        //      read its network-order NymID size (convert to host version), and
        // then read its NymID,
        //      read its network-order key content size (convert to host), and
        // then read its key content.

        uint32_t nymid_len_n = 0;
        uint32_t nReadNymIDSize = 0;

        if (0 == (nReadNymIDSize = dataInput.OTfread(
                      reinterpret_cast<uint8_t*>(&nymid_len_n),
                      static_cast<uint32_t>(sizeof(nymid_len_n))))) {
            otErr << szFunc << ": Error reading NymID length for an encrypted "
                               "symmetric key.\n";
            return false;
        }
        nRunningTotal += nReadNymIDSize;
        OT_ASSERT(nReadNymIDSize == static_cast<uint32_t>(sizeof(nymid_len_n)));

        // convert that array size from network to HOST endian.
        //
        uint32_t nymid_len = static_cast<uint32_t>(ntohl(static_cast<uint32_t>(
            nymid_len_n))); // FYI: ntohl returns uint32_t !!!!!

        otLog5 << __FUNCTION__
               << ": NymID length: " << static_cast<int64_t>(nymid_len) << "\n";

        //      nRunningTotal += nymid_len; // Nope!

        uint8_t* nymid =
            static_cast<uint8_t*>(malloc(sizeof(uint8_t) * nymid_len));
        OT_ASSERT(nullptr != nymid);
        nymid[0] = '\0'; // null terminator.

        uint32_t nReadNymID = 0;

        if (0 == (nReadNymID = dataInput.OTfread(
                      reinterpret_cast<uint8_t*>(nymid),
                      static_cast<uint32_t>(sizeof(uint8_t) *
                                            nymid_len)))) // this length
                                                          // includes the null
                                                          // terminator (it was
                                                          // written that way.)
        {
            otErr << szFunc
                  << ": Error reading NymID for an encrypted symmetric key.\n";
            free(nymid);
            nymid = nullptr;
            return false;
        }
        nRunningTotal += nReadNymID;
        OT_ASSERT(nReadNymID ==
                  static_cast<uint32_t>(sizeof(uint8_t) * nymid_len));
        //      OT_ASSERT(nymid_len == nReadNymID);

        nymid[nymid_len - 1] = '\0'; // for null terminator. If string is 10
                                     // bytes int64_t, it's from 0-9, and the
                                     // null terminator is at index 9.
        const OTString loopStrNymID(reinterpret_cast<char*>(nymid));
        free(nymid);
        nymid = nullptr;

        otLog5 << __FUNCTION__ << ": (LOOP) Current NymID: " << loopStrNymID
               << "    Strlen:  "
               << static_cast<int64_t>(loopStrNymID.GetLength()) << "\n";

        // loopStrNymID ... if this matches strNymID then it's the one we're
        // looking for.
        // But we have to load it all either way, just to iterate through them,
        // so might
        // as well load it all first, then check. If it matches, we use it and
        // break.
        // Otherwise we keep iterating until we find it.
        //
        // Read its network-order key content size (convert to host-order), and
        // then
        // read its key content.
        uint8_t* ek = nullptr;
        uint32_t eklen = 0;
        uint32_t eklen_n = 0;
        uint32_t nReadLength = 0;
        uint32_t nReadKey = 0;

        // First we read the encrypted key size.
        //
        if (0 == (nReadLength = dataInput.OTfread(
                      reinterpret_cast<uint8_t*>(&eklen_n),
                      static_cast<uint32_t>(sizeof(eklen_n))))) {
            otErr << szFunc << ": Error reading encrypted key size.\n";
            return false;
        }
        nRunningTotal += nReadLength;
        OT_ASSERT(nReadLength == static_cast<uint32_t>(sizeof(eklen_n)));

        // convert that key size from network to host endian.
        //
        eklen = static_cast<uint32_t>(ntohl(static_cast<uint32_t>(eklen_n)));
        //      eklen  = EVP_PKEY_size(private_key);  // We read this size from
        // file now...

        otLog5 << __FUNCTION__
               << ": EK length:  " << static_cast<int64_t>(eklen) << "   \n";

        //      nRunningTotal += eklen;  // Nope!

        ek = static_cast<uint8_t*>(
            malloc(static_cast<int32_t>(eklen) *
                   sizeof(uint8_t))); // I assume this is for the AES key
        OT_ASSERT(nullptr != ek);
        memset(static_cast<void*>(ek), 0, static_cast<int32_t>(eklen));

        // Next we read the encrypted key itself...
        //
        if (0 == (nReadKey = dataInput.OTfread(reinterpret_cast<uint8_t*>(ek),
                                               static_cast<uint32_t>(eklen)))) {
            otErr << szFunc << ": Error reading encrypted key.\n";
            free(ek);
            ek = nullptr;
            return false;
        }
        nRunningTotal += nReadKey;

        otLog5 << __FUNCTION__
               << ":    EK First byte: " << static_cast<int32_t>(ek[0])
               << "     EK Last byte: " << static_cast<int32_t>(ek[eklen - 1])
               << "\n";

        OT_ASSERT(nReadKey == static_cast<uint32_t>(eklen));

        // If we "found the key already" that means we already found the right
        // key on
        // a previous iteration, so therefore we're *definitely* just going to
        // throw
        // THIS one away. We just continue on to the next iteration and keep
        // counting
        // the bytes.
        //
        if (!bFoundKeyAlready) {
            // We have NOT found the right key yet, so let's see if this is the
            // one we're looking for.

            const bool bNymIDMatches =
                strNymID.Compare(loopStrNymID); // FOUND IT! <==========

            if ((ii == (array_size - 1)) || // If we're on the LAST INDEX in the
                                            // array (often the only index), OR
                                            // if the
                bNymIDMatches) // NymID is a guaranteed match, then we'll try to
                               // decrypt using this session key.
            { // (Of course also we know that we haven't found the Key yet, or
                // we wouldn't even be here.)
                // NOTE: What if we're on the last index, but the NymID DOES
                // exist, and it DEFINITELY doesn't match?
                // In other words, if loopStrNymID EXISTS, and it DEFINITELY
                // doesn't match (bNymIDMatches is false) then we
                // DEFINITELY want to skip it. But if bNymIDMatches is false
                // simply because loopStrNymID is EMPTY, then we
                // can't rule that key out, in that case.
                //
                if (!(loopStrNymID.Exists() &&
                      !bNymIDMatches)) // Skip if ID was definitely found and
                                       // definitely doesn't match.
                {
                    bFoundKeyAlready = true;

                    theRawEncryptedKey.Assign(static_cast<void*>(ek),
                                              static_cast<uint32_t>(eklen));
                    //                  theRawEncryptedKey.Assign(const_cast<const
                    // void *>(static_cast<void *>(ek)), eklen);
                }
            }
        }

        free(ek);
        ek = nullptr;

    } // for

    if (!bFoundKeyAlready) // Todo: AND if list of POTENTIAL matches is
                           // also empty...
    {
        otOut << __FUNCTION__
              << ": Sorry: Unable to find a session key for the Nym attempting "
                 "to open this envelope: " << strNymID << "\n";
        return false;
    }

    // Read network-order IV size (convert to host version) before then reading
    // IV itself.
    // (Then update encrypted blocks until evp open final...)
    //
    const uint32_t max_iv_length =
        OTCryptoConfig::SymmetricIvSize(); // I believe this is a max length, so
                                           // it may not match the actual
                                           // length.

    // Read the IV SIZE (network order version -- convert to host version.)
    //
    uint32_t iv_size_n = 0;
    uint32_t nReadIVSize = 0;

    if (0 == (nReadIVSize = dataInput.OTfread(
                  reinterpret_cast<uint8_t*>(&iv_size_n),
                  static_cast<uint32_t>(sizeof(iv_size_n))))) {
        otErr << szFunc
              << ": Error reading IV Size for encrypted symmetric keys.\n";
        return false;
    }
    nRunningTotal += nReadIVSize;
    OT_ASSERT(nReadIVSize == static_cast<uint32_t>(sizeof(iv_size_n)));

    // convert that iv size from network to HOST endian.
    //
    const uint32_t iv_size_host_order = ntohl(static_cast<uint32_t>(iv_size_n));

    if (iv_size_host_order > max_iv_length) {
        const int64_t l1 = iv_size_host_order, l2 = max_iv_length;
        otErr << __FUNCTION__ << ": Error: iv_size (" << l1
              << ") is larger than max_iv_length (" << l2 << ").\n";
        return false;
    }
    else
        otLog5 << __FUNCTION__
               << ": IV size: " << static_cast<int64_t>(iv_size_host_order)
               << "\n";

    // Then read the IV (initialization vector) itself.
    //
    if (0 == (nReadIV = dataInput.OTfread(
                  reinterpret_cast<uint8_t*>(iv),
                  static_cast<uint32_t>(iv_size_host_order)))) {
        otErr << szFunc << ": Error reading initialization vector.\n";
        return false;
    }

    nRunningTotal += nReadIV;
    OT_ASSERT(nReadIV == static_cast<uint32_t>(iv_size_host_order));

    otLog5 << __FUNCTION__
           << ":    IV First byte: " << static_cast<int32_t>(iv[0])
           << "     IV Last byte: "
           << static_cast<int32_t>(iv[iv_size_host_order - 1]) << "\n";

    // We read the encrypted key size, then we read the encrypted key itself,
    // with nReadKey containing
    // the number of bytes actually read. The IF statement says "if 0 ==" but it
    // should probably say
    // "if eklen !=" (right?) Wrong: because I think it's a max length.
    //
    // We create an OTData object to store the ciphertext itself, which begins
    // AFTER the end of the IV.
    // So we see pointer + nRunningTotal as the starting point for the
    // ciphertext.
    // the size of the ciphertext, meanwhile, is the size of the entire thing,
    // MINUS nRunningTotal.
    //
    OTData ciphertext(static_cast<const void*>(
                          static_cast<const uint8_t*>(dataInput.GetPointer()) +
                          nRunningTotal),
                      dataInput.GetSize() - nRunningTotal);

    //
    const EVP_CIPHER* cipher_type = EVP_aes_128_cbc(); // todo hardcoding.

    // int32_t EVP_OpenInit(
    //          EVP_CIPHER_CTX *ctx,
    //          EVP_CIPHER *type,
    //          uint8_t *ek,
    //          int32_t ekl,
    //          uint8_t *iv,
    //          EVP_PKEY *priv);

    //  if (!EVP_OpenInit(&ctx, cipher_type, ek, eklen, iv, private_key))
    if (!EVP_OpenInit(&ctx, cipher_type, static_cast<const uint8_t*>(
                                             theRawEncryptedKey.GetPointer()),
                      static_cast<int32_t>(theRawEncryptedKey.GetSize()),
                      static_cast<const uint8_t*>(iv), private_key)) {

        // EVP_OpenInit() initializes a cipher context ctx for decryption with
        // cipher type. It decrypts the encrypted
        //    symmetric key of length ekl bytes passed in the ek parameter using
        // the private key priv. The IV is supplied
        //    in the iv parameter.

        otErr << szFunc << ": EVP_OpenInit: failed.\n";
        return false;
    }

    // Now we process ciphertext and write the decrypted data to plaintext.
    //
    OTData plaintext;

    // We loop through the ciphertext and process it in blocks...
    //
    while (0 <
           (len = ciphertext.OTfread(reinterpret_cast<uint8_t*>(buffer),
                                     static_cast<uint32_t>(sizeof(buffer))))) {
        if (!EVP_OpenUpdate(&ctx, buffer_out, &len_out, buffer,
                            static_cast<int32_t>(len))) {
            otErr << szFunc << ": EVP_OpenUpdate: failed.\n";
            return false;
        }
        else if (len_out > 0)
            plaintext.Concatenate(reinterpret_cast<void*>(buffer_out),
                                  static_cast<uint32_t>(len_out));
        else
            break;
    }

    if (!EVP_OpenFinal(&ctx, buffer_out, &len_out)) {
        otErr << szFunc << ": EVP_OpenFinal: failed.\n";
        return false;
    }
    else if (len_out > 0) {
        bFinalized = true;
        plaintext.Concatenate(reinterpret_cast<void*>(buffer_out),
                              static_cast<uint32_t>(len_out));

    }
    else {
        // cppcheck-suppress unreadVariable
        bFinalized = true;
    }

    // Make sure it's null-terminated...
    //
    uint32_t nIndex =
        plaintext.GetSize() - 1; // null terminator is already part of length
                                 // here (it was, or at least should have been,
                                 // sealed that way in the first place.)
    (static_cast<uint8_t*>(const_cast<void*>(plaintext.GetPointer())))[nIndex] =
        '\0';

    // Set it into theOutput (to return the plaintext to the caller)
    //
    // if size is 10, then indices are 0..9 and we pass '10' as the size here.
    // Since it's an OTData, then the 10th byte (at index 9) is expected to
    // contain
    // the null terminator.
    // Thus the ACTUAL string is only 9 bytes int64_t, and is contained in
    // indices 0..8.
    //
    const bool bSetMem = theOutput.MemSet(
        static_cast<const char*>(plaintext.GetPointer()), plaintext.GetSize());

    if (bSetMem)
        otLog5 << __FUNCTION__ << ": Output:\n" << theOutput << "\n\n";
    else
        otErr << __FUNCTION__ << ": Error: Failed while trying to memset from "
                                 "plaintext OTData to output OTString.\n";

    return bSetMem;
}

// If length is 10,
// Then indices are 0..9
// Therefore '9' is the 10th byte, starting from 0.
// Therefore "GetSize()" would be 10,
// and "GetSize()-1" would be 9, which is the 10th byte starting from 0.
// Therefore if the string is 9 bytes int64_t, it will have data from 0 through
// 8, with 9 being \0.
// Normally you wouldn't expect a string to include the null terminator as part
// of its length.
// But for OTData, you WOULD expect the null 0 to be at the end.
//

// The default hashing algorithm in this software should be one that XOR
// combines two other,
// established and respected algorithms. In this case, we use the "SAMY" hash
// which is actually
// SHA512 XOR'd with WHIRLPOOL (also 512 in output). Credit to SAMY for the
// idea.
//
// This way if one is ever cracked, our system is still strong, and we can swap
// it out.
// Thus, I had to write this special function so that if the Default hash
// algorithm is the one
// chosen, ("SAMY") then we have to hash it twice using Hash1 (SHA512) and Hash2
// (Whirlpool)
// before we encrypt it with the private key.
//
// Since the envelope (EVP) interface did not allow this, I had to Google
// everywhere to find
// lower-level code I could model.

/*
 128 bytes * 8 bits == 1024 bits key.  (RSA)

 64 bytes * 8 bits == 512 bits key (for WHIRLPOOL and SHA-512 message digests.)

 BUT--now I want to allow various key sizes (above 1024...)
 and I also have a smaller message digest now: 256 bits.

 */
bool OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::SignContractDefaultHash(
    const OTString& strContractUnsigned, const EVP_PKEY* pkey,
    OTSignature& theSignature, const OTPasswordData*) const
{
    const char* szFunc = "OTCrypto_OpenSSL::SignContractDefaultHash";

    bool bReturnValue = false;

    // These two contain the output of the two message digest
    // functions that we're using (SHA-256 and WHIRLPOOL.)
    // the two output hashes are then merged together into this one.
    std::vector<uint8_t> vOutputHash1(OTCryptoConfig::SymmetricKeySizeMax());
    std::vector<uint8_t> vOutputHash2(OTCryptoConfig::SymmetricKeySizeMax());
    std::vector<uint8_t> vDigest(OTCryptoConfig::SymmetricKeySizeMax());

    // This stores the message digest, pre-encrypted, but with the padding
    // added.
    // This stores the final signature, when the EM value has been signed by RSA
    // private key.
    std::vector<uint8_t> vEM(OTCryptoConfig::PublicKeysizeMax());
    std::vector<uint8_t> vpSignature(OTCryptoConfig::PublicKeysizeMax());

    uint32_t uDigest1Len =
        OTCryptoConfig::Digest1Size(); // 32 bytes == 256 bits. (These are used
                                       // for function output below, not input.)
    uint32_t uDigest2Len =
        OTCryptoConfig::Digest2Size(); // 64 bytes == 512 bits. (These are used
                                       // for function output below, not input.)

    EVP_MD_CTX mdHash1_ctx, mdHash2_ctx;

    //  OTPassword::zeroMemory(uint8_t* szMemory, uint32_t theSize);
    //  OTPassword::zeroMemory(void* vMemory,     uint32_t theSize);
    OTPassword::zeroMemory(&vOutputHash1.at(0),
                           OTCryptoConfig::SymmetricKeySizeMax());
    OTPassword::zeroMemory(&vOutputHash2.at(0),
                           OTCryptoConfig::SymmetricKeySizeMax());
    OTPassword::zeroMemory(&vDigest.at(0),
                           OTCryptoConfig::SymmetricKeySizeMax());
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

    // Since the idea of this special code is that we're using 2 hash
    // algorithms,
    // let's look them up and see what they are.
    // addendum: unless we're on Android... then there's only 1 hash algorithm.
    //
    const EVP_MD* digest1 =
        OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::GetOpenSSLDigestByName(
            OTIdentifier::HashAlgorithm1); // SHA-256

    if (nullptr == digest1) {
        otErr << szFunc << ": Failure to load message digest algorithm.\n";
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }

    // hash the contents of the contract with HashAlgorithm1 (SHA-256)
    EVP_MD_CTX_init(&mdHash1_ctx);
    EVP_DigestInit(&mdHash1_ctx, digest1); // digest1 is the actual algorithm
    EVP_DigestUpdate(&mdHash1_ctx, strContractUnsigned.Get(),
                     strContractUnsigned.GetLength()); // input
    EVP_DigestFinal(&mdHash1_ctx, &vOutputHash1.at(0),
                    &uDigest1Len);    // output and length
    EVP_MD_CTX_cleanup(&mdHash1_ctx); // cleanup

    /*
     TODO:
     The functions EVP_DigestInit(), EVP_DigestFinal() and EVP_MD_CTX_copy() are
     obsolete but are retained to maintain compatibility
     with existing code. New applications should use EVP_DigestInit_ex(),
     EVP_DigestFinal_ex() and EVP_MD_CTX_copy_ex() because they
     can efficiently reuse a digest context instead of initializing and cleaning
     it up on each call and allow non default implementations
     of digests to be specified.
     */
    //#ifndef ANDROID
    const EVP_MD* digest2 =
        OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::GetOpenSSLDigestByName(
            OTIdentifier::HashAlgorithm2); // WHIRLPOOL (512)

    if (nullptr == digest2) {
        otErr << szFunc << ": Failure to load message digest algorithm.\n";
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }

    // hash the same contents with HashAlgorithm2 (WHIRLPOOL)
    EVP_MD_CTX_init(&mdHash2_ctx);
    EVP_DigestInit(&mdHash2_ctx, digest2); // digest2 is the algorithm
    EVP_DigestUpdate(&mdHash2_ctx, strContractUnsigned.Get(),
                     strContractUnsigned.GetLength()); // Input
    EVP_DigestFinal(&mdHash2_ctx, &vOutputHash2.at(0),
                    &uDigest2Len);    // output and length
    EVP_MD_CTX_cleanup(&mdHash2_ctx); // cleanup

    // (Goes with the smaller size.)
    const uint32_t uDigestMergedLength =
        (uDigest1Len > uDigest2Len ? uDigest2Len : uDigest1Len);

    // XOR the two together
    //
    for (uint32_t i = 0; i < uDigestMergedLength; i++) {
        vDigest.at(i) = ((vOutputHash1.at(i)) ^ (vOutputHash2.at(i)));
    }
    //#else // ANDROID
    //    const uint32_t uDigestMergedLength = uDigest1Len;
    //
    //    for (int32_t i = 0; i < uDigestMergedLength; i++)
    //    {
    //        pDigest[i] = (vOutputHash1.at(i));
    //    }
    //#endif // ANDROID

    // pDigest is now set up.
    // uDigestMergedLength contains its length in bytes.

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
    int32_t status =
        RSA_padding_add_PKCS1_PSS(pRsaKey, &vEM.at(0), &vDigest.at(0), digest1,
                                  -2); // maximum salt length

    // Above, pDigest is the input, but its length is not needed, since it is
    // determined
    // by the digest algorithm (digest1.) In this case, that size is 32 bytes ==
    // 256 bits.

    // Also notice that digest1 and digest2 are both processed, and then digest1
    // is used here
    // again, since RSA_padding_add_PKCS1_PSS requires a digest. Might be
    // optimization opportunities there.
    //
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

    OTData binSignature(&vpSignature.at(0), status); // RSA_private_encrypt
                                                     // actually returns the
                                                     // right size.
    //    OTData binSignature(pSignature, 128);    // stop hardcoding this block
    // size.

    // theSignature that was passed in, now contains the final signature.
    // The contents were hashed twice, and the resulting hashes were
    // XOR'd together, and then padding was added, and then it was signed
    // with the private key.
    theSignature.SetData(binSignature, true); // true means, "yes, with newlines
                                              // in the b64-encoded output,
                                              // please."
    bReturnValue = true;

    if (pRsaKey) RSA_free(pRsaKey);
    pRsaKey = nullptr;

    return bReturnValue;
}

// Verify a contract that has been signed with our own default algorithm (aka
// SAMY hash)
// Basically we had to customize for that algorithm since, by default, it XORs
// two different
// algorithms together (SHA256 and WHIRLPOOL) in anticipation of the day that
// one of them is
// broken.

bool OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::VerifyContractDefaultHash(
    const OTString& strContractToVerify, const EVP_PKEY* pkey,
    const OTSignature& theSignature, const OTPasswordData*) const
{
    const char* szFunc = "OTCrypto_OpenSSL::VerifyContractDefaultHash";

    bool bReturnValue = false;

    std::vector<uint8_t> vOutputHash1(
        OTCryptoConfig::SymmetricKeySizeMax()); // These two contain the output
                                                // of the two message digest
    std::vector<uint8_t> vOutputHash2(
        OTCryptoConfig::SymmetricKeySizeMax()); // functions that we're using
                                                // (SHA-256 and WHIRLPOOL.)
    std::vector<uint8_t> vDigest(
        OTCryptoConfig::SymmetricKeySizeMax()); // the two output hashes are
                                                // then merged together into
                                                // this one.

    std::vector<uint8_t> vDecrypted(
        OTCryptoConfig::PublicKeysizeMax()); // Contains the decrypted
                                             // signature.

    uint32_t uDigest1Len =
        OTCryptoConfig::Digest1Size(); // 32 bytes == 256 bits. (These are used
                                       // for function output below, not input.)
    uint32_t uDigest2Len =
        OTCryptoConfig::Digest2Size(); // 64 bytes == 512 bits. (These are used
                                       // for function output below, not input.)

    EVP_MD_CTX mdHash1_ctx, mdHash2_ctx;

    OTPassword::zeroMemory(&vOutputHash1.at(0),
                           OTCryptoConfig::SymmetricKeySizeMax());
    OTPassword::zeroMemory(&vOutputHash2.at(0),
                           OTCryptoConfig::SymmetricKeySizeMax());
    OTPassword::zeroMemory(&vDigest.at(0),
                           OTCryptoConfig::SymmetricKeySizeMax());
    OTPassword::zeroMemory(&vDecrypted.at(0),
                           OTCryptoConfig::PublicKeysizeMax());

    // Here, we convert the EVP_PKEY that was passed in, to an RSA key for
    // signing.
    RSA* pRsaKey = EVP_PKEY_get1_RSA(const_cast<EVP_PKEY*>(pkey));

    if (!pRsaKey) {
        otErr << szFunc << ": EVP_PKEY_get1_RSA failed with error "
              << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        return false;
    }

    // Since the idea of this special code is that we're using 2 hash
    // algorithms,
    // let's look them up and see what they are.
    const EVP_MD* digest1 =
        OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::GetOpenSSLDigestByName(
            OTIdentifier::HashAlgorithm1); // SHA-256
    if (nullptr == digest1) {
        otErr << szFunc << ": Failure to load message digest algorithm.\n";
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }

    // hash the contents of the contract with HashAlgorithm1 (SHA-256)
    EVP_MD_CTX_init(&mdHash1_ctx);
    EVP_DigestInit(&mdHash1_ctx, digest1); // digest1 is the algorithm itself
    EVP_DigestUpdate(&mdHash1_ctx, strContractToVerify.Get(),
                     strContractToVerify.GetLength()); // input
    EVP_DigestFinal(&mdHash1_ctx, &vOutputHash1.at(0),
                    &uDigest1Len);    // output and size
    EVP_MD_CTX_cleanup(&mdHash1_ctx); // cleanup

    //#ifndef ANDROID   // NOT Android.
    const EVP_MD* digest2 =
        OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::GetOpenSSLDigestByName(
            OTIdentifier::HashAlgorithm2); // WHIRLPOOL
    if (nullptr == digest2) {
        otErr << szFunc << ": Failure to load message digest algorithm.\n";
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }

    // hash the same contents with HashAlgorithm2 (WHIRLPOOL)
    EVP_MD_CTX_init(&mdHash2_ctx);
    EVP_DigestInit(&mdHash2_ctx, digest2); // digest2 is the algorithm itself
    EVP_DigestUpdate(&mdHash2_ctx, strContractToVerify.Get(),
                     strContractToVerify.GetLength()); // Input
    EVP_DigestFinal(&mdHash2_ctx, &vOutputHash2.at(0),
                    &uDigest2Len);    // output and size
    EVP_MD_CTX_cleanup(&mdHash2_ctx); // cleanup

    // (Goes with the smaller size.)
    const uint32_t uDigestMergedLength =
        (uDigest1Len > uDigest2Len ? uDigest2Len : uDigest1Len);

    // XOR the two together
    for (uint32_t i = 0; i < uDigestMergedLength; i++) {
        vDigest.at(i) = ((vOutputHash1.at(i)) ^ (vOutputHash2.at(i)));
    }
    //#else // ** is ** ANDROID
    //
    //    // (Goes with the smaller size.)
    //    const uint32_t uDigestMergedLength = uDigest1Len;
    //
    //    for (int32_t i = 0; i < uDigest1Len; i++)
    //    {
    //        pDigest[i] = (pOutputHash1[i]);
    //    }
    //#endif // ANDROID

    // Now we have the exact content in pDigest that we should also see if we
    // decrypt
    // the signature that was passed in.
    //

    OTData binSignature;

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
        binSignature.GetSize()); // converting from unsigned to signed (since
                                 // openssl wants it that way.)

    if ((binSignature.GetSize() < static_cast<uint32_t>(RSA_size(pRsaKey))) ||
        (nSignatureSize < RSA_size(pRsaKey))) // this one probably unnecessary.
    {
        otErr << szFunc << ": Decoded base64-encoded data for signature, but "
                           "resulting size was < RSA_size(pRsaKey): "
                           "Signed: " << nSignatureSize
              << ". Unsigned: " << binSignature.GetSize() << ".\n";
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
        nSignatureSize, // length of signature, aka RSA_size(rsa)
        static_cast<const uint8_t*>(
            binSignature.GetPointer()), // location of signature
        &vDecrypted.at(0), // Output--must be large enough to hold the md (which
                           // is smaller than RSA_size(rsa) - 11)
        pRsaKey,           // signer's public key
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
    status = RSA_verify_PKCS1_PSS(pRsaKey, &vDigest.at(0), digest1,
                                  &vDecrypted.at(0),
                                  -2); // salt length recovered from signature

    if (status == 1) {
        otLog5 << "  *Signature verified*\n";
        bReturnValue = true;
    }
    else {
        otLog5 << szFunc << ": RSA_verify_PKCS1_PSS failed with error: "
               << ERR_error_string(ERR_get_error(), nullptr) << "\n";
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }

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

    return bReturnValue;
}

// All the other various versions eventually call this one, where the actual
// work is done.
bool OTCrypto_OpenSSL::OTCrypto_OpenSSLdp::SignContract(
    const OTString& strContractUnsigned, const EVP_PKEY* pkey,
    OTSignature& theSignature, const OTString& strHashType,
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

    // Moving this lower...

    //  _OTCont_SignCont1 theInstance(szFunc, md_ctx);

    //    OTString strDoubleHash;

    // Are we using the special SAMY hash? In which case, we have to actually
    // combine two signatures.
    const bool bUsesDefaultHashAlgorithm =
        strHashType.Compare(OTIdentifier::DefaultHashAlgorithm);
    EVP_MD* md = nullptr;

    // SAMY hash. (The "default" hash.)
    if (bUsesDefaultHashAlgorithm) {
        //        OTIdentifier hash1, hash2;
        //
        //        hash1.CalculateDigest(strContractUnsigned,
        // OTIdentifier::HashAlgorithm1);
        //        hash2.CalculateDigest(strContractUnsigned,
        // OTIdentifier::HashAlgorithm2);
        //
        //        hash1.XOR(hash2);
        //        hash1.GetString(strDoubleHash);
        //
        //        md = (EVP_MD
        // *)OTCrypto_OpenSSL::GetOpenSSLDigestByName(OTIdentifier::HashAlgorithm1);

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

    //    if (bUsesDefaultHashAlgorithm)
    //    {
    //        EVP_SignUpdate (&md_ctx, strDoubleHash.Get(),
    // strDoubleHash.GetLength());
    //    }
    //    else
    {
        EVP_SignUpdate(&md_ctx, strContractUnsigned.Get(),
                       strContractUnsigned.GetLength());
    }

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
        OTData tempData;
        tempData.Assign(sig_buf, sig_len);
        theSignature.SetData(tempData);

        return true;
    }
}

bool OTCrypto_OpenSSL::SignContract(const OTString& strContractUnsigned,
                                    const OTAsymmetricKey& theKey,
                                    OTSignature& theSignature, // output
                                    const OTString& strHashType,
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

bool OTCrypto_OpenSSL::VerifySignature(const OTString& strContractToVerify,
                                       const OTAsymmetricKey& theKey,
                                       const OTSignature& theSignature,
                                       const OTString& strHashType,
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
    const OTString& strContractToVerify, const EVP_PKEY* pkey,
    const OTSignature& theSignature, const OTString& strHashType,
    const OTPasswordData* pPWData) const
{
    OT_ASSERT_MSG(strContractToVerify.Exists(),
                  "OTCrypto_OpenSSL::VerifySignature: ASSERT FAILURE: "
                  "strContractToVerify.Exists()");
    OT_ASSERT_MSG(nullptr != pkey,
                  "Null pkey in OTCrypto_OpenSSL::VerifySignature.\n");

    const char* szFunc = "OTCrypto_OpenSSL::VerifySignature";

    // Are we using the special SAMY hash? In which case, we have to actually
    // combine two hashes.
    const bool bUsesDefaultHashAlgorithm =
        strHashType.Compare(OTIdentifier::DefaultHashAlgorithm);
    EVP_MD* md = nullptr;

    if (bUsesDefaultHashAlgorithm) {
        //        OTIdentifier hash1, hash2;
        //
        //        hash1.CalculateDigest(strContractToVerify,
        // OTIdentifier::HashAlgorithm1);
        //        hash2.CalculateDigest(strContractToVerify,
        // OTIdentifier::HashAlgorithm2);
        //
        //        hash1.XOR(hash2);
        //        hash1.GetString(strDoubleHash);
        //
        //        md = (EVP_MD
        // *)OTCrypto_OpenSSL::GetOpenSSLDigestByName(OTIdentifier::HashAlgorithm1);

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

    OTData binSignature;

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

    //    if (bUsesDefaultHashAlgorithm)
    //    {
    //        EVP_VerifyUpdate(&ctx, strDoubleHash.Get(),
    // strDoubleHash.GetLength());
    //    }
    //    else
    {
        EVP_VerifyUpdate(&ctx, strContractToVerify.Get(),
                         strContractToVerify.GetLength());
    }

    // Now we pass in the Signature
    // EVP_VerifyFinal() returns 1 for a correct signature,
    // 0 for failure and -1 if some other error occurred.
    //
    int32_t nErr = EVP_VerifyFinal(
        &ctx, static_cast<const uint8_t*>(binSignature.GetPointer()),
        static_cast<uint32_t>(binSignature.GetSize()),
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
bool OTCrypto_OpenSSL::SignContract(const OTString& strContractUnsigned,
                                    const OTString& strSigHashType,
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
bool OTCrypto_OpenSSL::VerifySignature(const OTString& strContractToVerify,
                                       const OTString& strSigHashType,
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

} // namespace opentxs
