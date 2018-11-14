// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_USING_OPENSSL
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/CryptoSymmetricDecryptOutput.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/stacktrace.h"
#include "opentxs/core/util/Timer.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/crypto/key/RSA.hpp"
#endif
#include "opentxs/crypto/library/OpenSSL.hpp"
#include "opentxs/OT.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "crypto/key/RSA.hpp"
#include "crypto/key/RSA_private.hpp"
#endif
#include "AsymmetricProvider.hpp"
#include "OpenSSL_BIO.hpp"

extern "C" {
#include <netinet/in.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/conf.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/opensslconf.h>
#include <openssl/opensslv.h>
#include <openssl/ossl_typ.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>
#include <openssl/ui.h>
}

#include <cstdlib>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <ostream>
#include <set>
#include <string>
#include <thread>

#include "OpenSSL.hpp"

#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define OT_METHOD "opentxs::OpenSSL::"

#if OPENSSL_VERSION_NUMBER < 0x10100000L
extern "C" {
EVP_CIPHER_CTX* EVP_CIPHER_CTX_new() { return new EVP_CIPHER_CTX; }

EVP_MD_CTX* EVP_MD_CTX_new() { return new EVP_MD_CTX; }

void EVP_CIPHER_CTX_free(EVP_CIPHER_CTX* context) { delete context; }

void EVP_MD_CTX_free(EVP_MD_CTX* context) { delete context; }
}
#endif

namespace opentxs
{
crypto::OpenSSL* Factory::OpenSSL(const api::Crypto& crypto)
{
    return new crypto::implementation::OpenSSL(crypto);
}
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
std::mutex* OpenSSL::s_arrayMutex = nullptr;

class OpenSSL::OpenSSLdp
{
public:
    // These are protected because they contain OpenSSL-specific parameters.

    static const EVP_MD* HashTypeToOpenSSLType(const proto::HashType hashType);
    static const EVP_CIPHER* CipherModeToOpenSSLMode(
        const LegacySymmetricProvider::Mode cipher);

#if OT_CRYPTO_SUPPORTED_KEY_RSA
    bool SignContractDefaultHash(
        const Data& strContractUnsigned,
        const EVP_PKEY* pkey,
        Data& theSignature,  // output
        const OTPasswordData* pPWData = nullptr) const;
    bool VerifyContractDefaultHash(
        const Data& strContractToVerify,
        const EVP_PKEY* pkey,
        const Data& theSignature,
        const OTPasswordData* pPWData = nullptr) const;
    // Sign or verify using the actual OpenSSL EVP_PKEY
    bool SignContract(
        const Data& strContractUnsigned,
        const EVP_PKEY* pkey,
        Data& theSignature,  // output
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const;
    bool VerifySignature(
        const Data& strContractToVerify,
        const EVP_PKEY* pkey,
        const Data& theSignature,
        const proto::HashType hashType,
        const OTPasswordData* pPWData = nullptr) const;
#endif

    OpenSSLdp(
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        const api::Crypto& crypto
#endif
        )
#if OT_CRYPTO_SUPPORTED_KEY_RSA
        : crypto_(crypto)
#endif
    {
    }

private:
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    const api::Crypto& crypto_;

    OpenSSLdp() = delete;
#endif

    OpenSSLdp(OpenSSLdp&) = delete;
    OpenSSLdp(const OpenSSLdp&&) = delete;
    OpenSSLdp& operator=(OpenSSLdp&) = delete;
    OpenSSLdp& operator=(const OpenSSLdp&&) = delete;
};

OpenSSL::OpenSSL(const api::Crypto& crypto)
    :
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    AsymmetricProvider()
    ,
#endif
    crypto_(crypto)
    , dp_(new OpenSSLdp(
#if OT_CRYPTO_SUPPORTED_KEY_RSA
          crypto_
#endif
          ))
    , lock_()
{
}

OpenSSL::CipherContext::CipherContext()
    : context_(EVP_CIPHER_CTX_new())
{
    OT_ASSERT(nullptr != context_);
}

OpenSSL::CipherContext::~CipherContext()
{
    if (nullptr != context_) {
        EVP_CIPHER_CTX_free(context_);
        context_ = nullptr;
    }
}

OpenSSL::CipherContext::operator EVP_CIPHER_CTX*() { return context_; }

OpenSSL::DigestContext::DigestContext()
    : context_(EVP_MD_CTX_new())
{
    OT_ASSERT(nullptr != context_);
}

OpenSSL::DigestContext::~DigestContext()
{
    if (nullptr != context_) { EVP_MD_CTX_free(context_); }
}

OpenSSL::DigestContext::operator EVP_MD_CTX*() { return context_; }

extern "C" {
#if OPENSSL_VERSION_NUMBER - 0 < 0x10000000L
unsigned long ot_openssl_thread_id(void);
#else
void ot_openssl_thread_id(CRYPTO_THREADID*);
#endif

void ot_openssl_locking_callback(
    std::int32_t mode,
    std::int32_t type,
    const char* file,
    std::int32_t line);
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
    std::uint64_t ret =
        std::hash<std::thread::id>()(std::this_thread::get_id());

    return (ret);
}

#else
void ot_openssl_thread_id(CRYPTO_THREADID* id)
{
    OT_ASSERT(nullptr != id);

    // TODO: Possibly do this by pointer instead of by std::uint64_t,
    // for certain platforms. (OpenSSL provides functions for both.)
    //

    [[maybe_unused]] unsigned long val =
        std::hash<std::thread::id>()(std::this_thread::get_id());

    //    void CRYPTO_THREADID_set_numeric(CRYPTO_THREADID* id, std::uint64_t
    //    val);
    //    void CRYPTO_THREADID_set_pointer(CRYPTO_THREADID* id, void* ptr);

    CRYPTO_THREADID_set_numeric(id, val);
}
#endif

/*
 locking_function(std::int32_t mode, std::int32_t n, const char* file,
 std::int32_t line) is needed to perform locking on shared data structures.
 (Note that OpenSSL uses a number of global data structures that will be
 implicitly shared whenever multiple threads use OpenSSL.) Multi-threaded
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
void ot_openssl_locking_callback(
    std::int32_t mode,
    std::int32_t type,
    const char*,
    std::int32_t)
{
    if (mode & CRYPTO_LOCK) {
        OpenSSL::s_arrayMutex[type].lock();
    } else {
        OpenSSL::s_arrayMutex[type].unlock();
    }
}
}  // extern "C"

OTPassword* OpenSSL::DeriveNewKey(
    const OTPassword& userPassword,
    const Data& dataSalt,
    std::uint32_t uIterations,
    Data& dataCheckHash) const
{
    OT_ASSERT(!dataSalt.empty());

    LogVerbose(OT_METHOD)(__FUNCTION__)(
        ": Using a text passphrase, salt, and iteration count, ")(
        "to make a derived key... ")
        .Flush();

    std::unique_ptr<OTPassword> pDerivedKey(InstantiateBinarySecret());

    OT_ASSERT(pDerivedKey);

    // Key derivation in OpenSSL.
    //
    // std::int32_t PKCS5_PBKDF2_HMAC_SHA1(const char*, std::int32_t, const
    // std::uint8_t*,
    // std::int32_t, std::int32_t, std::int32_t, std::uint8_t*)
    //
    PKCS5_PBKDF2_HMAC_SHA1(
        reinterpret_cast<const char*>  // If is password... supply password,
                                       // otherwise supply memory.
        (userPassword.isPassword() ? userPassword.getPassword_uint8()
                                   : userPassword.getMemory_uint8()),
        static_cast<std::int32_t>(
            userPassword.isPassword() ? userPassword.getPasswordSize()
                                      : userPassword.getMemorySize()),
        static_cast<const std::uint8_t*>(dataSalt.data()),
        static_cast<std::int32_t>(dataSalt.size()),
        static_cast<std::int32_t>(uIterations),
        static_cast<std::int32_t>(pDerivedKey->getMemorySize()),
        static_cast<std::uint8_t*>(pDerivedKey->getMemoryWritable()));

    // For The HashCheck
    const bool bHaveCheckHash = !dataCheckHash.empty();

    auto tmpHashCheck = Data::Factory();
    tmpHashCheck->SetSize(crypto_.Config().SymmetricKeySize());

    // We take the DerivedKey, and hash it again, then get a 'hash-check'
    // Compare that with the supplied one, (if there is one).
    // If there isn't one, we return the

    PKCS5_PBKDF2_HMAC_SHA1(
        reinterpret_cast<const char*>(pDerivedKey->getMemory()),
        static_cast<std::int32_t>(pDerivedKey->getMemorySize()),
        static_cast<const std::uint8_t*>(dataSalt.data()),
        static_cast<std::int32_t>(dataSalt.size()),
        static_cast<std::int32_t>(uIterations),
        static_cast<std::int32_t>(tmpHashCheck->size()),
        const_cast<std::uint8_t*>(
            static_cast<const std::uint8_t*>(tmpHashCheck->data())));

    if (bHaveCheckHash) {
        auto strDataCheck = String::Factory(), strTestCheck = String::Factory();
        strDataCheck->Set(
            static_cast<const char*>(dataCheckHash.data()),
            dataCheckHash.size());
        strTestCheck->Set(
            static_cast<const char*>(tmpHashCheck->data()),
            tmpHashCheck->size());

        if (!strDataCheck->Compare(strTestCheck)) {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Incorrect password provided. "
                                               "Provided check hash: ")(
                strDataCheck)(" Calculated check hash: ")(strTestCheck)
                .Flush();

            pDerivedKey.reset();
        }
    }

    dataCheckHash.Assign(tmpHashCheck->data(), tmpHashCheck->size());

    return pDerivedKey.release();
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

const EVP_MD* OpenSSL::OpenSSLdp::HashTypeToOpenSSLType(
    const proto::HashType hashType)
{
    const EVP_MD* OpenSSLType;

    switch (hashType) {
        case proto::HASHTYPE_SHA256:
            OpenSSLType = EVP_sha256();
            break;
        case proto::HASHTYPE_SHA512:
            OpenSSLType = EVP_sha512();
            break;
        default:
            OpenSSLType = nullptr;
    }
    return OpenSSLType;
}

const EVP_CIPHER* OpenSSL::OpenSSLdp::CipherModeToOpenSSLMode(
    const LegacySymmetricProvider::Mode cipher)
{
    const EVP_CIPHER* OpenSSLCipher;

    switch (cipher) {
        case LegacySymmetricProvider::AES_128_CBC:
            OpenSSLCipher = EVP_aes_128_cbc();
            break;
        case LegacySymmetricProvider::AES_256_CBC:
            OpenSSLCipher = EVP_aes_256_cbc();
            break;
        case LegacySymmetricProvider::AES_256_ECB:
            OpenSSLCipher = EVP_aes_256_ecb();
            break;
        case LegacySymmetricProvider::AES_128_GCM:
            OpenSSLCipher = EVP_aes_128_gcm();
            break;
        case LegacySymmetricProvider::AES_256_GCM:
            OpenSSLCipher = EVP_aes_256_gcm();
            break;
        default:
            OpenSSLCipher = nullptr;
    }
    return OpenSSLCipher;
}

/*
 SHA256_CTX context;
 std::uint8_t md[SHA256_DIGEST_LENGTH];

 SHA256_Init(&context);
 SHA256_Update(&context, (std::uint8_t*)input, length);
 SHA256_Final(md, &context);
 */

// (To instantiate a text secret, just do this:  OTPassword thePassword;)

// Caller MUST delete!
// todo return a smartpointer here.
OTPassword* OpenSSL::InstantiateBinarySecret() const
{
    std::uint8_t* tmp_data = new uint8_t[crypto_.Config().SymmetricKeySize()];
    OTPassword* pNewKey = new OTPassword(
        static_cast<void*>(&tmp_data[0]), crypto_.Config().SymmetricKeySize());
    OT_ASSERT_MSG(nullptr != pNewKey, "pNewKey = new OTPassword");

    if (nullptr != tmp_data) {
        delete[] tmp_data;
        tmp_data = nullptr;
    }

    return pNewKey;
}

BinarySecret OpenSSL::InstantiateBinarySecretSP() const
{
    BinarySecret binarySecret;
    binarySecret.reset(InstantiateBinarySecret());
    return binarySecret;
}

void OpenSSL::thread_setup() const
{
    OpenSSL::s_arrayMutex = new std::mutex[CRYPTO_num_locks()];

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
    std::int32_t nResult = CRYPTO_THREADID_set_callback(ot_openssl_thread_id);
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

void OpenSSL::thread_cleanup() const
{
    CRYPTO_set_locking_callback(nullptr);

    if (nullptr != OpenSSL::s_arrayMutex) { delete[] OpenSSL::s_arrayMutex; }

    OpenSSL::s_arrayMutex = nullptr;
}

void OpenSSL::Init()
{
    LogDetail(OT_METHOD)(__FUNCTION__)(
        ": Setting up OpenSSL: SSL_library_init, error "
        "strings and algorithms, and OpenSSL config...")
        .Flush();

    static bool Initialized = false;

    if (Initialized) { return; }

    Initialized = true;

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
    CRYPTO_malloc_init();  //      # -1
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
    SSL_library_init();  //     #0

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
     Then, on
     http://www.openssl.org/docs/crypto/ERR_load_crypto_.Config().strings.html#,
     in
     reference to SSL_load_error_strings and
     ERR_load_crypto_.Config().strings, it says:

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
     - ERR_load_crypto_.Config().strings() registers the error
     strings for all libcrypto functions.
     - SSL_load_error_strings() does the same, but also registers the libssl
     error strings.
     One of these functions should be called before generating textual error
     messages.
     - ERR_free_strings() frees all previously loaded error strings.
     */

    SSL_load_error_strings();  // DONE -- corresponds to ERR_free_strings in
                               // OT_Cleanup()   #1

    //  ERR_load_crypto_.Config().strings();   // Redundant --
    //  SSL_load_error_strings does
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

    OpenSSL_add_all_algorithms();  // DONE -- corresponds to EVP_cleanup() in
                                   // OT_Cleanup().    #2
    OpenSSL_add_all_digests();
    //
    //
    // RAND
    //
    /*
     RAND_bytes() automatically calls RAND_poll() if it has not already been
     done at least once. So you do not have to call it yourself. RAND_poll()
     feeds on what the operating system provides: on Linux, Solaris, FreeBSD and
     similar Unix-like systems, it will use /dev/urandom (or /dev/random if
     there is no /dev/urandom) to obtain a cryptographically secure initial
     seed; on Windows, it will call CryptGenRandom() for the same effect.

     RAND_screen() is provided by OpenSSL only for backward compatibility with
     (much) older code which
     may call it (that was before OpenSSL used proper OS-based seed
     initialization).

     So the "normal" way of dealing with RAND_poll() and RAND_screen() is to
     call neither. Just use RAND_bytes() and be happy.

     RESPONSE: Thanks for the detailed answer. In regards to your suggestion to
     call neither, the problem under Windows is that RAND_poll can take some
     time and will block our UI. So we call it upon initialization, which works
     for us.
     */
    // I guess Windows will seed the PRNG whenever someone tries to get
    // some RAND_bytes() the first time...
    //
    //#ifdef _WIN32
    // CORRESPONDS to RAND_cleanup in OT_Cleanup().
    //      RAND_screen();
    //#else
    // note: optimization: might want to remove this, since supposedly it
    // happens anyway when you use RAND_bytes. So the "lazy evaluation" rule
    // would seem to imply, not bothering to slow things down NOW, since it's
    // not really needed until THEN.
    //

#if defined(USE_RAND_POLL)

    RAND_poll();  //                                   #3

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

    OPENSSL_config(nullptr);  // const char *config_name = nullptr: the default
                              // name openssl_conf will be used.

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

    LogDetail(OT_METHOD)(__FUNCTION__)(
        ": OpenSSL WAS compiled with thread support, FYI. "
        "Setting up mutexes...")
        .Flush();

    this->thread_setup();

#else
    // no thread support

    LogOutput(OT_METHOD)(__FUNCTION__)(
        ": WARNING: OpenSSL was NOT compiled with thread support. "
        "(Also: Master Key will not expire).")
        .Flush();

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

void OpenSSL::Cleanup()
{
    LogTrace(OT_METHOD)(__FUNCTION__)(": Cleaning up OpenSSL...").Flush();

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
    CONF_modules_free();  // CORRESPONDS to: OPENSSL_config() in OT_Init().   #4

    RAND_cleanup();  // Corresponds to RAND_screen / RAND_poll in OT_Init()  #3

    EVP_cleanup();  // DONE (brutal) -- corresponds to
                    // OpenSSL_add_all_algorithms
                    // in OT_Init(). #2

    CRYPTO_cleanup_all_ex_data();  // (brutal)
                                   //    CRYPTO_mem_leaks(bio_err);

    ERR_free_strings();  // DONE (brutal) -- corresponds to
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

// #define crypto_.Config().SymmetricBufferSize()   default: 4096

bool OpenSSL::ArgumentCheck(
    const bool encrypt,
    const LegacySymmetricProvider::Mode cipher,
    const OTPassword& key,
    const Data& iv,
    const Data& tag,
    const char* input,
    const std::uint32_t inputLength,
    bool& AEAD,
    bool& ECB) const
{
    AEAD =
        ((LegacySymmetricProvider::AES_128_GCM == cipher) ||
         (LegacySymmetricProvider::AES_256_GCM == cipher));
    ECB = (LegacySymmetricProvider::AES_256_ECB == cipher);

    // Debug logging
    LogDebug(OT_METHOD)(__FUNCTION__)(": Using cipher: ")(
        LegacySymmetricProvider::ModeToString(cipher))
        .Flush();

    if (ECB) { LogDebug("...in ECB mode.").Flush(); }

    if (AEAD) { LogDebug("...in AEAD mode.").Flush(); }

    LogDebug(OT_METHOD)(__FUNCTION__)(":...with a ")(
        8 * LegacySymmetricProvider::KeySize(cipher))("bit key.")
        .Flush();

    LogDebug(OT_METHOD)(__FUNCTION__)(": Actual key bytes: ")(
        key.getMemorySize())
        .Flush();
    LogDebug(OT_METHOD)(__FUNCTION__)("Actual IV bytes: ")(iv.size()).Flush();
    if ((!encrypt) & AEAD) {
        LogDebug(OT_METHOD)(__FUNCTION__)(": Actual tag bytes: ")(tag.size())
            .Flush();
    }

    // Validate input parameters
    if (!encrypt) {
        if (AEAD && (LegacySymmetricProvider::TagSize(cipher) != tag.size())) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect tag size.").Flush();
            return false;
        }
    }

    if ((encrypt && ECB) &&
        (inputLength != LegacySymmetricProvider::KeySize(cipher))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Input size must be exactly one block for ECB mode.")
            .Flush();
        return false;
    }

    if (!ECB && (iv.size() != LegacySymmetricProvider::IVSize(cipher))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect IV size.").Flush();
        LogOutput(OT_METHOD)(__FUNCTION__)(": Actual IV bytes: ")(iv.size())
        (".").Flush();
        return false;
    }

    if (key.getMemorySize() != LegacySymmetricProvider::KeySize(cipher)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key size. Expected: ")(
            LegacySymmetricProvider::KeySize(cipher))(".")
            .Flush();
        LogOutput(OT_METHOD)(__FUNCTION__)(": Actual key bytes: ")(
            key.getMemorySize())(".")
            .Flush();
        return false;
    }

    if (nullptr == input) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Input pointer does not exist.")
            .Flush();
        return false;
    }

    if (0 == inputLength) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Input is empty.").Flush();
        return false;
    }

