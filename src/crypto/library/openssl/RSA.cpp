// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::HashType
// IWYU pragma: no_forward_declare opentxs::crypto::SecretStyle
// IWYU pragma: no_forward_declare opentxs::crypto::asymmetric::Role

#include "crypto/library/openssl/OpenSSL.hpp"  // IWYU pragma: associated

extern "C" {
#include <openssl/evp.h>
#include <openssl/pem.h>
}

#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/asymmetric/Role.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/asymmetric/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::implementation
{
auto OpenSSL::generate_dh(const Parameters& options, ::EVP_PKEY* output)
    const noexcept -> bool
{
    auto context = OpenSSL_EVP_PKEY_CTX{
        ::EVP_PKEY_CTX_new_id(EVP_PKEY_DH, nullptr), ::EVP_PKEY_CTX_free};

    if (false == bool(context)) {
        LogError()(OT_PRETTY_CLASS())("Failed EVP_PKEY_CTX_new_id").Flush();

        return false;
    }

    if (1 != ::EVP_PKEY_paramgen_init(context.get())) {
        LogError()(OT_PRETTY_CLASS())("Failed EVP_PKEY_paramgen_init").Flush();

        return false;
    }

    if (1 != EVP_PKEY_CTX_set_dh_paramgen_prime_len(
                 context.get(), options.keySize())) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed EVP_PKEY_CTX_set_dh_paramgen_prime_len")
            .Flush();

        return false;
    }

    if (1 != EVP_PKEY_paramgen(context.get(), &output)) {
        LogError()(OT_PRETTY_CLASS())("Failed EVP_PKEY_paramgen").Flush();

        return false;
    }

    return true;
}

auto OpenSSL::get_params(
    Writer&& params,
    const Parameters& options,
    ::EVP_PKEY* output) const noexcept -> bool
{
    const auto existing = options.DHParams();

    if (0 < existing.size()) {
        if (false == import_dh(existing, output)) {
            LogError()(OT_PRETTY_CLASS())("Failed to import dh params").Flush();

            return false;
        }
    } else {
        if (false == generate_dh(options, output)) {
            LogError()(OT_PRETTY_CLASS())("Failed to generate dh params")
                .Flush();

            return false;
        }
    }

    return write_dh(std::move(params), output);
}

auto OpenSSL::import_dh(const ReadView existing, ::EVP_PKEY* output)
    const noexcept -> bool
{
    struct DH {
        ::DH* dh_;

        DH()
            : dh_(::DH_new())
        {
            OT_ASSERT(nullptr != dh_);
        }
        DH(const DH&) = delete;
        DH(DH&&) = delete;
        auto operator=(const DH&) -> DH& = delete;
        auto operator=(DH&&) -> DH& = delete;

        ~DH()
        {
            if (nullptr != dh_) {
                ::DH_free(dh_);
                dh_ = nullptr;
            }
        }
    };

    auto dh = DH{};
    auto params = BIO{};

    if (false == params.Import(existing)) {
        LogError()(OT_PRETTY_CLASS())("Failed to read dh params").Flush();

        return false;
    }

    if (nullptr == ::PEM_read_bio_DHparams(params, &dh.dh_, nullptr, nullptr)) {
        LogError()(OT_PRETTY_CLASS())("Failed PEM_read_bio_DHparams").Flush();

        return false;
    }

    OT_ASSERT(nullptr != dh.dh_);

    if (1 != ::EVP_PKEY_set1_DH(output, dh.dh_)) {
        LogError()(OT_PRETTY_CLASS())("Failed EVP_PKEY_set1_DH").Flush();

        return false;
    }

    return true;
}

auto OpenSSL::make_dh_key(
    Writer&& privateKey,
    Writer&& publicKey,
    Writer&& dhParams,
    const Parameters& options) const noexcept -> bool
{
    struct Key {
        ::EVP_PKEY* key_;

        Key()
            : key_(::EVP_PKEY_new())
        {
            OT_ASSERT(nullptr != key_);
        }
        Key(const Key&) = delete;
        Key(Key&&) = delete;
        auto operator=(const Key&) -> Key& = delete;
        auto operator=(Key&&) -> Key& = delete;

        ~Key()
        {
            if (nullptr != key_) {
                ::EVP_PKEY_free(key_);
                key_ = nullptr;
            }
        }
    };

    auto params = Key{};
    auto key = Key{};

    if (false == get_params(std::move(dhParams), options, params.key_)) {
        LogError()(OT_PRETTY_CLASS())("Failed to set up dh params").Flush();

        return false;
    }

    auto context = OpenSSL_EVP_PKEY_CTX{
        ::EVP_PKEY_CTX_new(params.key_, nullptr), ::EVP_PKEY_CTX_free};

    if (false == bool(context)) {
        LogError()(OT_PRETTY_CLASS())("Failed EVP_PKEY_CTX_new").Flush();

        return false;
    }

    if (1 != ::EVP_PKEY_keygen_init(context.get())) {
        LogError()(OT_PRETTY_CLASS())("Failed EVP_PKEY_keygen_init").Flush();

        return false;
    }

    if (1 != ::EVP_PKEY_keygen(context.get(), &key.key_)) {
        LogError()(OT_PRETTY_CLASS())("Failed EVP_PKEY_keygen").Flush();

        return false;
    }

    return write_keypair(std::move(privateKey), std::move(publicKey), key.key_);
}

