// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "crypto/library/sodium/Sodium.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <Ciphertext.pb.h>
#include <Enums.pb.h>
#include <SHA1/sha1.hpp>
#include <argon2.h>
#include <array>
#include <cstring>
#include <memory>
#include <stdexcept>

#include "crypto/library/AsymmetricProvider.hpp"
#include "crypto/library/EcdsaProvider.hpp"
#include "internal/crypto/key/Key.hpp"
#include "internal/crypto/library/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/symmetric/Algorithm.hpp"
#include "opentxs/crypto/symmetric/Source.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto Sodium(const api::Crypto& crypto) noexcept
    -> std::unique_ptr<crypto::Sodium>
{
    using ReturnType = crypto::implementation::Sodium;

    return std::make_unique<ReturnType>(crypto);
}
}  // namespace opentxs::factory

namespace opentxs::crypto::implementation
{
Sodium::Sodium(const api::Crypto& crypto) noexcept
    : AsymmetricProvider()
    , EcdsaProvider(crypto)
{
    const auto result = ::sodium_init();

    OT_ASSERT(-1 != result);
}

auto Sodium::blank_private() noexcept -> ReadView
{
    static const auto blank = space(crypto_sign_SECRETKEYBYTES);

    return reader(blank);
}

auto Sodium::Decrypt(
    const proto::Ciphertext& ciphertext,
    const std::uint8_t* key,
    const std::size_t keySize,
    std::uint8_t* plaintext) const -> bool
{
    const auto& message = ciphertext.data();
    const auto& nonce = ciphertext.iv();
    const auto& mac = ciphertext.tag();
    const auto& mode = ciphertext.mode();

    if (KeySize(translate(mode)) != keySize) {
        LogError()(OT_PRETTY_CLASS())("Incorrect key size.").Flush();

        return false;
    }

    if (IvSize(translate(mode)) != nonce.size()) {
        LogError()(OT_PRETTY_CLASS())("Incorrect nonce size.").Flush();

        return false;
    }

    switch (ciphertext.mode()) {
        case proto::SMODE_CHACHA20POLY1305: {
            return (
                0 == crypto_aead_chacha20poly1305_ietf_decrypt_detached(
                         plaintext,
                         nullptr,
                         reinterpret_cast<const unsigned char*>(message.data()),
                         message.size(),
                         reinterpret_cast<const unsigned char*>(mac.data()),
                         nullptr,
                         0,
                         reinterpret_cast<const unsigned char*>(nonce.data()),
                         key));
        }
        case proto::SMODE_ERROR:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported encryption mode (")(
                mode)(").")
                .Flush();
        }
    }

    return false;
}

auto Sodium::DefaultMode() const -> opentxs::crypto::symmetric::Algorithm
{
    return opentxs::crypto::symmetric::Algorithm::ChaCha20Poly1305;
}