    return true;
}

bool OpenSSL::Encrypt(
    const OTPassword& theRawSymmetricKey,  // The symmetric key, in clear form.
    const char* szInput,                   // This is the Plaintext.
    const std::uint32_t lInputLength,
    const Data& theIV,               // (We assume this IV
                                     // is already generated
                                     // and passed in.)
    Data& theEncryptedOutput) const  // OUTPUT. (Ciphertext.)
{
    return Encrypt(
        LegacySymmetricProvider::AES_256_CBC,  // What OT was using before
        theRawSymmetricKey,
        theIV,
        szInput,
        lInputLength,
        theEncryptedOutput);
}

bool OpenSSL::Encrypt(
    const LegacySymmetricProvider::Mode cipher,
    const OTPassword& key,
    const char* plaintext,
    std::uint32_t plaintextLength,
    Data& ciphertext) const
{
    auto unusedIV = Data::Factory();

    return Encrypt(
        cipher, key, unusedIV, plaintext, plaintextLength, ciphertext);
}

bool OpenSSL::Encrypt(
    const LegacySymmetricProvider::Mode cipher,
    const OTPassword& key,
    const Data& iv,
    const char* plaintext,
    std::uint32_t plaintextLength,
    Data& ciphertext) const
{
    auto unusedTag = Data::Factory();

    return Encrypt(
        cipher, key, iv, plaintext, plaintextLength, ciphertext, unusedTag);
}

bool OpenSSL::Encrypt(
    const LegacySymmetricProvider::Mode cipher,
    const OTPassword& key,
    const Data& iv,
    const char* plaintext,
    std::uint32_t plaintextLength,
    Data& ciphertext,
    Data& tag) const
{
    bool AEAD = false;
    bool ECB = false;
    bool goodInputs = ArgumentCheck(
        true, cipher, key, iv, tag, plaintext, plaintextLength, AEAD, ECB);

    if (!goodInputs) { return false; }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX* pCONTEXT = &ctx;
#else
    CipherContext context;
    EVP_CIPHER_CTX* pCONTEXT = (EVP_CIPHER_CTX*)(context);
#endif

    std::vector<std::uint8_t> vBuffer(
        crypto_.Config().SymmetricBufferSize());  // 4096
    std::vector<std::uint8_t> vBuffer_out(
        crypto_.Config().SymmetricBufferSize() + EVP_MAX_IV_LENGTH);
    std::int32_t len_out = 0;

    memset(&vBuffer.at(0), 0, crypto_.Config().SymmetricBufferSize());
    memset(
        &vBuffer_out.at(0),
        0,
        crypto_.Config().SymmetricBufferSize() + EVP_MAX_IV_LENGTH);

    //
    // This is where the envelope final contents will be placed.
    // including the size of the IV, the IV itself, and the ciphertext.
    //
    ciphertext.Release();

#if OPENSSL_VERSION_NUMBER < 0x10100000L
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
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failure in EVP_CIPHER_CTX_cleanup. (It "
                    "returned 0).")
                    .Flush();

            m_szFunc = nullptr;  // keep the static analyzer happy
        }
    };
    _OTEnv_Enc_stat theInstance(szFunc, ctx);