auto OpenSSL::make_signing_key(
    Writer&& privateKey,
    Writer&& publicKey,
    const Parameters& options) const noexcept -> bool
{
    auto evp = OpenSSL_EVP_PKEY{::EVP_PKEY_new(), ::EVP_PKEY_free};

    OT_ASSERT(evp);

    {
        auto exponent = OpenSSL_BN{::BN_new(), ::BN_free};
        auto rsa = OpenSSL_RSA{::RSA_new(), ::RSA_free};

        OT_ASSERT(exponent);
        OT_ASSERT(rsa);

        auto rc = ::BN_set_word(exponent.get(), 65537);

        if (1 != rc) {
            LogError()(OT_PRETTY_CLASS())("Failed BN_set_word").Flush();

            return false;
        }

        rc = ::RSA_generate_multi_prime_key(
            rsa.get(),
            options.keySize(),
            primes(options.keySize()),
            exponent.get(),
            nullptr);

        if (1 != rc) {
            LogError()(OT_PRETTY_CLASS())("Failed to generate key").Flush();

            return false;
        }

        rc = ::EVP_PKEY_assign_RSA(evp.get(), rsa.release());

        if (1 != rc) {
            LogError()(OT_PRETTY_CLASS())("Failed EVP_PKEY_assign_RSA").Flush();

            return false;
        }
    }

    return write_keypair(
        std::move(privateKey), std::move(publicKey), evp.get());
}

auto OpenSSL::primes(const int bits) -> int
{
    if (8192 <= bits) {
        return 5;
    } else if (4096 <= bits) {
        return 4;
    } else if (1024 <= bits) {
        return 3;
    } else {
        return 2;
    }
}

auto OpenSSL::RandomKeypair(
    Writer&& privateKey,
    Writer&& publicKey,
    const crypto::asymmetric::Role role,
    const Parameters& options,
    Writer&& params) const noexcept -> bool
{
    if (crypto::asymmetric::Role::Encrypt == role) {

        return make_dh_key(
            std::move(privateKey),
            std::move(publicKey),
            std::move(params),
            options);
    } else {

        return make_signing_key(
            std::move(privateKey), std::move(publicKey), options);
    }
}

auto OpenSSL::SharedSecret(
    const ReadView pub,
    const ReadView prv,
    const SecretStyle style,
    Secret& secret) const noexcept -> bool
{
    if (SecretStyle::Default != style) {
        LogError()(OT_PRETTY_CLASS())("Unsupported secret style").Flush();

        return false;
    }

    auto dh = DH{};

    if (false == dh.init_keys(prv, pub)) { return false; }

    if (1 != ::EVP_PKEY_derive_init(dh)) {
        LogError()(OT_PRETTY_CLASS())("Failed EVP_PKEY_derive_init").Flush();

        return false;
    }

    if (1 != ::EVP_PKEY_derive_set_peer(dh, dh.Remote())) {
        LogVerbose()(OT_PRETTY_CLASS())("Failed EVP_PKEY_derive_set_peer")
            .Flush();

        return false;
    }

    auto size = 0_uz;

    if (1 != ::EVP_PKEY_derive(dh, nullptr, &size)) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate shared secret size")
            .Flush();

        return false;
    }

    auto output = secret.WriteInto(Secret::Mode::Mem).Reserve(size);

    if (false == output.IsValid(size)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to allocate space for shared secret")
            .Flush();

        return false;
    }

    if (1 != EVP_PKEY_derive(dh, output.as<unsigned char>(), &size)) {
        LogError()(OT_PRETTY_CLASS())("Failed to derive shared secret").Flush();

        return false;
    }

    return true;
}

