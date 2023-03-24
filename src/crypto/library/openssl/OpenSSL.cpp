// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/library/openssl/OpenSSL.hpp"  // IWYU pragma: associated

extern "C" {
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/pem.h>
#if __has_include(<openssl/provider.h>)
// TODO openssl-3
#include <openssl/provider.h>  // IWYU pragma: keep
#endif
}

#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include "crypto/hasher/HasherPrivate.hpp"
#include "internal/core/String.hpp"
#include "internal/crypto/library/Factory.hpp"
#include "internal/crypto/library/HashingProvider.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Hasher.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto OpenSSL() noexcept -> std::unique_ptr<crypto::OpenSSL>
{
    using ReturnType = crypto::implementation::OpenSSL;

    return std::make_unique<ReturnType>();
}
}  // namespace opentxs::factory

namespace opentxs::crypto::implementation
{
#if __has_include(<openssl/provider.h>)
OpenSSL::OpenSSL() noexcept
    : default_provider_(OSSL_PROVIDER_load(nullptr, "legacy"))
    , legacy_provider_(OSSL_PROVIDER_load(nullptr, "default"))
{
    OT_ASSERT(nullptr != default_provider_);
    OT_ASSERT(nullptr != legacy_provider_);
}
#else
OpenSSL::OpenSSL() noexcept = default;
#endif

#if OPENSSL_VERSION_NUMBER >= 0x1010000fl
OpenSSL::BIO::BIO(const BIO_METHOD* type) noexcept
#else
OpenSSL::BIO::BIO(BIO_METHOD* type) noexcept
#endif
    : bio_(::BIO_new(type), ::BIO_free)
{
    OT_ASSERT(bio_);
}

OpenSSL::MD::MD() noexcept
#if OPENSSL_VERSION_NUMBER >= 0x1010000fl
    : md_(::EVP_MD_CTX_new(), ::EVP_MD_CTX_free)
#else
    : md_(std::make_unique<::EVP_MD_CTX>())
#endif
    , hash_(nullptr)
    , key_(nullptr, ::EVP_PKEY_free)
{
    OT_ASSERT(md_);
}

OpenSSL::DH::DH() noexcept
    : dh_(nullptr, ::EVP_PKEY_CTX_free)
    , local_(nullptr, ::EVP_PKEY_free)
    , remote_(nullptr, ::EVP_PKEY_free)
{
}

auto OpenSSL::BIO::Export(Writer&& allocator) noexcept -> bool
{
    auto bytes = ::BIO_ctrl_pending(bio_.get());
    auto out = allocator.Reserve(bytes);

    if (false == out.IsValid(bytes)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate space for output")
            .Flush();

        return false;
    }

    const auto size{static_cast<int>(out.size())};

    if (size != BIO_read(bio_.get(), out, size)) {
        LogError()(OT_PRETTY_CLASS())("Failed write output").Flush();

        return false;
    }

    return true;
}

auto OpenSSL::BIO::Import(const ReadView in) noexcept -> bool
{
    const auto size{static_cast<int>(in.size())};

    if (size != BIO_write(bio_.get(), in.data(), size)) {
        LogError()(OT_PRETTY_CLASS())("Failed read input").Flush();

        return false;
    }

    return true;
}

auto OpenSSL::DH::init_keys(const ReadView prv, const ReadView pub) noexcept
    -> bool
{
    if (false ==
        init_key(
            prv,
            [](auto* bio) {
                return PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
            },
            local_)) {
        LogError()(OT_PRETTY_CLASS())("Failed initializing local private key")
            .Flush();

        return false;
    }

    if (false ==
        init_key(
            pub,
            [](auto* bio) {
                return PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
            },
            remote_)) {
        LogError()(OT_PRETTY_CLASS())("Failed initializing remote public key")
            .Flush();

        return false;
    }

    dh_.reset(EVP_PKEY_CTX_new(local_.get(), nullptr));

    if (false == bool(dh_)) {
        LogError()(OT_PRETTY_CLASS())("Failed initializing dh context").Flush();

        return false;
    }

    return true;
}

auto OpenSSL::MD::init_digest(const crypto::HashType hash) noexcept -> bool
{
    hash_ = HashTypeToOpenSSLType(hash);

    if (nullptr == hash_) {
        LogVerbose()(OT_PRETTY_CLASS())("Unsupported hash algorithm.").Flush();

        return false;
    }

    return true;
}

auto OpenSSL::MD::init_sign(
    const crypto::HashType hash,
    const ReadView key) noexcept -> bool
{
    if (false == init_digest(hash)) { return false; }

    return init_key(
        key,
        [](auto* bio) {
            return PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
        },
        key_);
}

auto OpenSSL::MD::init_verify(
    const crypto::HashType hash,
    const ReadView key) noexcept -> bool
{
    if (false == init_digest(hash)) { return false; }

    return init_key(
        key,
        [](auto* bio) {
            return PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
        },
        key_);
}

auto OpenSSL::HashTypeToOpenSSLType(const crypto::HashType hashType) noexcept
    -> const EVP_MD*
{
    using enum crypto::HashType;

    switch (hashType) {
        case Sha256: {
            return ::EVP_sha256();
        }
        case Sha512: {
            return ::EVP_sha512();
        }
        case Blake2b256: {
            return ::EVP_blake2s256();
        }
        case Blake2b512: {
            return ::EVP_blake2b512();
        }
        case Ripemd160: {
            return ::EVP_ripemd160();
        }
        case Sha1: {
            return ::EVP_sha1();
        }
        case Sha256D: {
            return ::EVP_sha256();
        }
        case Bitcoin: {
            return ::EVP_sha256();
        }
        case Error:
        case None:
        case Blake2b160:
        case Sha256DC:
        case SipHash24:
        default: {
            return nullptr;
        }
    }
}

auto OpenSSL::HashTypeToOpenSSLTypeStage2(
    const crypto::HashType hashType) noexcept -> const EVP_MD*
{
    using enum crypto::HashType;

    switch (hashType) {
        case Sha256D: {
            return ::EVP_sha256();
        }
        case Bitcoin: {
            return ::EVP_ripemd160();
        }
        case Error:
        case None:
        case Sha256:
        case Sha512:
        case Blake2b160:
        case Blake2b256:
        case Blake2b512:
        case Ripemd160:
        case Sha1:
        case Sha256DC:
        case SipHash24:
        default: {
            return nullptr;
        }
    }
}

auto OpenSSL::Digest(
    const crypto::HashType type,
    const ReadView data,
    Writer&& output) const noexcept -> bool

{
    try {
        if (data.size() > std::numeric_limits<int>::max()) {
            throw std::runtime_error{"input too large"};
        }

        const auto size = HashSize(type);

        OT_ASSERT(size <= std::size_t{EVP_MAX_MD_SIZE});

        auto buf = output.Reserve(size);

        if (false == buf.IsValid(size)) {
            throw std::runtime_error{"failed to allocate space for output"};
        }

        auto md = MD{};

        if (false == md.init_digest(type)) {
            throw std::runtime_error{"failed to initialize context"};
        }

        static_assert(
            EVP_MAX_MD_SIZE <= std::numeric_limits<unsigned int>::max());
        auto bytes = static_cast<unsigned int>(size);
        auto rc = EVP_DigestInit_ex(md, md, nullptr);

        if (1 != rc) {
            throw std::runtime_error{"failed to initialize digest operation"};
        }

        rc = EVP_DigestUpdate(
            md,
            reinterpret_cast<const unsigned char*>(data.data()),
            data.size());

        if (1 != rc) {
            throw std::runtime_error{"failed to process plaintext"};
        }

        rc = EVP_DigestFinal_ex(md, buf.as<unsigned char>(), &bytes);

        if (1 != rc) { throw std::runtime_error{"failed to write digest"}; }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

// Calculate an HMAC given some input data and a key
auto OpenSSL::HMAC(
    const crypto::HashType hashType,
    const ReadView key,
    const ReadView data,
    Writer&& output) const noexcept -> bool
{
    try {
        if (false == valid(data)) { throw std::runtime_error{"invalid input"}; }

        if (false == valid(key)) { throw std::runtime_error{"invalid key"}; }

        if (data.size() > std::numeric_limits<int>::max()) {
            throw std::runtime_error{"input too large"};
        }

        if (key.size() > std::numeric_limits<int>::max()) {
            throw std::runtime_error{"key too large"};
        }

        const auto size = HashSize(hashType);

        OT_ASSERT(size <= std::size_t{EVP_MAX_MD_SIZE});

        auto buf = output.Reserve(size);

        if (false == buf.IsValid(size)) {
            throw std::runtime_error{"failed to allocate space for output"};
        }

        const auto* context = HashTypeToOpenSSLType(hashType);

        if (nullptr == context) {
            throw std::runtime_error{"invalid hash type"};
        }

        static_assert(
            EVP_MAX_MD_SIZE <= std::numeric_limits<unsigned int>::max());
        auto bytes = static_cast<unsigned int>(size);
        const auto* rc = ::HMAC(
            context,
            key.data(),
            static_cast<int>(key.size()),
            reinterpret_cast<const unsigned char*>(data.data()),
            data.size(),
            buf.as<unsigned char>(),
            &bytes);

        if (nullptr == rc) {
            throw std::runtime_error{"failed to calculate HMAC"};
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto OpenSSL::Hasher(const crypto::HashType hashType) const noexcept
    -> opentxs::crypto::Hasher
{
    return std::make_unique<HasherPrivate>(
               HashTypeToOpenSSLType(hashType),
               HashTypeToOpenSSLTypeStage2(hashType))
        .release();
}

auto OpenSSL::init_key(
    const ReadView bytes,
    const Instantiate function,
    OpenSSL_EVP_PKEY& output) noexcept -> bool
{
    auto pem = BIO{};

    if (false == pem.Import(bytes)) {
        LogError()(OT_PRETTY_STATIC(OpenSSL))("Failed read private key")
            .Flush();

        return false;
    }

    OT_ASSERT(function);

    output.reset(function(pem));

    if (false == bool(output)) {
        LogError()(OT_PRETTY_STATIC(OpenSSL))("Failed to instantiate EVP_PKEY")
            .Flush();

        return false;
    }

    OT_ASSERT(output);

    return true;
}

auto OpenSSL::PKCS5_PBKDF2_HMAC(
    const void* input,
    const std::size_t inputSize,
    const void* salt,
    const std::size_t saltSize,
    const std::size_t iterations,
    const crypto::HashType hashType,
    const std::size_t bytes,
    void* output) const noexcept -> bool
{
    const auto max = static_cast<std::size_t>(std::numeric_limits<int>::max());

    if (inputSize > max) {
        LogError()(OT_PRETTY_CLASS())("Invalid input size").Flush();

        return false;
    }

    if (saltSize > max) {
        LogError()(OT_PRETTY_CLASS())("Invalid salt size").Flush();

        return false;
    }

    if (iterations > max) {
        LogError()(OT_PRETTY_CLASS())("Invalid iteration count").Flush();

        return false;
    }

    if (bytes > max) {
        LogError()(OT_PRETTY_CLASS())("Invalid output size").Flush();

        return false;
    }

    const auto* algorithm = HashTypeToOpenSSLType(hashType);

    if (nullptr == algorithm) {
        LogError()(OT_PRETTY_CLASS())("Error: invalid hash type: ")(
            HashingProvider::HashTypeToString(hashType).get())
            .Flush();

        return false;
    }

    return 1 == ::PKCS5_PBKDF2_HMAC(
                    static_cast<const char*>(input),
                    static_cast<int>(inputSize),
                    static_cast<const unsigned char*>(salt),
                    static_cast<int>(saltSize),
                    static_cast<int>(iterations),
                    algorithm,
                    static_cast<int>(bytes),
                    static_cast<unsigned char*>(output));
}

auto OpenSSL::RIPEMD160(const ReadView data, Writer&& destination)
    const noexcept -> bool
{
    return Digest(crypto::HashType::Ripemd160, data, std::move(destination));
}

#if __has_include(<openssl/provider.h>)
OpenSSL::~OpenSSL()
{
    OSSL_PROVIDER_unload(legacy_provider_);
    OSSL_PROVIDER_unload(default_provider_);
}
#else
OpenSSL::~OpenSSL() = default;
#endif
}  // namespace opentxs::crypto::implementation