#endif

    Lock lock(lock_);
    const EVP_CIPHER* cipher_type = dp_->CipherModeToOpenSSLMode(cipher);
    lock.unlock();

    OT_ASSERT(nullptr != cipher_type);

    //  int EVP_EncryptInit_ex(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *type,
    //                         ENGINE *impl, unsigned char *key, unsigned char
    //                         *iv);

    /*
    EVP_EncryptInit_ex() sets up cipher context ctx for encryption with cipher
    type from ENGINE impl. ctx must be initialized before calling this function.
     type is normally supplied by a function such as EVP_des_cbc().

     If impl is NULL then the default implementation is used.
     key is the symmetric key to use and iv is the IV to use (if necessary),
     the actual number of bytes used for the key and IV depends on the cipher.

     It is possible to set all parameters to NULL except type in an initial call
     and supply the remaining parameters in subsequent calls, all of which have
     type set to NULL. This is done when the default cipher parameters are not \
     appropriate.

     EVP_EncryptInit_ex(), EVP_EncryptUpdate() and EVP_EncryptFinal_ex()
     return 1 for success and 0 for failure.
     */
    if (!EVP_EncryptInit_ex(pCONTEXT, cipher_type, nullptr, nullptr, nullptr)) {
        //  if (!EVP_EncryptInit_ex(context, cipher_type, nullptr, nullptr,
        //  nullptr))
        LogOutput(OT_METHOD)(__FUNCTION__)(": Could not set cipher type.")
            .Flush();
        return false;
    }

    if (AEAD) {
        // set GCM IV length
        if (!EVP_CIPHER_CTX_ctrl(
                pCONTEXT, EVP_CTRL_GCM_SET_IVLEN, iv.size(), nullptr)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Could not set IV length.")
                .Flush();
            return false;
        }
    }

    if (!ECB) {
        // set IV
        if (!EVP_EncryptInit_ex(
                pCONTEXT,
                nullptr,
                nullptr,
                nullptr,
                static_cast<std::uint8_t*>(const_cast<void*>(iv.data())))) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Could not set IV.").Flush();
            return false;
        }
    }

    // set key
    if (!EVP_EncryptInit_ex(
            pCONTEXT,
            nullptr,
            nullptr,
            const_cast<std::uint8_t*>(key.getMemory_uint8()),
            nullptr)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Could not set key.").Flush();
        return false;
    }

    // TODO: set AAD

    // Now we process the input and write the encrypted data to
    // the output.
    //
    std::uint32_t lRemainingLength = plaintextLength;
    std::uint32_t lCurrentIndex = 0;

    while (lRemainingLength > 0) {
        // If the remaining length is less than the default buffer size, then
        // set len to remaining length.
        // else if remaining length is larger than or equal to default buffer
        // size, then use the default buffer size.
        // Resulting value stored in len.
        //

        std::uint32_t len =
            (lRemainingLength < crypto_.Config().SymmetricBufferSize())
                ? lRemainingLength
                : crypto_.Config().SymmetricBufferSize();  // 4096

        if (!EVP_EncryptUpdate(
                pCONTEXT,
                &vBuffer_out.at(0),
                &len_out,
                const_cast<std::uint8_t*>(reinterpret_cast<const uint8_t*>(
                    &(plaintext[lCurrentIndex]))),
                len)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": EVP_EncryptUpdate: failed.")
                .Flush();
            return false;
        }
        lRemainingLength -= len;
        lCurrentIndex += len;

        if (len_out > 0)
            ciphertext.Concatenate(
                reinterpret_cast<void*>(&vBuffer_out.at(0)),
                static_cast<std::uint32_t>(len_out));
    }

    if (!EVP_EncryptFinal_ex(pCONTEXT, &vBuffer_out.at(0), &len_out)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": EVP_EncryptFinal: failed.")
            .Flush();
        return false;
    }

    if (AEAD) {
        /* Get the tag */
        tag.SetSize(LegacySymmetricProvider::TagSize(cipher));
        tag.zeroMemory();

        if (!EVP_CIPHER_CTX_ctrl(
                pCONTEXT,
                EVP_CTRL_GCM_GET_TAG,
                LegacySymmetricProvider::TagSize(cipher),
                const_cast<void*>(tag.data()))) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Could not extract tag.")
                .Flush();
            return false;
        }
    }

    // This is the "final" piece that is added from EncryptFinal just above.
    //
    if (len_out > 0) {
        ciphertext.Concatenate(
            reinterpret_cast<void*>(&vBuffer_out.at(0)),
            static_cast<std::uint32_t>(len_out));
    }

    return true;
}

bool OpenSSL::Decrypt(
    const OTPassword& theRawSymmetricKey,  // The symmetric key, in clear form.
    const char* szInput,                   // This is the Ciphertext.
    const std::uint32_t lInputLength,
    const Data& theIV,  // (We assume this IV
                        // is already generated
                        // and passed in.)
    CryptoSymmetricDecryptOutput& theDecryptedOutput) const  // OUTPUT.
                                                             // (Recovered
// plaintext.) You can
// pass OTPassword& OR
// Data& here (either
// will work.)
{
    return Decrypt(
        LegacySymmetricProvider::AES_256_CBC,  // What OT was using before
        theRawSymmetricKey,
        theIV,
        szInput,
        lInputLength,
        theDecryptedOutput);
}

bool OpenSSL::Decrypt(
    const LegacySymmetricProvider::Mode cipher,
    const OTPassword& key,
    const char* ciphertext,
    std::uint32_t ciphertextLength,
    CryptoSymmetricDecryptOutput& plaintext) const
{
    auto unusedIV = Data::Factory();

    return Decrypt(
        cipher, key, unusedIV, ciphertext, ciphertextLength, plaintext);
}

bool OpenSSL::Decrypt(
    const LegacySymmetricProvider::Mode cipher,
    const OTPassword& key,
    const Data& iv,
    const char* ciphertext,
    const std::uint32_t ciphertextLength,
    CryptoSymmetricDecryptOutput& plaintext) const
{
    auto unusedTag = Data::Factory();

    return Decrypt(
        cipher, key, iv, unusedTag, ciphertext, ciphertextLength, plaintext);
}

bool OpenSSL::Decrypt(
    const LegacySymmetricProvider::Mode cipher,
    const OTPassword& key,
    const Data& iv,
    const Data& tag,
    const char* ciphertext,
    const std::uint32_t ciphertextLength,
    CryptoSymmetricDecryptOutput& plaintext) const
{
    bool AEAD = false;
    bool ECB = false;
    bool goodInputs = ArgumentCheck(
        false, cipher, key, iv, tag, ciphertext, ciphertextLength, AEAD, ECB);

    if (!goodInputs) { return false; }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_CIPHER_CTX ctx;
#else
    CipherContext context;
#endif
    std::vector<std::uint8_t> vBuffer(
        crypto_.Config().SymmetricBufferSize());  // 4096
    std::vector<std::uint8_t> vBuffer_out(
        crypto_.Config().SymmetricBufferSize() + EVP_MAX_IV_LENGTH);
    std::int32_t len_out = 0;

    memset(&vBuffer.at(0), 0, crypto_.Config().SymmetricBufferSize());
    memset(
        &vBuffer_out.at(0),
        0,
        crypto_.Config().SymmetricBufferSize() + EVP_MAX_IV_LENGTH);

    //
    // This is where the plaintext results will be placed.
    //
    plaintext.Release();

#if OPENSSL_VERSION_NUMBER < 0x10100000L
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
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failure in EVP_CIPHER_CTX_cleanup. (It "
                    "returned 0).")
                    .Flush();
            m_szFunc = nullptr;  // to keep the static analyzer happy.
        }
    };
    _OTEnv_Dec_stat theInstance(szFunc, ctx);
#endif

    Lock lock(lock_);
    const EVP_CIPHER* cipher_type = dp_->CipherModeToOpenSSLMode(cipher);
    lock.unlock();

    OT_ASSERT(nullptr != cipher_type);

// set algorith,
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (!EVP_DecryptInit_ex(&ctx, cipher_type, nullptr, nullptr, nullptr)) {
#else
    if (!EVP_DecryptInit_ex(context, cipher_type, nullptr, nullptr, nullptr)) {
#endif
        LogOutput(OT_METHOD)(__FUNCTION__)(": Could not set cipher type.")
            .Flush();
        return false;
    }

    if (AEAD) {
        // set GCM IV length
        if (!EVP_CIPHER_CTX_ctrl(
#if OPENSSL_VERSION_NUMBER < 0x10100000L
                &ctx, EVP_CTRL_GCM_SET_IVLEN, iv.GetSize(), nullptr)) {
#else
                context, EVP_CTRL_GCM_SET_IVLEN, iv.size(), nullptr)) {
#endif
            LogOutput(OT_METHOD)(__FUNCTION__)(": Could not set IV length.")
                .Flush();
            return false;
        }
    }  // namespace opentxs

    if (!ECB) {
        // set IV
        if (!EVP_DecryptInit_ex(
#if OPENSSL_VERSION_NUMBER < 0x10100000L
                &ctx,
#else
                context,
#endif
                nullptr,
                nullptr,
                nullptr,
                static_cast<std::uint8_t*>(const_cast<void*>(iv.data())))) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Could not set IV.").Flush();
            return false;
        }
    }

    // set key
    if (!EVP_DecryptInit_ex(
#if OPENSSL_VERSION_NUMBER < 0x10100000L
            &ctx,
#else
            context,
#endif
            nullptr,
            nullptr,
            const_cast<std::uint8_t*>(key.getMemory_uint8()),
            nullptr)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Could not set key.").Flush();
        return false;
    }
    // TODO: set AAD

    // Now we process the input and write the decrypted data to
    // the output.
    //
    std::uint32_t lRemainingLength = ciphertextLength;
    std::uint32_t lCurrentIndex = 0;

    while (lRemainingLength > 0) {
        // If the remaining length is less than the default buffer size, then
        // set len to remaining length.
        // else if remaining length is larger than or equal to default buffer
        // size, then use the default buffer size.
        // Resulting value stored in len.
        //
        std::uint32_t len =
            (lRemainingLength < crypto_.Config().SymmetricBufferSize())
                ? lRemainingLength
                : crypto_.Config().SymmetricBufferSize();  // 4096
        lRemainingLength -= len;

        if (!EVP_DecryptUpdate(
#if OPENSSL_VERSION_NUMBER < 0x10100000L
                &ctx,
#else
                context,
#endif
                &vBuffer_out.at(0),
                &len_out,
                const_cast<std::uint8_t*>(reinterpret_cast<const uint8_t*>(
                    &(ciphertext[lCurrentIndex]))),
                len)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": EVP_DecryptUpdate: Failed.")
                .Flush();
            return false;
        }
        lCurrentIndex += len;

        if (len_out > 0)
            if (false == plaintext.Concatenate(
                             reinterpret_cast<void*>(&vBuffer_out.at(0)),
                             static_cast<std::uint32_t>(len_out))) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failure: theDecryptedOutput isn't large "
                    "enough for the decrypted output (1).")
                    .Flush();
                return false;
            }
    }

    if (AEAD) {
        // Load AEAD verification tag
        if (!EVP_CIPHER_CTX_ctrl(
#if OPENSSL_VERSION_NUMBER < 0x10100000L
                &ctx,
#else
                context,
#endif
                EVP_CTRL_GCM_SET_TAG,
                LegacySymmetricProvider::TagSize(cipher),
                const_cast<void*>(tag.data()))) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Could not set tag.").Flush();
            return false;
        }
    }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (!EVP_DecryptFinal_ex(&ctx, &vBuffer_out.at(0), &len_out)) {
#else
    if (!EVP_DecryptFinal_ex(context, &vBuffer_out.at(0), &len_out)) {
#endif
        LogOutput(OT_METHOD)(__FUNCTION__)(": EVP_DecryptFinal: Failed.")
            .Flush();
        return false;
    }

    // This is the "final" piece that is added from DecryptFinal just above.
    //
    if (len_out > 0)
        if (false == plaintext.Concatenate(
                         reinterpret_cast<void*>(&vBuffer_out.at(0)),
                         static_cast<std::uint32_t>(len_out))) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failure: theDecryptedOutput isn't large "
                "enough for the decrypted output (2).")
                .Flush();
            return false;
        }

    return true;
}