auto Sodium::Derive(
    const std::uint8_t* input,
    const std::size_t inputSize,
    const std::uint8_t* salt,
    const std::size_t saltSize,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::uint64_t parallel,
    const symmetric::Source type,
    std::uint8_t* output,
    std::size_t outputSize) const -> bool
{
    try {
        const auto minOps = [&] {
            using Type = symmetric::Source;

            switch (type) {
                case Type::Argon2i: {

                    return crypto_pwhash_argon2i_OPSLIMIT_MIN;
                }
                case Type::Argon2id: {

                    return crypto_pwhash_argon2id_OPSLIMIT_MIN;
                }
                case Type::Error:
                case Type::Raw:
                case Type::ECDH:
                default: {

                    throw std::runtime_error{"unsupported algorithm"};
                }
            }
        }();
        const auto requiredSize = SaltSize(type);

        if (requiredSize != saltSize) {
            const auto error = UnallocatedCString{"Incorrect salt size ("} +
                               std::to_string(saltSize) + "). Required: (" +
                               std::to_string(requiredSize);

            throw std::runtime_error{error};
        }

        OT_ASSERT(nullptr != salt);

        if (outputSize < crypto_pwhash_BYTES_MIN) {
            throw std::runtime_error{"output too small"};
        }

        if (outputSize > crypto_pwhash_BYTES_MAX) {
            throw std::runtime_error{"output too large"};
        }

        if (inputSize > crypto_pwhash_PASSWD_MAX) {
            throw std::runtime_error{"input too large"};
        }

        if (operations < minOps) {
            throw std::runtime_error{"operations too low"};
        }

        if (operations > crypto_pwhash_OPSLIMIT_MAX) {
            throw std::runtime_error{"operations too high"};
        }

        if (difficulty < crypto_pwhash_MEMLIMIT_MIN) {
            throw std::runtime_error{"memory too low"};
        }

        if (difficulty > crypto_pwhash_MEMLIMIT_MAX) {
            throw std::runtime_error{"memory too high"};
        }

        if (parallel > ARGON2_MAX_THREADS) {
            throw std::runtime_error{"too many threads"};
        }

        static const auto blank = char{};
        const auto empty = ((nullptr == input) || (0u == inputSize));
        const auto* ptr = empty ? &blank : reinterpret_cast<const char*>(input);
        const auto effective = empty ? 0_uz : inputSize;

        if (1u < parallel) {
            const auto rc = [&] {
                switch (type) {
                    case symmetric::Source::Argon2i: {

                        return ::argon2i_hash_raw_fucklibsodium(
                            static_cast<std::uint32_t>(operations),
                            static_cast<std::uint32_t>(difficulty >> 10),
                            static_cast<std::uint32_t>(parallel),
                            ptr,
                            effective,
                            salt,
                            saltSize,
                            output,
                            outputSize);
                    }
                    case symmetric::Source::Argon2id: {

                        return ::argon2id_hash_raw_fucklibsodium(
                            static_cast<std::uint32_t>(operations),
                            static_cast<std::uint32_t>(difficulty >> 10),
                            static_cast<std::uint32_t>(parallel),
                            ptr,
                            effective,
                            salt,
                            saltSize,
                            output,
                            outputSize);
                    }
                    case crypto::symmetric::Source::Error:
                    case crypto::symmetric::Source::Raw:
                    case crypto::symmetric::Source::ECDH:
                    default: {

                        throw std::runtime_error{"unsupported algorithm"};
                    }
                }
            }();

            if (ARGON2_OK != rc) {
                throw std::runtime_error{
                    ::argon2_error_message_fucklibsodium(rc)};
            }
        } else {
            const auto rc = [&] {
                switch (type) {
                    case symmetric::Source::Argon2i: {

                        return ::crypto_pwhash(
                            output,
                            outputSize,
                            ptr,
                            effective,
                            salt,
                            operations,
                            static_cast<std::size_t>(difficulty),
                            crypto_pwhash_ALG_ARGON2I13);
                    }
                    case symmetric::Source::Argon2id: {

                        return ::crypto_pwhash(
                            output,
                            outputSize,
                            ptr,
                            effective,
                            salt,
                            operations,
                            static_cast<std::size_t>(difficulty),
                            crypto_pwhash_ALG_ARGON2ID13);
                    }
                    case crypto::symmetric::Source::Error:
                    case crypto::symmetric::Source::Raw:
                    case crypto::symmetric::Source::ECDH:
                    default: {

                        throw std::runtime_error{"unsupported algorithm"};
                    }
                }
            }();

            if (0 != rc) { throw std::runtime_error{"failed to derive hash"}; }
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Sodium::Digest(
    const crypto::HashType hashType,
    const ReadView data,
    Writer&& output) const noexcept -> bool
{
    try {
        const auto size = HashSize(hashType);
        auto buf = output.Reserve(size);

        if (false == buf.IsValid(size)) {
            throw std::runtime_error{"failed to allocate space for output"};
        }

        switch (hashType) {
            case (crypto::HashType::Blake2b160):
            case (crypto::HashType::Blake2b256):
            case (crypto::HashType::Blake2b512): {

                return (
                    0 ==
                    ::crypto_generichash(
                        buf.as<unsigned char>(),
                        buf.size(),
                        reinterpret_cast<const unsigned char*>(data.data()),
                        data.size(),
                        nullptr,
                        0));
            }
            case (crypto::HashType::Sha256): {

                return (
                    0 ==
                    ::crypto_hash_sha256(
                        buf.as<unsigned char>(),
                        reinterpret_cast<const unsigned char*>(data.data()),
                        data.size()));
            }
            case (crypto::HashType::Sha512): {

                return (
                    0 ==
                    ::crypto_hash_sha512(
                        buf.as<unsigned char>(),
                        reinterpret_cast<const unsigned char*>(data.data()),
                        data.size()));
            }
            case (crypto::HashType::Sha1): {

                return sha1(data, buf);
            }
            default: {
                throw std::runtime_error{"unsupported hash type"};
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Sodium::Encrypt(
    const std::uint8_t* input,
    const std::size_t inputSize,
    const std::uint8_t* key,
    const std::size_t keySize,
    proto::Ciphertext& ciphertext) const -> bool
{
    OT_ASSERT(nullptr != input);
    OT_ASSERT(nullptr != key);

    const auto& mode = translate(ciphertext.mode());
    const auto& nonce = ciphertext.iv();
    auto& tag = *ciphertext.mutable_tag();
    auto& output = *ciphertext.mutable_data();

    bool result = false;

    if (mode == opentxs::crypto::symmetric::Algorithm::Error) {
        LogError()(OT_PRETTY_CLASS())("Incorrect mode.").Flush();

        return result;
    }

    if (KeySize(mode) != keySize) {
        LogError()(OT_PRETTY_CLASS())("Incorrect key size.").Flush();

        return result;
    }

    if (IvSize(mode) != nonce.size()) {
        LogError()(OT_PRETTY_CLASS())("Incorrect nonce size.").Flush();

        return result;
    }

    ciphertext.set_version(1);
    tag.resize(TagSize(mode), 0x0);
    output.resize(inputSize, 0x0);

    OT_ASSERT(false == nonce.empty());
    OT_ASSERT(false == tag.empty());

    using Type = opentxs::crypto::symmetric::Algorithm;

    switch (mode) {
        case Type::ChaCha20Poly1305: {
            return (
                0 == crypto_aead_chacha20poly1305_ietf_encrypt_detached(
                         reinterpret_cast<unsigned char*>(output.data()),
                         reinterpret_cast<unsigned char*>(tag.data()),
                         nullptr,
                         input,
                         inputSize,
                         nullptr,
                         0,
                         nullptr,
                         reinterpret_cast<const unsigned char*>(nonce.data()),
                         key));
        }
        case Type::Error:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported encryption mode (")(
                value(mode))(").")
                .Flush();
        }
    }

    return result;
}

auto Sodium::Generate(
    const ReadView input,
    const ReadView salt,
    const std::uint64_t N,
    const std::uint32_t r,
    const std::uint32_t p,
    const std::size_t bytes,
    Writer&& writer) const noexcept -> bool
{
    if (bytes < crypto_pwhash_scryptsalsa208sha256_BYTES_MIN) {
        LogError()(OT_PRETTY_CLASS())("Too few bytes requested: ")(
            bytes)(" vs "
                   "minimum:"
                   " ")(crypto_pwhash_scryptsalsa208sha256_BYTES_MIN)
            .Flush();

        return false;
    }

    if (bytes > crypto_pwhash_scryptsalsa208sha256_BYTES_MAX) {
        LogError()(OT_PRETTY_CLASS())("Too many bytes requested: ")(
            bytes)(" vs "
                   "maximum:"
                   " ")(crypto_pwhash_scryptsalsa208sha256_BYTES_MAX)
            .Flush();

        return false;
    }

    auto output = writer.Reserve(bytes);

    if (false == output.IsValid(bytes)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocated requested ")(
            bytes)(" bytes")
            .Flush();

        return false;
    }

    return 0 == ::crypto_pwhash_scryptsalsa208sha256_ll(
                    reinterpret_cast<const std::uint8_t*>(input.data()),
                    input.size(),
                    reinterpret_cast<const std::uint8_t*>(salt.data()),
                    salt.size(),
                    N,
                    r,
                    p,
                    output.as<std::uint8_t>(),
                    output.size());
}

auto Sodium::HMAC(
    const crypto::HashType hashType,
    const ReadView key,
    const ReadView data,
    Writer&& output) const noexcept -> bool
{
    try {
        if (false == valid(data)) { throw std::runtime_error{"invalid input"}; }

        if (false == valid(key)) { throw std::runtime_error{"invalid key"}; }

        const auto size = HashSize(hashType);
        auto buf = output.Reserve(size);

        if (false == buf.IsValid(size)) {
            throw std::runtime_error{"failed to allocate space for output"};
        }

        switch (hashType) {
            case (crypto::HashType::Blake2b160):
            case (crypto::HashType::Blake2b256):
            case (crypto::HashType::Blake2b512): {
                return (
                    0 ==
                    ::crypto_generichash(
                        buf.as<unsigned char>(),
                        buf.size(),
                        reinterpret_cast<const unsigned char*>(data.data()),
                        data.size(),
                        reinterpret_cast<const unsigned char*>(key.data()),
                        key.size()));
            }
            case (crypto::HashType::Sha256): {
                auto success{false};
                auto state = ::crypto_auth_hmacsha256_state{};
                success =
                    (0 ==
                     ::crypto_auth_hmacsha256_init(
                         &state,
                         reinterpret_cast<const unsigned char*>(key.data()),
                         key.size()));

                if (false == success) {
                    throw std::runtime_error{
                        "Failed to initialize sha256 context"};
                }

                success =
                    (0 ==
                     ::crypto_auth_hmacsha256_update(
                         &state,
                         reinterpret_cast<const unsigned char*>(data.data()),
                         data.size()));

                if (false == success) {
                    throw std::runtime_error{"Failed to update sha256 context"};
                }

                return (
                    0 == ::crypto_auth_hmacsha256_final(
                             &state, buf.as<unsigned char>()));
            }
            case (crypto::HashType::Sha512): {
                auto success{false};
                auto state = ::crypto_auth_hmacsha512_state{};
                success =
                    (0 ==
                     ::crypto_auth_hmacsha512_init(
                         &state,
                         reinterpret_cast<const unsigned char*>(key.data()),
                         key.size()));

                if (false == success) {
                    throw std::runtime_error{
                        "Failed to initialize sha512 context"};
                }

                success =
                    (0 ==
                     ::crypto_auth_hmacsha512_update(
                         &state,
                         reinterpret_cast<const unsigned char*>(data.data()),
                         data.size()));

                if (false == success) {
                    throw std::runtime_error{"Failed to update sha512 context"};
                }

                return (
                    0 == ::crypto_auth_hmacsha512_final(
                             &state, buf.as<unsigned char>()));
            }
            case (crypto::HashType::SipHash24): {
                auto temp = std::array<char, crypto_shorthash_KEYBYTES>{};
                const auto keyView = [&]() -> ReadView {
                    if (auto s = key.size(); crypto_shorthash_KEYBYTES < s) {
                        const auto error =
                            UnallocatedCString{"Excessive key size: "}
                                .append(std::to_string(s))
                                .append(" vs maximum ")
                                .append(
                                    std::to_string(crypto_shorthash_KEYBYTES));

                        throw std::runtime_error{error};
                    } else if (s == crypto_shorthash_KEYBYTES) {

                        return key;
                    } else {
                        std::memcpy(temp.data(), key.data(), key.size());

                        return {temp.data(), temp.size()};
                    }
                }();

                return 0 ==
                       ::crypto_shorthash(
                           buf.as<unsigned char>(),
                           reinterpret_cast<const unsigned char*>(data.data()),
                           data.size(),
                           reinterpret_cast<const unsigned char*>(
                               keyView.data()));
            }
            default: {
                throw std::runtime_error{"unsupported hash type"};
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Sodium::IvSize(const opentxs::crypto::symmetric::Algorithm mode) const
    -> std::size_t
{
    using Type = opentxs::crypto::symmetric::Algorithm;

    switch (mode) {
        case Type::ChaCha20Poly1305: {
            return crypto_aead_chacha20poly1305_IETF_NPUBBYTES;
        }
        case Type::Error:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported encryption mode (")(
                value(mode))(").")
                .Flush();
        }
    }
    return 0;
}

auto Sodium::KeySize(const opentxs::crypto::symmetric::Algorithm mode) const
    -> std::size_t
{
    using Type = opentxs::crypto::symmetric::Algorithm;

    switch (mode) {
        case Type::ChaCha20Poly1305: {
            return crypto_aead_chacha20poly1305_IETF_KEYBYTES;
        }
        case Type::Error:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported encryption mode (")(
                value(mode))(").")
                .Flush();
        }
    }
    return 0;
}

auto Sodium::RandomizeMemory(void* destination, const std::size_t size) const
    -> bool
{
    ::randombytes_buf(destination, size);

    return true;
}

auto Sodium::SaltSize(const crypto::symmetric::Source type) const -> std::size_t
{
    switch (type) {
        case crypto::symmetric::Source::Argon2i:
        case crypto::symmetric::Source::Argon2id: {

            return crypto_pwhash_SALTBYTES;
        }
        case crypto::symmetric::Source::Error:
        case crypto::symmetric::Source::Raw:
        case crypto::symmetric::Source::ECDH:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported key type (")(
                value(type))(").")
                .Flush();
        }
    }

    return 0;
}

auto Sodium::sha1(const ReadView data, WriteBuffer& output) const -> bool
{
    try {
        auto hex = std::array<char, SHA1_HEX_SIZE>{};
        ::sha1()
            .add(data.data(), shorten(data.size()))
            .finalize()
            .print_hex(hex.data());
        const auto hash = [&]() {
            auto out = ByteArray{};
            out.DecodeHex({hex.data(), hex.size()});
            return out;
        }();
        std::memcpy(output.data(), hash.data(), hash.size());

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Sodium::TagSize(const opentxs::crypto::symmetric::Algorithm mode) const
    -> std::size_t
{
    using Type = opentxs::crypto::symmetric::Algorithm;

    switch (mode) {
        case Type::ChaCha20Poly1305: {
            return crypto_aead_chacha20poly1305_IETF_ABYTES;
        }
        case Type::Error:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported encryption mode (")(
                value(mode))(").")
                .Flush();
        }
    }
    return 0;
}
}  // namespace opentxs::crypto::implementation
