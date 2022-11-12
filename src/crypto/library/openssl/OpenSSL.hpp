// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::HashType
// IWYU pragma: no_forward_declare opentxs::crypto::SecretStyle
// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::Role
// IWYU pragma: no_include "opentxs/crypto/asymmetric/Role.hpp"

#pragma once

extern "C" {
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/dh.h>
#include <openssl/evp.h>
#include <openssl/opensslv.h>
#if __has_include(<openssl/provider.h>)
// TODO openssl-3
#include <openssl/provider.h>  // IWYU pragma: keep
#endif
#include <openssl/rsa.h>
#if __has_include(<openssl/types.h>)
// TODO openssl-3
#include <openssl/types.h>  // IWYU pragma: keep
#elif __has_include(<openssl/ossl_typ.h>)
#include <openssl/ossl_typ.h>  // IWYU pragma: keep
#endif
}

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

#include "crypto/library/AsymmetricProvider.hpp"
#include "internal/crypto/library/OpenSSL.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/SecretStyle.hpp"
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
class Session;
}  // namespace api

namespace crypto
{
class Parameters;
}  // namespace crypto

class Data;
class PasswordPrompt;
class Secret;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::crypto
{
using OpenSSL_BIO = std::unique_ptr<::BIO, decltype(&::BIO_free)>;
using OpenSSL_BN = std::unique_ptr<::BIGNUM, decltype(&::BN_free)>;
using OpenSSL_DH = std::unique_ptr<::DH, decltype(&::DH_free)>;
using OpenSSL_EVP_PKEY =
    std::unique_ptr<::EVP_PKEY, decltype(&::EVP_PKEY_free)>;
using OpenSSL_EVP_PKEY_CTX =
    std::unique_ptr<::EVP_PKEY_CTX, decltype(&::EVP_PKEY_CTX_free)>;
#if OPENSSL_VERSION_NUMBER >= 0x1010000fl
using OpenSSL_EVP_MD_CTX =
    std::unique_ptr<::EVP_MD_CTX, decltype(&::EVP_MD_CTX_free)>;
#else
using OpenSSL_EVP_MD_CTX = std::unique_ptr<::EVP_MD_CTX>;
#endif
using OpenSSL_RSA = std::unique_ptr<::RSA, decltype(&::RSA_free)>;
}  // namespace opentxs::crypto

namespace opentxs::crypto::implementation
{
class OpenSSL final : virtual public crypto::OpenSSL, public AsymmetricProvider
{
public:
    auto Digest(
        const crypto::HashType hashType,
        const ReadView data,
        Writer&& output) const noexcept -> bool final;
    auto HMAC(
        const crypto::HashType hashType,
        const ReadView key,
        const ReadView data,
        Writer&& output) const noexcept -> bool final;
    auto PKCS5_PBKDF2_HMAC(
        const void* input,
        const std::size_t inputSize,
        const void* salt,
        const std::size_t saltSize,
        const std::size_t iterations,
        const crypto::HashType hashType,
        const std::size_t bytes,
        void* output) const noexcept -> bool final;
    auto RIPEMD160(const ReadView data, Writer&& destination) const noexcept
        -> bool final;

    using AsymmetricProvider::RandomKeypair;
    auto RandomKeypair(
        Writer&& privateKey,
        Writer&& publicKey,
        const crypto::asymmetric::Role role,
        const Parameters& options,
        Writer&& params) const noexcept -> bool final;
    auto SharedSecret(
        const ReadView publicKey,
        const ReadView privateKey,
        const SecretStyle style,
        Secret& secret) const noexcept -> bool final;
    auto Sign(
        const ReadView plaintext,
        const ReadView key,
        const crypto::HashType hash,
        Writer&& signature) const -> bool final;
    auto Verify(
        const ReadView plaintext,
        const ReadView theKey,
        const ReadView signature,
        const crypto::HashType hashType) const -> bool final;