/*
#include <openssl/evp.h>
std::int32_t EVP_OpenInit(EVP_CIPHER_CTX* ctx, EVP_CIPHER* type, std::uint8_t*
ek,
                 std::int32_t ekl, std::uint8_t* iv, EVP_PKEY* priv);
std::int32_t EVP_OpenUpdate(EVP_CIPHER_CTX* ctx, std::uint8_t* out,
std::int32_t* outl, std::uint8_t* in, std::int32_t inl); std::int32_t
EVP_OpenFinal(EVP_CIPHER_CTX* ctx, std::uint8_t* out, std::int32_t* outl);
DESCRIPTION

The EVP envelope routines are a high level interface to envelope decryption.
They decrypt a public key
 encrypted symmetric key and then decrypt data using it.

 std::int32_t EVP_OpenInit(EVP_CIPHER_CTX* ctx, EVP_CIPHER* type, std::uint8_t*
ek,
std::int32_t
ekl, std::uint8_t* iv, EVP_PKEY* priv);
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

bool OpenSSL::Digest(
    const proto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    std::uint8_t* output) const

{
    const auto size = HashingProvider::HashSize(hashType);

    if ((proto::HASHTYPE_ERROR == hashType) ||
        (proto::HASHTYPE_NONE == hashType) ||
        (proto::HASHTYPE_BLAKE2B160 == hashType) ||
        (proto::HASHTYPE_BLAKE2B256 == hashType) ||
        (proto::HASHTYPE_BLAKE2B512 == hashType)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: invalid hash type: ")(
            HashingProvider::HashTypeToString(hashType))(".")
            .Flush();

        return false;
    }

    EVP_MD_CTX* context = EVP_MD_CTX_create();
    Lock lock(lock_);
    const EVP_MD* algorithm = dp_->HashTypeToOpenSSLType(hashType);
    lock.unlock();
    unsigned int hash_length = 0;

    if (nullptr != algorithm) {
        EVP_DigestInit_ex(context, algorithm, NULL);
        EVP_DigestUpdate(context, input, inputSize);
        EVP_DigestFinal_ex(context, output, &hash_length);
        EVP_MD_CTX_destroy(context);

        OT_ASSERT(size == hash_length);

        return true;
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: invalid hash type.")
            .Flush();

        return false;
    }
}

// Calculate an HMAC given some input data and a key
bool OpenSSL::HMAC(
    const proto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    const std::uint8_t* key,
    const size_t keySize,
    std::uint8_t* output) const
{
    unsigned int size = 0;
    const EVP_MD* evp_md = OpenSSLdp::HashTypeToOpenSSLType(hashType);

    if (nullptr != evp_md) {
        void* data =
            ::HMAC(evp_md, key, keySize, input, inputSize, nullptr, &size);

        if (nullptr != data) {
            OTPassword::safe_memcpy(output, size, data, size);

            return true;
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to produce a valid HMAC.")
                .Flush();

            return false;
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid hash type.").Flush();

        return false;
    }
}

#if OT_CRYPTO_SUPPORTED_KEY_RSA
/*
 128 bytes * 8 bits == 1024 bits key.  (RSA)

 */
bool OpenSSL::OpenSSLdp::SignContractDefaultHash(
    const Data& strContractUnsigned,
    const EVP_PKEY* pkey,
    Data& theSignature,
    const OTPasswordData*) const
{

    // 32 bytes, double sha256
    // This stores the message digest, pre-encrypted, but with the padding
    // added.
    auto hash = Data::Factory();
    crypto_.Hash().Digest(proto::HASHTYPE_SHA256, strContractUnsigned, hash);

    // This stores the final signature, when the EM value has been signed by RSA
    // private key.
    std::vector<std::uint8_t> vEM(crypto_.Config().PublicKeysizeMax());
    std::vector<std::uint8_t> vpSignature(crypto_.Config().PublicKeysizeMax());

    OTPassword::zeroMemory(&vEM.at(0), crypto_.Config().PublicKeysizeMax());
    OTPassword::zeroMemory(
        &vpSignature.at(0), crypto_.Config().PublicKeysizeMax());

    // Here, we convert the EVP_PKEY that was passed in, to an RSA key for
    // signing.
    //
    auto* pRsaKey = EVP_PKEY_get1_RSA(const_cast<EVP_PKEY*>(pkey));

    if (!pRsaKey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": EVP_PKEY_get1_RSA failed with error ")(
            ERR_error_string(ERR_get_error(), nullptr))(".")
            .Flush();
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
     std::int32_t RSA_padding_add_PKCS1_PSS(RSA* rsa, std::uint8_t* EM, const
     uint8_t*
     mHash, const EVP_MD* Hash, std::int32_t sLen);
     */
    //    std::int32_t RSA_padding_add_xxx(std::uint8_t* to, std::int32_t tlen,
    //                            std::uint8_t *f, std::int32_t fl);
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
    std::int32_t status = RSA_padding_add_PKCS1_PSS(
        pRsaKey,
        &vEM.at(0),
        static_cast<const unsigned char*>(hash->data()),
        md_sha256,
        -2);  // maximum salt length

    // Above, pDigest is the input, but its length is not needed, since it is
    // determined
    // by the digest algorithm (md_sha256.) In this case, that size is 32 bytes
    // ==
    // 256 bits.

    // More clearly: pDigest is 256 bits std::int64_t, aka 32 bytes. The call to
    // RSA_padding_add_PKCS1_PSS above
    // is transforming its contents based on digest1, into EM. Once this is
    // done, the new digest stored in
    // EM will be RSA_size(pRsaKey)-11 bytes in size, with the rest padded.
    // Therefore if this is sucessful, then we can call RSA_private_encrypt
    // without any further padding,
    // since it's already accomplished here. EM itself will be RSA_size(pRsaKey)
    // in size total (exactly.)

    if (!status)  // 1 or 0.
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": RSA_padding_add_PKCS1_PSS failure: ")(
            ERR_error_string(ERR_get_error(), nullptr))(".")
            .Flush();
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
    //    std::int32_t RSA_private_encrypt(int32_t flen, std::uint8_t* from,
    //                            std::uint8_t *to, key::RSA* rsa, std::int32_t
    //                            padding);
    // RSA_private_encrypt() signs the *flen* bytes at *from* (usually a message
    // digest with
    // an algorithm identifier) using the private key rsa and stores the
    // signature in *to*.
    // to must point to RSA_size(rsa) bytes of memory.
    // RSA_private_encrypt() returns the size of the signature (i.e.,
    // RSA_size(rsa)).
    //
    status = RSA_private_encrypt(
        RSA_size(pRsaKey),   // input
        &vEM.at(0),          // padded message digest (input)
        &vpSignature.at(0),  // encrypted padded message digest (output)
        pRsaKey,             // private key (input )
        RSA_NO_PADDING);  // why not RSA_PKCS1_PADDING ? (Custom padding above
                          // in
                          // PSS mode with two hashes.)

    if (status == -1) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": RSA_private_encrypt failure: ")(
            ERR_error_string(ERR_get_error(), nullptr))(".")
            .Flush();
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }
    // status contains size

    // RSA_private_encrypt actually returns the right size.
    const auto binSignature = Data::Factory(&vpSignature.at(0), status);
    theSignature.Assign(binSignature->data(), binSignature->size());

    if (pRsaKey) { RSA_free(pRsaKey); }

    pRsaKey = nullptr;

    return true;
}