auto OpenSSL::Sign(
    const ReadView in,
    const ReadView key,
    const crypto::HashType type,
    Writer&& signature) const -> bool
{
    switch (type) {
        case crypto::HashType::Blake2b160:
        case crypto::HashType::Blake2b256:
        case crypto::HashType::Blake2b512: {
            LogVerbose()(OT_PRETTY_CLASS())("Unsupported hash algorithm.")
                .Flush();

            return false;
        }
        default: {
        }
    }

    auto md = MD{};

    if (false == md.init_sign(type, key)) { return false; }

    if (1 != EVP_DigestSignInit(md, nullptr, md, nullptr, md)) {
        LogError()(OT_PRETTY_CLASS())("Failed to initialize signing operation")
            .Flush();

        return false;
    }

    if (1 != EVP_DigestSignUpdate(md, in.data(), in.size())) {
        LogError()(OT_PRETTY_CLASS())("Failed to process plaintext").Flush();

        return false;
    }

    auto bytes = 0_uz;

    if (1 != EVP_DigestSignFinal(md, nullptr, &bytes)) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate signature size")
            .Flush();

        return false;
    }

    auto out = signature.Reserve(bytes);

    if (false == out.IsValid(bytes)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate space for signature")
            .Flush();

        return false;
    }

    if (1 != EVP_DigestSignFinal(
                 md, reinterpret_cast<unsigned char*>(out.data()), &bytes)) {
        LogError()(OT_PRETTY_CLASS())("Failed to write signature").Flush();

        return false;
    }

    return true;
}

auto OpenSSL::Verify(
    const ReadView in,
    const ReadView key,
    const ReadView sig,
    const crypto::HashType type) const -> bool
{
    switch (type) {
        case crypto::HashType::Blake2b160:
        case crypto::HashType::Blake2b256:
        case crypto::HashType::Blake2b512: {
            LogVerbose()(OT_PRETTY_CLASS())("Unsupported hash algorithm.")
                .Flush();

            return false;
        }
        default: {
        }
    }

    auto md = MD{};

    if (false == md.init_verify(type, key)) { return false; }

    if (1 != EVP_DigestVerifyInit(md, nullptr, md, nullptr, md)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to initialize verification operation")
            .Flush();

        return false;
    }

    if (1 != EVP_DigestVerifyUpdate(md, in.data(), in.size())) {
        LogError()(OT_PRETTY_CLASS())("Failed to process plaintext").Flush();

        return false;
    }

    if (1 != EVP_DigestVerifyFinal(
                 md,
                 reinterpret_cast<const unsigned char*>(sig.data()),
                 sig.size())) {
        LogVerbose()(OT_PRETTY_CLASS())("Invalid signature").Flush();

        return false;
    }

    return true;
}

auto OpenSSL::write_dh(Writer&& dhParams, ::EVP_PKEY* input) const noexcept
    -> bool
{
    auto dh = OpenSSL_DH{::EVP_PKEY_get1_DH(input), ::DH_free};

    if (false == bool(dh)) {
        LogError()(OT_PRETTY_CLASS())("Failed EVP_PKEY_get1_DH").Flush();

        return false;
    }

    auto params = BIO{};
    const auto rc = ::PEM_write_bio_DHparams(params, dh.get());

    if (1 != rc) {
        LogError()(OT_PRETTY_CLASS())("Failed PEM_write_DHparams").Flush();

        return false;
    }

    if (false == params.Export(std::move(dhParams))) {
        LogError()(OT_PRETTY_CLASS())("Failed write dh params").Flush();

        return false;
    }

    return true;
}

auto OpenSSL::write_keypair(
    Writer&& privateKey,
    Writer&& publicKey,
    ::EVP_PKEY* evp) const noexcept -> bool
{
    OT_ASSERT(nullptr != evp);

    auto privKey = BIO{};
    auto pubKey = BIO{};

    auto rc = ::PEM_write_bio_PrivateKey(
        privKey, evp, nullptr, nullptr, 0, nullptr, nullptr);

    if (1 != rc) {
        LogError()(OT_PRETTY_CLASS())("Failed PEM_write_bio_PrivateKey")
            .Flush();

        return false;
    }

    rc = ::PEM_write_bio_PUBKEY(pubKey, evp);

    if (1 != rc) {
        LogError()(OT_PRETTY_CLASS())("Failed PEM_write_bio_PUBKEY").Flush();

        return false;
    }

    if (false == pubKey.Export(std::move(publicKey))) {
        LogError()(OT_PRETTY_CLASS())("Failed write public key").Flush();

        return false;
    }

    if (false == privKey.Export(std::move(privateKey))) {
        LogError()(OT_PRETTY_CLASS())("Failed write private key").Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::crypto::implementation