    OpenSSL() noexcept;
    OpenSSL(const OpenSSL&) = delete;
    OpenSSL(OpenSSL&&) = delete;
    auto operator=(const OpenSSL&) -> OpenSSL& = delete;
    auto operator=(OpenSSL&&) -> OpenSSL& = delete;

    ~OpenSSL() final;

private:
    struct BIO {
        using Buffer = ::BIO*;

        operator Buffer() noexcept { return bio_.get(); }

        auto Export(Writer&& output) noexcept -> bool;
        auto Import(const ReadView input) noexcept -> bool;

#if OPENSSL_VERSION_NUMBER >= 0x1010000fl
        BIO(const BIO_METHOD* type = ::BIO_s_mem()) noexcept;
#else
        BIO(BIO_METHOD* type = ::BIO_s_mem()) noexcept;
#endif

    private:
        OpenSSL_BIO bio_;
    };

    struct DH {
        using Context = ::EVP_PKEY_CTX*;
        using Hash = const ::EVP_MD*;
        using Key = ::EVP_PKEY*;

        OpenSSL_EVP_PKEY_CTX dh_;
        OpenSSL_EVP_PKEY local_;
        OpenSSL_EVP_PKEY remote_;

        operator Context() noexcept { return dh_.get(); }

        auto Local() noexcept { return local_.get(); }
        auto Remote() noexcept { return remote_.get(); }

        auto init_keys(const ReadView prv, const ReadView pub) noexcept -> bool;

        DH() noexcept;
        DH(const DH&) = delete;
        DH(DH&&) = delete;
        auto operator=(const DH&) -> DH& = delete;
        auto operator=(DH&&) -> DH& = delete;
    };

    struct MD {
        using Context = ::EVP_MD_CTX*;
        using Hash = const ::EVP_MD*;
        using Key = ::EVP_PKEY*;

        OpenSSL_EVP_MD_CTX md_;
        Hash hash_;
        OpenSSL_EVP_PKEY key_;

        operator Context() noexcept { return md_.get(); }
        operator Hash() noexcept { return hash_; }
        operator Key() noexcept { return key_.get(); }

        auto init_digest(const crypto::HashType hash) noexcept -> bool;
        auto init_sign(const crypto::HashType hash, const ReadView key) noexcept
            -> bool;
        auto init_verify(
            const crypto::HashType hash,
            const ReadView key) noexcept -> bool;

        MD() noexcept;
        MD(const MD&) = delete;
        MD(MD&&) = delete;
        auto operator=(const MD&) -> MD& = delete;
        auto operator=(MD&&) -> MD& = delete;
    };

    using Instantiate = std::function<::EVP_PKEY*(::BIO*)>;

#if __has_include(<openssl/provider.h>)
    OSSL_PROVIDER* default_provider_;
    OSSL_PROVIDER* legacy_provider_;
#endif
    static auto init_key(
        const ReadView bytes,
        const Instantiate function,
        OpenSSL_EVP_PKEY& output) noexcept -> bool;
    static auto HashTypeToOpenSSLType(const crypto::HashType hashType) noexcept
        -> const ::EVP_MD*;
    static auto primes(const int bits) -> int;

    auto generate_dh(const Parameters& options, ::EVP_PKEY* output)
        const noexcept -> bool;
    auto get_params(
        Writer&& params,
        const Parameters& options,
        ::EVP_PKEY* output) const noexcept -> bool;
    auto import_dh(const ReadView existing, ::EVP_PKEY* output) const noexcept
        -> bool;
    auto make_dh_key(
        Writer&& privateKey,
        Writer&& publicKey,
        Writer&& params,
        const Parameters& options) const noexcept -> bool;
    auto make_signing_key(
        Writer&& privateKey,
        Writer&& publicKey,
        const Parameters& options) const noexcept -> bool;
    auto write_dh(Writer&& params, ::EVP_PKEY* dh) const noexcept -> bool;
    auto write_keypair(Writer&& privateKey, Writer&& publicKey, ::EVP_PKEY* evp)
        const noexcept -> bool;
};
}  // namespace opentxs::crypto::implementation