bool OpenSSL::OpenSSLdp::VerifyContractDefaultHash(
    const Data& strContractToVerify,
    const EVP_PKEY* pkey,
    const Data& theSignature,
    const OTPasswordData*) const
{

    // 32 bytes, double sha256
    auto hash = Data::Factory();
    crypto_.Hash().Digest(proto::HASHTYPE_SHA256, strContractToVerify, hash);

    std::vector<std::uint8_t> vDecrypted(
        crypto_.Config().PublicKeysizeMax());  // Contains the
                                               // decrypted
                                               // signature.

    OTPassword::zeroMemory(
        &vDecrypted.at(0), crypto_.Config().PublicKeysizeMax());

    // Here, we convert the EVP_PKEY that was passed in, to an RSA key for
    // signing.
    auto* pRsaKey = EVP_PKEY_get1_RSA(const_cast<EVP_PKEY*>(pkey));

    if (!pRsaKey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": EVP_PKEY_get1_RSA failed with error ")(
            ERR_error_string(ERR_get_error(), nullptr))(".")
            .Flush();
        return false;
    }

    const std::int32_t nSignatureSize = static_cast<std::int32_t>(
        theSignature.size());  // converting from unsigned to signed (since
                               // openssl wants it that way.)

    if ((theSignature.size() < static_cast<std::uint32_t>(RSA_size(pRsaKey))) ||
        (nSignatureSize < RSA_size(pRsaKey)))  // this one probably unnecessary.
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Decoded base64-encoded data for signature, but "
            "resulting size was < RSA_size(pRsaKey): "
            "Signed: ")(nSignatureSize)(". Unsigned: ")(theSignature.size())(
            ".")
            .Flush();
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }

    // now we will verify the signature
    // Start by a RAW decrypt of the signature
    // output goes to pDecrypted
    // FYI: const void * theSignature.GetPointer()

    // RSA_PKCS1_OAEP_PADDING
    // RSA_PKCS1_PADDING

    // the 128 in the below call was a BUG. The SIZE of the ciphertext
    // (signature) being decrypted is NOT 128 (modulus / cleartext size).
    // Rather, the size of the signature is RSA_size(pRsaKey).  Will have to
    // revisit this likely, elsewhere in the code.
    //    status = RSA_public_decrypt(128, static_cast<const
    // std::uint8_t*>(theSignature.GetPointer()), pDecrypted, pRsaKey,
    // RSA_NO_PADDING);
    std::int32_t status = RSA_public_decrypt(
        nSignatureSize,  // length of signature, aka RSA_size(rsa)
        static_cast<const std::uint8_t*>(theSignature.data()),  // location of
                                                                // signature
        &vDecrypted.at(0),  // Output--must be large enough to hold the md
                            // (which is smaller than RSA_size(rsa) - 11)
        pRsaKey,            // signer's public key
        RSA_NO_PADDING);

    // std::int32_t RSA_public_decrypt(int32_t flen, std::uint8_t* from,
    //                            std::uint8_t *to, key::RSA* rsa, std::int32_t
    //                            padding);

    // RSA_public_decrypt() recovers the message digest from the *flen* bytes
    // std::int64_t signature at *from*,
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

    if (status == -1)  // Error
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": RSA_public_decrypt failed with error ")(
            ERR_error_string(ERR_get_error(), nullptr))(".")
            .Flush();
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
     std::int32_t RSA_verify_PKCS1_PSS(RSA* rsa, const std::uint8_t* mHash,
     const EVP_MD* Hash, const uint8_t* EM, std::int32_t sLen)
     */  // rsa        mHash    Hash alg.    EM         sLen

    const EVP_MD* md_sha256 = EVP_sha256();
    status = RSA_verify_PKCS1_PSS(
        pRsaKey,
        static_cast<const unsigned char*>(hash->data()),
        md_sha256,
        &vDecrypted.at(0),
        -2);  // salt length recovered from signature

    if (!status) {
        LogInsane(OT_METHOD)(__FUNCTION__)(
            ": RSA_verify_PKCS1_PSS failed with error: ")(
            std::string(ERR_error_string(ERR_get_error(), nullptr)))
            .Flush();
        RSA_free(pRsaKey);
        pRsaKey = nullptr;
        return false;
    }

    LogInsane(OT_METHOD)(__FUNCTION__)(": Signature verified").Flush();

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

     std::int32_t RSA_sign(int32_t type, const std::uint8_t* m, std::uint32_t
     m_len, uint8_t*
     sigret, std::uint32_t* siglen, key::RSA* rsa);
     std::int32_t RSA_verify(int32_t type, const std::uint8_t* m, std::uint32_t
     m_len, uint8_t*
     sigbuf, std::uint32_t siglen, key::RSA* rsa);

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
     > bits std::int64_t, BUT:
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
     > I had an std::uint8_t array a with length = 20, key::RSA* r,
     > std::uint8_t* sig = (uint8_t*) malloc(RSA_size(r)) and then I invoked
     > function std::int32_t i = RSA_private_encrypt(20,a ,sign,r,RSA_NO_PADDING
     );
     The
     > returned value  i = -1 means that this function failed. However, when I
     > invoked std::int32_t i = RSA_private_encrypt(20,a,sig,r,RSA_PKCS1_PADDING
     ),
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
     knotwork, go into OTLib/Contract.cpp
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
     std::uint8_t*>(theSignature.GetPointer()), pDecrypted, pRsaKey,
     RSA_NO_PADDING);
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
bool OpenSSL::OpenSSLdp::SignContract(
    const Data& strContractUnsigned,
    const EVP_PKEY* pkey,
    Data& theSignature,
    const proto::HashType hashType,
    const OTPasswordData* pPWData) const
{
    OT_ASSERT_MSG(
        nullptr != pkey, "Null private key sent to OpenSSL::SignContract.\n");

#if OPENSSL_VERSION_NUMBER < 0x10100000L
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
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failure in cleanup. (It returned 0).")
                    .Flush();
        }
    };
#endif

    OTString strHashType = HashingProvider::HashTypeToString(hashType);

    EVP_MD* md = nullptr;

    if (proto::HASHTYPE_SHA256 == hashType) {
        return SignContractDefaultHash(
            strContractUnsigned, pkey, theSignature, pPWData);
    }

    //    else
    {
        md = const_cast<EVP_MD*>(OpenSSLdp::HashTypeToOpenSSLType(hashType));
    }

    // If it's not the default hash, then it's just a normal hash.
    // Either way then we process it, first by getting the message digest
    // pointer for signing.

    if (nullptr == md) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to decipher Hash algorithm: ")(strHashType)(".")
            .Flush();
        return false;
    }

// RE: EVP_SignInit() or EVP_MD_CTX_init()...
//
// Since only a copy of the digest context is ever finalized the
// context MUST be cleaned up after use by calling EVP_MD_CTX_cleanup()
// or a memory leak will occur.
//
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_MD_CTX md_ctx;
    _OTCont_SignCont1 theInstance(szFunc, md_ctx);
#else
    DigestContext context;
#endif

// Do the signature
// Note: I just changed this to the _ex version (in case I'm debugging later
// and find a problem here.)
//
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_SignInit_ex(&md_ctx, md, nullptr);
#else
    EVP_SignInit_ex(context, md, nullptr);
#endif

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_SignUpdate(
        &md_ctx,
        strContractUnsigned.GetPointer(),
        strContractUnsigned.GetSize());
#else
    EVP_SignUpdate(
        context, strContractUnsigned.data(), strContractUnsigned.size());
#endif

    std::uint8_t sig_buf[4096];  // Safe since we pass the size when we use it.

    std::int32_t sig_len = sizeof(sig_buf);
    std::int32_t err = EVP_SignFinal(
        context,
        sig_buf,
        reinterpret_cast<std::uint32_t*>(&sig_len),
        const_cast<EVP_PKEY*>(pkey));

    if (err != 1) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error signing xml contents.")
            .Flush();
        return false;
    } else {
        LogDebug(OT_METHOD)(__FUNCTION__)(": Successfully signed xml contents.")
            .Flush();

        // We put the signature data into the signature object that
        // was passed in for that purpose.
        theSignature.Assign(sig_buf, sig_len);

        return true;
    }
}

// All the other various versions eventually call this one, where the actual
// work is done.
bool OpenSSL::OpenSSLdp::VerifySignature(
    const Data& strContractToVerify,
    const EVP_PKEY* pkey,
    const Data& theSignature,
    const proto::HashType hashType,
    const OTPasswordData* pPWData) const
{
    OT_ASSERT_MSG(
        strContractToVerify.size() > 0,
        "OpenSSL::VerifySignature: ASSERT FAILURE: "
        "strContractToVerify.GetSize()>0");
    OT_ASSERT_MSG(nullptr != pkey, "Null pkey in OpenSSL::VerifySignature.\n");

    const char* szFunc = "OpenSSL::VerifySignature";

    OTString strHashType = HashingProvider::HashTypeToString(hashType);

    EVP_MD* md = nullptr;

    if (proto::HASHTYPE_SHA256 == hashType) {
        return VerifyContractDefaultHash(
            strContractToVerify, pkey, theSignature, pPWData);
    }

    //    else
    {
        md = const_cast<EVP_MD*>(OpenSSLdp::HashTypeToOpenSSLType(hashType));
    }

    if (!md) {
        LogDetail(OT_METHOD)(__FUNCTION__)(szFunc)(
            ": Unknown message digest algorithm: ")(strHashType)
            .Flush();
        return false;
    }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_MD_CTX ctx;
    EVP_MD_CTX_init(&ctx);
    EVP_VerifyInit(&ctx, md);
#else
    DigestContext context;
    EVP_VerifyInit(context, md);
#endif

    // Here I'm adding the actual XML portion of the contract (the portion that
    // gets signed.)
    // Basically we are repeating similarly to the signing process in order to
    // verify.

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_VerifyUpdate(
        &ctx, strContractToVerify.GetPointer(), strContractToVerify.GetSize());
#else
    EVP_VerifyUpdate(
        context, strContractToVerify.data(), strContractToVerify.size());
#endif

    // Now we pass in the Signature
    // EVP_VerifyFinal() returns 1 for a correct signature,
    // 0 for failure and -1 if some other error occurred.
    //
    std::int32_t nErr = EVP_VerifyFinal(
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        &ctx,
#else
        context,
#endif
        static_cast<const std::uint8_t*>(theSignature.data()),
        theSignature.size(),
        const_cast<EVP_PKEY*>(pkey));

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_MD_CTX_cleanup(&ctx);
#endif

    // the moment of true. 1 means the signature verified.
    if (1 == nErr)
        return true;
    else
        return false;
}

bool OpenSSL::Sign(
    const Data& plaintext,
    const key::Asymmetric& theKey,
    const proto::HashType hashType,
    Data& signature,  // output
    const OTPasswordData* pPWData,
    __attribute__((unused)) const OTPassword* exportPassword) const
{

    auto& theTempKey = const_cast<key::Asymmetric&>(theKey);
    auto* pTempOpenSSLKey =
        dynamic_cast<key::implementation::RSA*>(&theTempKey);
    OT_ASSERT(nullptr != pTempOpenSSLKey);

    const EVP_PKEY* pkey = pTempOpenSSLKey->dp->GetKey(pPWData);
    OT_ASSERT(nullptr != pkey);

    Lock lock(lock_);
    if (false ==
        dp_->SignContract(plaintext, pkey, signature, hashType, pPWData)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": SignContract returned false.")
            .Flush();
        return false;
    }

    return true;
}

bool OpenSSL::Verify(
    const Data& plaintext,
    const key::Asymmetric& theKey,
    const Data& signature,
    const proto::HashType hashType,
    const OTPasswordData* pPWData) const
{
    auto& theTempKey = const_cast<key::Asymmetric&>(theKey);
    auto* pTempOpenSSLKey =
        dynamic_cast<key::implementation::RSA*>(&theTempKey);
    OT_ASSERT(nullptr != pTempOpenSSLKey);

    const EVP_PKEY* pkey = pTempOpenSSLKey->dp->GetKey(pPWData);
    OT_ASSERT(nullptr != pkey);

    Lock lock(lock_);
    if (false ==
        dp_->VerifySignature(plaintext, pkey, signature, hashType, pPWData)) {
        LogDebug(OT_METHOD)(__FUNCTION__)(": VerifySignature returned false.")
            .Flush();
        return false;
    }

    return true;
}

// Seal up as envelope (Asymmetric, using public key and then AES key.)
bool OpenSSL::EncryptSessionKey(
    const mapOfAsymmetricKeys& RecipPubKeys,
    Data& plaintext,
    Data& dataOutput) const
{
    OT_ASSERT_MSG(
        !RecipPubKeys.empty(),
        "OpenSSL::Seal: ASSERT: RecipPubKeys.size() > 0");

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_CIPHER_CTX ctx;
#else
    CipherContext context;
#endif

    std::uint8_t buffer[4096];
    std::uint8_t buffer_out[4096 + EVP_MAX_IV_LENGTH];
    std::uint8_t iv[EVP_MAX_IV_LENGTH];

    std::uint32_t len = 0;
    std::int32_t len_out = 0;

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
        nullptr;  // These will be pointers we use, but do NOT need to clean-up.
    std::uint8_t** ek = nullptr;    // These we DO need to cleanup...
    std::int32_t* eklen = nullptr;  // This will just be an array of integers.

    bool bFinalized = false;  // If this is set true, then we don't bother to
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
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        const char* m_szFunc;
        EVP_CIPHER_CTX& m_ctx;  // reference to openssl cipher context.
#endif
        EVP_PKEY*** m_array_pubkey;  // pointer to array of public key pointers.
        std::uint8_t*** m_ek;  // pointer to array of encrypted symmetric keys.
        std::int32_t** m_eklen;  // pointer to array of lengths for each
                                 // encrypted
                                 // symmetric key
        const mapOfAsymmetricKeys& m_RecipPubKeys;  // array of public keys (to
        // initialize the above members
        // with.)
        std::int32_t m_nLastPopulatedIndex;  // We store the highest-populated
                                             // index
        // (so we can free() up 'til the same
        // index, in destructor.)
        bool& m_bFinalized;

    public:
        _OTEnv_Seal(
#if OPENSSL_VERSION_NUMBER < 0x10100000L
            const char* param_szFunc,
            EVP_CIPHER_CTX& theCTX,
#endif
            EVP_PKEY*** param_array_pubkey,
            std::uint8_t*** param_ek,
            std::int32_t** param_eklen,
            const mapOfAsymmetricKeys& param_RecipPubKeys,
            bool& param_Finalized)
#if OPENSSL_VERSION_NUMBER < 0x10100000L
            : m_szFunc(param_szFunc)
            , m_ctx(theCTX)
            , m_array_pubkey(nullptr)
#else
            : m_array_pubkey(nullptr)
#endif
            , m_ek(nullptr)
            , m_eklen(nullptr)
            , m_RecipPubKeys(param_RecipPubKeys)
            , m_nLastPopulatedIndex(-1)
            , m_bFinalized(param_Finalized)
        {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
            if (nullptr == param_szFunc) OT_FAIL;
#endif
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

#if OPENSSL_VERSION_NUMBER < 0x10100000L
            // EVP_CIPHER_CTX_init() corresponds to: EVP_CIPHER_CTX_cleanup()
            // EVP_CIPHER_CTX_cleanup clears all information from a cipher
            // context and free up any allocated
            // memory associate with it. It should be called after all
            // operations using a cipher are complete
            // so sensitive information does not remain in memory.
            //
            EVP_CIPHER_CTX_init(&m_ctx);
#endif

            // (*m_array_pubkey)[] array must have m_RecipPubKeys.size() no. of
            // elements (each containing a pointer
            // to an EVP_PKEY that we must NOT clean up.)
            //
            *m_array_pubkey = static_cast<EVP_PKEY**>(
                malloc(m_RecipPubKeys.size() * sizeof(EVP_PKEY*)));
            OT_ASSERT(nullptr != *m_array_pubkey);
            memset(
                *m_array_pubkey,
                0,
                m_RecipPubKeys.size() * sizeof(EVP_PKEY*));  // size of array
                                                             // length *
                                                             // sizeof(pointer)

            // (*m_ek)[] array must have m_RecipPubKeys.size() no. of elements
            // (each will contain a pointer from OpenSSL that we must clean up.)
            //
            *m_ek = static_cast<std::uint8_t**>(
                malloc(m_RecipPubKeys.size() * sizeof(std::uint8_t*)));
            if (nullptr == *m_ek) OT_FAIL;
            memset(
                *m_ek,
                0,
                m_RecipPubKeys.size() * sizeof(std::uint8_t*));  // size of
                                                                 // array
                                                                 // length
                                                                 // *
            // sizeof(pointer)

            // (*m_eklen)[] array must also have m_RecipPubKeys.size() no. of
            // elements (each containing a size as integer.)
            //
            *m_eklen = static_cast<std::int32_t*>(
                malloc(m_RecipPubKeys.size() * sizeof(std::int32_t)));
            OT_ASSERT(nullptr != *m_eklen);
            memset(
                *m_eklen,
                0,
                m_RecipPubKeys.size() *
                    sizeof(std::int32_t));  // size of array length *
                                            // sizeof(std::int32_t)

            //
            // ABOVE is all just above allocating the memory and setting it to 0
            // / nullptr.
            //
            // Whereas BELOW is about populating that memory, so the actual
            // OTEnvelope::Seal() function can use it.
            //

            std::int32_t nKeyIndex = -1;  // it will be 0 upon first iteration.

            for (auto& it : m_RecipPubKeys) {
                ++nKeyIndex;  // 0 on first iteration.
                m_nLastPopulatedIndex = nKeyIndex;

                key::Asymmetric* pTempPublicKey =
                    it.second;  // first is the NymID
                OT_ASSERT(nullptr != pTempPublicKey);

                auto* pPublicKey =
                    dynamic_cast<key::implementation::RSA*>(pTempPublicKey);
                OT_ASSERT(nullptr != pPublicKey);

                EVP_PKEY* public_key =
                    const_cast<EVP_PKEY*>(pPublicKey->dp->GetKey());
                OT_ASSERT(nullptr != public_key);

                // Copy the public key pointer to an array of public key
                // pointers...
                //
                (*m_array_pubkey)[nKeyIndex] =
                    public_key;  // For OpenSSL, it needs an array of ALL the
                                 // public keys.

                // We allocate enough space for the encrypted symmetric key to
                // be placed
                // at this index (the space determined based on size of the
                // public key that
                // the symmetric key will be encrypted to.) The space is left
                // empty, for OpenSSL
                // to populate.

                // (*m_ek)[i] must have room for EVP_PKEY_size(pubk[i]) bytes.
                (*m_ek)[nKeyIndex] = static_cast<std::uint8_t*>(
                    malloc(EVP_PKEY_size(public_key)));
                OT_ASSERT(nullptr != (*m_ek)[nKeyIndex]);
                memset((*m_ek)[nKeyIndex], 0, EVP_PKEY_size(public_key));
            }
        }

        ~_OTEnv_Seal()
        {
            OT_ASSERT(nullptr != m_array_pubkey);  // 1. pointer to an array of
                                                   // pointers to EVP_PKEY,
            OT_ASSERT(nullptr != m_ek);  // 2. pointer to an array of pointers
                                         // to encrypted symmetric keys
            OT_ASSERT(nullptr != m_eklen);  // 3. pointer to an array storing
                                            // the lengths of those keys.

            // Iterate the array of encrypted symmetric keys, and free the key
            // at each index...
            //
            // We know how many there are, because each pointer will otherwise
            // be nullptr.
            // Plus we have m_nLastPopulatedIndex, which is obviously as far as
            // we will go.
            //

            std::int32_t nKeyIndex = -1;  // it will be 0 upon first iteration.
            while (nKeyIndex < m_nLastPopulatedIndex)  // if
                                                       // m_nLastPopulatedIndex
                                                       // is 0, then this loop
            // will iterate ONCE, with
            // nKeyIndex incrementing
            // to 0 on the first line.
            {
                ++nKeyIndex;  // 0 on first iteration.

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

            if (nullptr != *m_array_pubkey)  // NOTE: The individual pubkeys are
                                             // NOT to be cleaned up, but this
                                             // array, containing pointers to
                                             // those pubkeys, IS cleaned up.
                free(*m_array_pubkey);
            *m_array_pubkey = nullptr;
            m_array_pubkey = nullptr;
            if (nullptr != *m_ek) free(*m_ek);
            *m_ek = nullptr;
            m_ek = nullptr;
            if (nullptr != *m_eklen) free(*m_eklen);
            *m_eklen = nullptr;
            m_eklen = nullptr;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
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
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failure in EVP_CIPHER_CTX_cleanup. "
                        "(It returned 0).")
                        .Flush();
            }
#endif
        }
    };  // class _OTEnv_Seal

    // INSTANTIATE IT (This does all our setup on construction here, AND cleanup
    // on destruction, whenever exiting this function.)
    _OTEnv_Seal local_RAII(
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        szFunc, ctx, &array_pubkey, &ek, &eklen, RecipPubKeys, bFinalized);
#else
        &array_pubkey, &ek, &eklen, RecipPubKeys, bFinalized);
#endif

    // This is where the envelope final contents will be placed.
    // including the size of the encrypted symmetric key, the symmetric key
    // itself, the initialization vector, and the ciphertext.
    //
    dataOutput.Release();

    const EVP_CIPHER* cipher_type = EVP_aes_256_cbc();  // todo hardcoding.

    /*
    std::int32_t EVP_SealInit(EVP_CIPHER_CTX* ctx, const EVP_CIPHER* type,
                     std::uint8_t **ek, std::int32_t* ekl, uint8_t* iv,
                     EVP_PKEY **pubk,     std::int32_t npubk);

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
    //    std::uint8_t ** ek           = nullptr;  // These we DO need to
    //    cleanup...
    //    std::int32_t           *  eklen        = nullptr;  // This will just
    //    be an
    // array of integers.

    if (!EVP_SealInit(
#if OPENSSL_VERSION_NUMBER < 0x10100000L
            &ctx,
#else
            context,
#endif
            cipher_type,
            ek,
            eklen,  // array of buffers for output of encrypted copies of the
                    // symmetric "session key". (Plus array of ints, to receive
                    // the size of each key.)
            iv,     // A buffer where the generated IV is written. Must contain
                    // room for the corresponding cipher's IV, as determined by
                    // (for example) EVP_CIPHER_iv_length(type).
            array_pubkey,
            static_cast<std::int32_t>(RecipPubKeys.size())))  // array of public
                                                              // keys
    // we are addressing
    // this envelope to.
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(": EVP_SealInit: Failed.").Flush();
        return false;
    }

    // Write the ENVELOPE TYPE (network order version.)
    //
    // 0 == Error
    // 1 == Asymmetric Key  (this function -- Seal / Open)
    // 2 == Symmetric Key   (other functions -- Encrypt / Decrypt use this.)
    // Anything else: error.

    uint16_t temp_env_type = 1;  // todo hardcoding.
    // Calculate "network-order" version of envelope type 1.
    uint16_t env_type_n = htons(temp_env_type);

    dataOutput.Concatenate(
        reinterpret_cast<void*>(&env_type_n),
        static_cast<std::uint32_t>(sizeof(env_type_n)));

    // Write the ARRAY SIZE (network order version.)

    // Calculate "network-order" version of array size.
    std::uint32_t array_size_n = htonl(RecipPubKeys.size());

    dataOutput.Concatenate(
        reinterpret_cast<void*>(&array_size_n),
        static_cast<std::uint32_t>(sizeof(array_size_n)));

    LogInsane(OT_METHOD)(__FUNCTION__)(": Envelope type:  ")(env_type_n)(
        "    Array size: ")(array_size_n)
        .Flush();

    OT_ASSERT(nullptr != ek);
    OT_ASSERT(nullptr != eklen);

    // Loop through the encrypted symmetric keys, and for each, write its
    // network-order NymID size, and its NymID, and its network-order content
    // size,
    // and its content, to the envelope data contents
    // (that we are currently building...)
    //
    std::int32_t ii = -1;  // it will be 0 upon first iteration.

    for (auto& it : RecipPubKeys) {
        ++ii;  // 0 on first iteration.

        std::string str_nym_id = it.first;
        //        key::Asymmetric * pTempPublicKey = it->second;
        //        OT_ASSERT(nullptr != pTempPublicKey);

        //        RSA * pPublicKey =
        // dynamic_cast<key::RSA*>(pTempPublicKey);
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

        const auto strNymID = String::Factory(str_nym_id.c_str());

        // +1 for null terminator.
        std::uint32_t nymid_len = strNymID->GetLength() + 1;
        // Calculate "network-order" version of length (+1 for null terminator)
        std::uint32_t nymid_len_n = htonl(nymid_len);

        // Write nymid_len_n and strNymID for EACH encrypted symmetric key.
        //
        dataOutput.Concatenate(
            reinterpret_cast<void*>(&nymid_len_n),
            static_cast<std::uint32_t>(sizeof(nymid_len_n)));

        // (+1 for null terminator is included here already, from above.)
        dataOutput.Concatenate(
            reinterpret_cast<const void*>(strNymID->Get()), nymid_len);

        LogInsane(OT_METHOD)(__FUNCTION__)(": INDEX: ")(ii)(
            "  NymID length:  ")(nymid_len_n)("   Nym ID: ")(strNymID)(
            "   Strlen (should be a byte shorter): ")(strNymID->GetLength())
            .Flush();

        OT_ASSERT(nullptr != ek[ii]);  // assert key pointer not null.
        OT_ASSERT(eklen[ii] > 0);      // assert key length larger than 0.

        // Calculate "network-order" version of length.
        std::uint32_t eklen_n = htonl(static_cast<uint32_t>(eklen[ii]));

        dataOutput.Concatenate(
            reinterpret_cast<void*>(&eklen_n),
            static_cast<std::uint32_t>(sizeof(eklen_n)));

        dataOutput.Concatenate(
            reinterpret_cast<void*>(ek[ii]),
            static_cast<std::uint32_t>(eklen[ii]));

        LogInsane(OT_METHOD)(__FUNCTION__)(": EK length:  ")(eklen_n)(
            "     First byte: ")((ek[ii])[0])("      Last byte: ")(
            (ek[ii])[eklen[ii] - 1])
            .Flush();
    }

    // Write IV size before then writing IV itself.
    //
    std::uint32_t ivlen = static_cast<uint32_t>(
        EVP_CIPHER_iv_length(cipher_type));  // Length of IV for this cipher...
    // (TODO: add cipher name to output,
    // and use it for looking up cipher
    // upon Open.)
    //  OT_ASSERT(ivlen > 0);
    // Calculate "network-order" version of iv length.
    std::uint32_t ivlen_n = htonl(ivlen);
    dataOutput.Concatenate(
        reinterpret_cast<void*>(&ivlen_n),
        static_cast<std::uint32_t>(sizeof(ivlen_n)));
    dataOutput.Concatenate(reinterpret_cast<void*>(iv), ivlen);
    LogInsane(OT_METHOD)(__FUNCTION__)(": iv_size: ")(ivlen_n)(
        "   IV first byte: ")(iv[0])("    IV last byte: ")(iv[ivlen - 1])
        .Flush();

    // Now we process the input and write the encrypted data to the
    // output.
    while (0 < (len = plaintext.OTfread(
                    reinterpret_cast<std::uint8_t*>(buffer),
                    static_cast<std::uint32_t>(sizeof(buffer))))) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        if (!EVP_SealUpdate(
                &ctx,
                buffer_out,
                &len_out,
                buffer,
                static_cast<std::int32_t>(len))) {
#else
        if (!EVP_SealUpdate(
                context,
                buffer_out,
                &len_out,
                buffer,
                static_cast<std::int32_t>(len))) {
#endif
            LogOutput(OT_METHOD)(__FUNCTION__)(": EVP_SealUpdate failed.")
                .Flush();
            return false;
        } else if (len_out > 0)
            dataOutput.Concatenate(
                reinterpret_cast<void*>(buffer_out),
                static_cast<std::uint32_t>(len_out));
        else
            break;
    }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (!EVP_SealFinal(&ctx, buffer_out, &len_out)) {
#else
    if (!EVP_SealFinal(context, buffer_out, &len_out)) {
#endif
        LogOutput(OT_METHOD)(__FUNCTION__)(": EVP_SealFinal failed.").Flush();
        return false;
    }
    // This is the "final" piece that is added from SealFinal just above.
    //
    else if (len_out > 0) {
        bFinalized = true;
        dataOutput.Concatenate(
            reinterpret_cast<void*>(buffer_out),
            static_cast<std::uint32_t>(len_out));
    } else {
        // cppcheck-suppress unreadVariable
        bFinalized = true;
    }

    return true;
}

// RSA / AES
bool OpenSSL::DecryptSessionKey(
    Data& dataInput,
    const Nym& theRecipient,
    Data& plaintext,
    const OTPasswordData* pPWData) const
{

    std::uint8_t buffer[4096];
    std::uint8_t buffer_out[4096 + EVP_MAX_IV_LENGTH];
    std::uint8_t iv[EVP_MAX_IV_LENGTH];

    std::uint32_t len = 0;
    std::int32_t len_out = 0;
    bool bFinalized = false;  // We only clean up the ctx if the Open "Final"
                              // function hasn't been called, since it does that
                              // automatically already.

    memset(buffer, 0, 4096);
    memset(buffer_out, 0, 4096 + EVP_MAX_IV_LENGTH);
    memset(iv, 0, EVP_MAX_IV_LENGTH);

    // plaintext is where we'll put the decrypted result.
    //
    plaintext.zeroMemory();

    // Grab the NymID of the recipient, so we can find his session
    // key (there might be symmetric keys for several Nyms, not just this
    // one, and we need to find the right one in order to perform this Open.)
    //
    auto strNymID = String::Factory();
    theRecipient.GetIdentifier(strNymID);

    key::Asymmetric& theTempPrivateKey =
        const_cast<key::Asymmetric&>(theRecipient.GetPrivateEncrKey());
    auto* pPrivateKey =
        dynamic_cast<key::implementation::RSA*>(&theTempPrivateKey);

    EVP_PKEY* private_key = nullptr;
    if (nullptr != pPrivateKey) {
        private_key = const_cast<EVP_PKEY*>(pPrivateKey->dp->GetKey(pPWData));
    }

    if (nullptr == private_key) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Null private key on recipient. (Returning false).")
            .Flush();
        return false;
    } else {
        LogInsane(OT_METHOD)(__FUNCTION__)(
            ": Private key is available for NymID: ")(strNymID)
            .Flush();
    }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    EVP_CIPHER_CTX ctx;
#else
    CipherContext context;
#endif

    class _OTEnv_Open
    {
    private:
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        const char* m_szFunc;
        EVP_CIPHER_CTX& m_ctx;  // reference to openssl cipher context.
#endif
        key::Asymmetric& m_privateKey;  // reference to OTAsymmetricKey object.
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        bool& m_bFinalized;
#endif

    public:
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        _OTEnv_Open(
            const char* param_szFunc,
            EVP_CIPHER_CTX& theCTX,
            key::Asymmetric& param_privateKey,
            bool& param_Finalized)
            : m_szFunc(param_szFunc)
            , m_ctx(theCTX)
            , m_privateKey(param_privateKey)
            , m_bFinalized(param_Finalized)
#else
        _OTEnv_Open(key::Asymmetric& param_privateKey, bool&)
            : m_privateKey(param_privateKey)
#endif
        {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
            OT_ASSERT(nullptr != param_szFunc);

            EVP_CIPHER_CTX_init(&m_ctx);
#endif
        }

        ~_OTEnv_Open()  // DESTRUCTOR
        {
            m_privateKey.ReleaseKey();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
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
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failure in EVP_CIPHER_CTX_cleanup. "
                        "(It returned 0).")
                        .Flush();
            }

            m_szFunc = nullptr;
#endif
        }
    };

// INSTANTIATE the clean-up object.
//
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    _OTEnv_Open theNestedInstance(szFunc, ctx, *pPrivateKey, bFinalized);
#else
    _OTEnv_Open theNestedInstance(*pPrivateKey, bFinalized);
#endif

    dataInput.reset();  // Reset the fread position on this object to 0.

    std::uint32_t nRunningTotal =
        0;  // Everytime we read something, we add the length to this variable.

    std::uint32_t nReadEnvType = 0;
    std::uint32_t nReadArraySize = 0;
    std::uint32_t nReadIV = 0;

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
                  reinterpret_cast<std::uint8_t*>(&env_type_n),
                  static_cast<std::uint32_t>(sizeof(env_type_n))))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error reading Envelope Type. Expected "
            "asymmetric(1) or symmetric (2).")
            .Flush();
        return false;
    }
    nRunningTotal += nReadEnvType;
    OT_ASSERT(nReadEnvType == static_cast<std::uint32_t>(sizeof(env_type_n)));

    // convert that envelope type from network to HOST endian.
    //
    const uint16_t env_type = ntohs(env_type_n);
    //  nRunningTotal += env_type;    // NOPE! Just because envelope type is 1
    // or 2, doesn't mean we add 1 or 2 extra bytes to the length here. Nope!

    if (1 != env_type) { return false; }

    // Read the ARRAY SIZE (network order version -- convert to host version.)
    //
    std::uint32_t array_size_n = 0;

    if (0 == (nReadArraySize = dataInput.OTfread(
                  reinterpret_cast<std::uint8_t*>(&array_size_n),
                  static_cast<std::uint32_t>(sizeof(array_size_n))))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error reading Array Size for encrypted symmetric keys.")
            .Flush();
        return false;
    }

    nRunningTotal += nReadArraySize;

    OT_ASSERT(
        nReadArraySize == static_cast<std::uint32_t>(sizeof(array_size_n)));

    // convert that array size from network to HOST endian.
    const std::uint32_t array_size = ntohl(array_size_n);
    LogInsane(OT_METHOD)(__FUNCTION__)(": Array size: ")(array_size).Flush();

    // We are going to loop through all the keys and load each up (then delete.)
    // Each one is proceeded by its length.
    // IF we find the one we are looking for, then we set it onto this variable,
    // theRawEncryptedKey, so we have it available below this loop.
    //
    auto theRawEncryptedKey = Data::Factory();
    bool bFoundKeyAlready =
        false;  // If we find it during the loop below, we'll set this to true.

    // Loop through as we read the encrypted symmetric keys, and for each:
    //      read its network-order NymID size (convert to host version), and
    // then read its NymID,
    //      read its network-order key content size (convert to host), and then
    // read its key content,
    //
    for (std::uint32_t ii = 0; ii < array_size; ++ii) {

        // Loop through the encrypted symmetric keys, and for each:
        //      read its network-order NymID size (convert to host version), and
        // then read its NymID,
        //      read its network-order key content size (convert to host), and
        // then read its key content.

        std::uint32_t nymid_len_n = 0;
        std::uint32_t nReadNymIDSize = 0;

        if (0 == (nReadNymIDSize = dataInput.OTfread(
                      reinterpret_cast<std::uint8_t*>(&nymid_len_n),
                      static_cast<std::uint32_t>(sizeof(nymid_len_n))))) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error reading NymID length for an encrypted "
                "symmetric key.")
                .Flush();
            return false;
        }

        nRunningTotal += nReadNymIDSize;

        OT_ASSERT(
            nReadNymIDSize == static_cast<std::uint32_t>(sizeof(nymid_len_n)));

        // convert that array size from network to HOST endian.
        std::uint32_t nymid_len = ntohl(nymid_len_n);
        LogInsane(OT_METHOD)(__FUNCTION__)(": NymID length: ")(nymid_len)
            .Flush();
        std::uint8_t* nymid =
            static_cast<std::uint8_t*>(malloc(sizeof(uint8_t) * nymid_len));

        OT_ASSERT(nullptr != nymid);

        nymid[0] = '\0';  // null terminator.
        std::uint32_t nReadNymID = 0;

        if (0 == (nReadNymID = dataInput.OTfread(
                      nymid,
                      static_cast<std::uint32_t>(
                          sizeof(std::uint8_t) * nymid_len))))  // this length
        // includes the null
        // terminator (it was
        // written that way.)
        {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error reading NymID for an encrypted symmetric key.")
                .Flush();
            free(nymid);
            nymid = nullptr;
            return false;
        }
        nRunningTotal += nReadNymID;
        OT_ASSERT(
            nReadNymID ==
            static_cast<std::uint32_t>(sizeof(std::uint8_t) * nymid_len));
        //      OT_ASSERT(nymid_len == nReadNymID);

        nymid[nymid_len - 1] = '\0';  // for null terminator. If string is 10
        // bytes std::int64_t, it's from 0-9, and the
        // null terminator is at index 9.
        const auto loopStrNymID =
            String::Factory(reinterpret_cast<char*>(nymid));
        free(nymid);
        nymid = nullptr;
        LogInsane(OT_METHOD)(__FUNCTION__)(": (LOOP) Current NymID: ")(
            loopStrNymID)("    Strlen:  ")(loopStrNymID->GetLength())
            .Flush();

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
        std::uint8_t* ek = nullptr;
        std::uint32_t eklen = 0;
        std::uint32_t eklen_n = 0;
        std::uint32_t nReadLength = 0;
        std::uint32_t nReadKey = 0;

        // First we read the encrypted key size.
        //
        if (0 == (nReadLength = dataInput.OTfread(
                      reinterpret_cast<std::uint8_t*>(&eklen_n),
                      static_cast<std::uint32_t>(sizeof(eklen_n))))) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error reading encrypted key size.")
                .Flush();
            return false;
        }

        nRunningTotal += nReadLength;

        OT_ASSERT(nReadLength == static_cast<std::uint32_t>(sizeof(eklen_n)));

        // convert that key size from network to host endian.
        eklen = ntohl(eklen_n);

        LogInsane(OT_METHOD)(__FUNCTION__)(": EK length:  ")(eklen).Flush();
        ek = static_cast<std::uint8_t*>(malloc(
            static_cast<std::int32_t>(eklen) *
            sizeof(std::uint8_t)));  // I assume this is for the AES key

        OT_ASSERT(nullptr != ek);

        memset(static_cast<void*>(ek), 0, static_cast<std::int32_t>(eklen));

        // Next we read the encrypted key itself...
        if (0 == (nReadKey = dataInput.OTfread(ek, eklen))) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error reading encrypted key.")
                .Flush();
            free(ek);
            ek = nullptr;
            return false;
        }

        nRunningTotal += nReadKey;
        LogInsane(OT_METHOD)(__FUNCTION__)(":    EK First byte: ")(ek[0])(
            "     EK Last byte: ")(ek[eklen - 1])
            .Flush();

        OT_ASSERT(nReadKey == eklen);

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
                strNymID->Compare(loopStrNymID);  // FOUND IT! <==========

            if ((ii == (array_size - 1)) ||  // If we're on the LAST INDEX in
                                             // the array (often the only
                                             // index), OR if the
                bNymIDMatches)  // NymID is a guaranteed match, then we'll try
                                // to
                                // decrypt using this session key.
            {  // (Of course also we know that we haven't found the Key yet, or
                // we wouldn't even be here.)
                // NOTE: What if we're on the last index, but the NymID DOES
                // exist, and it DEFINITELY doesn't match?
                // In other words, if loopStrNymID EXISTS, and it DEFINITELY
                // doesn't match (bNymIDMatches is false) then we
                // DEFINITELY want to skip it. But if bNymIDMatches is false
                // simply because loopStrNymID is EMPTY, then we
                // can't rule that key out, in that case.
                //
                if (!(loopStrNymID->Exists() &&
                      !bNymIDMatches))  // Skip if ID was definitely found and
                                        // definitely doesn't match.
                {
                    bFoundKeyAlready = true;

                    theRawEncryptedKey->Assign(static_cast<void*>(ek), eklen);
                    //                  theRawEncryptedKey.Assign(const_cast<const
                    // void *>(static_cast<void *>(ek)), eklen);
                }
            }
        }

        free(ek);
        ek = nullptr;

    }  // for

    if (!bFoundKeyAlready)  // Todo: AND if list of POTENTIAL matches is
                            // also empty...
    {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ":  Sorry: Unable to find a session key for the Nym attempting "
            "to open this envelope: ")(strNymID)(".")
            .Flush();
        return false;
    }

    // Read network-order IV size (convert to host version) before then reading
    // IV itself.
    // (Then update encrypted blocks until evp open final...)
    //
    const std::uint32_t max_iv_length =
        crypto_.Config().SymmetricIvSize();  // I believe this is a
                                             // max length, so it may
                                             // not match the actual
                                             // length.

    // Read the IV SIZE (network order version -- convert to host version.)
    //
    std::uint32_t iv_size_n = 0;
    std::uint32_t nReadIVSize = 0;

    if (0 == (nReadIVSize = dataInput.OTfread(
                  reinterpret_cast<std::uint8_t*>(&iv_size_n),
                  static_cast<std::uint32_t>(sizeof(iv_size_n))))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error reading IV Size for encrypted symmetric keys.")
            .Flush();
        return false;
    }
    nRunningTotal += nReadIVSize;
    OT_ASSERT(nReadIVSize == static_cast<std::uint32_t>(sizeof(iv_size_n)));

    // convert that iv size from network to HOST endian.
    //
    const std::uint32_t iv_size_host_order = ntohl(iv_size_n);

    if (iv_size_host_order > max_iv_length) {
        const std::int64_t l1 = iv_size_host_order, l2 = max_iv_length;
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: iv_size (")(l1)(
            ") is larger than max_iv_length (")(l2)(").")
            .Flush();
        return false;
    } else
        LogInsane(OT_METHOD)(__FUNCTION__)(": IV size: ")(iv_size_host_order)
            .Flush();

    // Then read the IV (initialization vector) itself.
    //
    if (0 == (nReadIV = dataInput.OTfread(
                  reinterpret_cast<std::uint8_t*>(iv),
                  static_cast<std::uint32_t>(iv_size_host_order)))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error reading initialization vector.")
            .Flush();
        return false;
    }

    nRunningTotal += nReadIV;

    OT_ASSERT(nReadIV == static_cast<std::uint32_t>(iv_size_host_order));

    LogInsane(OT_METHOD)(__FUNCTION__)(":    IV First byte: ")(iv[0])(
        "     IV Last byte: ")(iv[iv_size_host_order - 1])
        .Flush();

    // We read the encrypted key size, then we read the encrypted key itself,
    // with nReadKey containing
    // the number of bytes actually read. The IF statement says "if 0 ==" but it
    // should probably say
    // "if eklen !=" (right?) Wrong: because I think it's a max length.
    //
    // We create an Data object to store the ciphertext itself, which begins
    // AFTER the end of the IV.
    // So we see pointer + nRunningTotal as the starting point for the
    // ciphertext.
    // the size of the ciphertext, meanwhile, is the size of the entire thing,
    // MINUS nRunningTotal.
    //
    auto ciphertext = Data::Factory(
        static_cast<const void*>(
            static_cast<const std::uint8_t*>(dataInput.data()) + nRunningTotal),
        dataInput.size() - nRunningTotal);

    //
    const EVP_CIPHER* cipher_type = EVP_aes_256_cbc();  // todo hardcoding.

    // std::int32_t EVP_OpenInit(
    //          EVP_CIPHER_CTX *ctx,
    //          EVP_CIPHER *type,
    //          std::uint8_t *ek,
    //          std::int32_t ekl,
    //          std::uint8_t *iv,
    //          EVP_PKEY *priv);

    //  if (!EVP_OpenInit(&ctx, cipher_type, ek, eklen, iv, private_key))
    if (!EVP_OpenInit(
#if OPENSSL_VERSION_NUMBER < 0x10100000L
            &ctx,
#else
            context,
#endif
            cipher_type,
            static_cast<const std::uint8_t*>(theRawEncryptedKey->data()),
            static_cast<std::int32_t>(theRawEncryptedKey->size()),
            static_cast<const std::uint8_t*>(iv),
            private_key)) {

        // EVP_OpenInit() initializes a cipher context ctx for decryption with
        // cipher type. It decrypts the encrypted
        //    symmetric key of length ekl bytes passed in the ek parameter using
        // the private key priv. The IV is supplied
        //    in the iv parameter.

        LogOutput(OT_METHOD)(__FUNCTION__)(": EVP_OpenInit: Failed.").Flush();
        return false;
    }

    // Now we process ciphertext and write the decrypted data to plaintext.
    // We loop through the ciphertext and process it in blocks...
    //
    while (0 < (len = ciphertext->OTfread(
                    reinterpret_cast<std::uint8_t*>(buffer),
                    static_cast<std::uint32_t>(sizeof(buffer))))) {
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        if (!EVP_OpenUpdate(
                &ctx,
                buffer_out,
                &len_out,
                buffer,
                static_cast<std::int32_t>(len))) {
#else
        if (!EVP_OpenUpdate(
                context,
                buffer_out,
                &len_out,
                buffer,
                static_cast<std::int32_t>(len))) {
#endif
            LogOutput(OT_METHOD)(__FUNCTION__)(": EVP_OpenUpdate: Failed.")
                .Flush();
            return false;
        } else if (len_out > 0)
            plaintext.Concatenate(
                reinterpret_cast<void*>(buffer_out),
                static_cast<std::uint32_t>(len_out));
        else
            break;
    }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    if (!EVP_OpenFinal(&ctx, buffer_out, &len_out)) {
#else
    if (!EVP_OpenFinal(context, buffer_out, &len_out)) {
#endif
        LogOutput(OT_METHOD)(__FUNCTION__)(": EVP_OpenFinal: Failed.").Flush();
        return false;
    } else if (len_out > 0) {
        bFinalized = true;
        plaintext.Concatenate(
            reinterpret_cast<void*>(buffer_out),
            static_cast<std::uint32_t>(len_out));

    } else {
        // cppcheck-suppress unreadVariable
        bFinalized = true;
    }

    return bFinalized;
}
#endif
}  // namespace opentxs::crypto::implementation
#endif  // OT_CRYPTO_USING_OPENSSL
