// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/library/secp256k1/Secp256k1.hpp"  // IWYU pragma: associated

extern "C" {
#include <secp256k1.h>
#include <secp256k1_ecdh.h>
}

#include <array>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "crypto/library/EcdsaProvider.hpp"
#include "internal/crypto/library/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/SecretStyle.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

extern "C" {
auto get_x_value(
    unsigned char* output,
    const unsigned char* x32,
    const unsigned char* y32,
    void*) -> int;
auto get_x_value(
    unsigned char* output,
    const unsigned char* x32,
    const unsigned char* y32,
    void*) -> int
{
    std::memcpy(output, x32, 32);

    return 1;
}
}

namespace opentxs::factory
{
auto Secp256k1(
    const api::Crypto& crypto,
    const api::crypto::Util& util) noexcept
    -> std::unique_ptr<crypto::Secp256k1>
{
    using ReturnType = crypto::implementation::Secp256k1;

    return std::make_unique<ReturnType>(crypto, util);
}
}  // namespace opentxs::factory

namespace opentxs::crypto::implementation
{
bool Secp256k1::Initialized_ = false;

Secp256k1::Secp256k1(
    const api::Crypto& crypto,
    const api::crypto::Util& ssl) noexcept
    : EcdsaProvider(crypto)
    , context_(secp256k1_context_create(
          SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY))
    , ssl_(ssl)
{
}

auto Secp256k1::blank_private() noexcept -> ReadView
{
    static const auto blank = space(PrivateKeySize);

    return reader(blank);
}

auto Secp256k1::PubkeyAdd(ReadView pubkey, ReadView scalar, Writer&& result)
    const noexcept -> bool
{
    if ((0 == pubkey.size()) || (nullptr == pubkey.data())) {
        LogError()(OT_PRETTY_CLASS())("Missing pubkey").Flush();

        return false;
    }

    if ((PrivateKeySize != scalar.size()) || (nullptr == scalar.data())) {
        LogError()(OT_PRETTY_CLASS())("Invalid scalar").Flush();

        return false;
    }

    auto parsed = ::secp256k1_pubkey{};
    auto rc = 1 == ::secp256k1_ec_pubkey_parse(
                       context_,
                       &parsed,
                       reinterpret_cast<const unsigned char*>(pubkey.data()),
                       pubkey.size());

    if (false == rc) {
        LogError()(OT_PRETTY_CLASS())("Invalid public key").Flush();

        return false;
    }

    rc = 1 == ::secp256k1_ec_pubkey_tweak_add(
                  context_,
                  &parsed,
                  reinterpret_cast<const unsigned char*>(scalar.data()));

    if (false == rc) {
        LogError()(OT_PRETTY_CLASS())("Failed to add scalar to public key")
            .Flush();

        return false;
    }

    auto out = result.Reserve(PublicKeySize);

    if (false == out.IsValid(PublicKeySize)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate space for result")
            .Flush();

        return false;
    }

    auto size = out.size();
    rc = ::secp256k1_ec_pubkey_serialize(
        context_,
        out.as<unsigned char>(),
        &size,
        &parsed,
        SECP256K1_EC_COMPRESSED);

    if (false == rc) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize public key").Flush();

        return false;
    }

    return true;
}

auto Secp256k1::RandomKeypair(
    Writer&& privateKey,
    Writer&& publicKey,
    const opentxs::crypto::asymmetric::Role,
    const Parameters&,
    Writer&&) const noexcept -> bool
{
    if (nullptr == context_) { return false; }

    auto output = privateKey.Reserve(PrivateKeySize);

    if (false == output.IsValid(PrivateKeySize)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed to allocate space for private key")
            .Flush();

        return false;
    }

    auto counter{0};
    auto valid{false};

    while (false == valid) {
        crypto_.Util().RandomizeMemory(output.data(), output.size());
        valid = 1 == ::secp256k1_ec_seckey_verify(
                         context_, output.as<unsigned char>());

        OT_ASSERT(3 > ++counter);
    }

    return ScalarMultiplyBase(output, std::move(publicKey));
}

auto Secp256k1::ScalarAdd(ReadView lhs, ReadView rhs, Writer&& result)
    const noexcept -> bool
{
    if (PrivateKeySize != lhs.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid lhs scalar").Flush();

        return false;
    }

    if (PrivateKeySize != rhs.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid rhs scalar").Flush();

        return false;
    }

    auto key = result.Reserve(PrivateKeySize);

    if (false == key.IsValid(PrivateKeySize)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate space for result")
            .Flush();

        return false;
    }

    std::memcpy(key.data(), lhs.data(), lhs.size());

    return 1 == ::secp256k1_ec_seckey_tweak_add(
                    context_,
                    key.as<unsigned char>(),
                    reinterpret_cast<const unsigned char*>(rhs.data()));
}

auto Secp256k1::ScalarMultiplyBase(ReadView scalar, Writer&& result)
    const noexcept -> bool
{
    if (PrivateKeySize != scalar.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid scalar").Flush();

        return false;
    }

    auto key = secp256k1_pubkey{};
    const auto created =
        1 == ::secp256k1_ec_pubkey_create(
                 context_,
                 &key,
                 reinterpret_cast<const unsigned char*>(scalar.data()));

    if (1 != created) { return false; }

    auto pub = result.Reserve(PublicKeySize);

    if (false == pub.IsValid(PublicKeySize)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate space for public key")
            .Flush();

        return false;
    }

    auto size{pub.size()};

    return 1 == ::secp256k1_ec_pubkey_serialize(
                    context_,
                    pub.as<unsigned char>(),
                    &size,
                    &key,
                    SECP256K1_EC_COMPRESSED);
}

auto Secp256k1::SharedSecret(
    const ReadView pub,
    const ReadView prv,
    const SecretStyle style,
    Secret& secret) const noexcept -> bool
{
    if (PrivateKeySize != prv.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid private key").Flush();

        return false;
    }

    auto key = ::secp256k1_pubkey{};

    if (1 != ::secp256k1_ec_pubkey_parse(
                 context_,
                 &key,
                 reinterpret_cast<const unsigned char*>(pub.data()),
                 pub.size())) {
        LogError()(OT_PRETTY_CLASS())("Invalid public key").Flush();

        return false;
    }

    auto out = secret.WriteInto(Secret::Mode::Mem).Reserve(PrivateKeySize);

    OT_ASSERT(out.IsValid(PrivateKeySize));

    const auto function = [&] {
        switch (style) {
            case SecretStyle::X_only: {

                return get_x_value;
            }
            case SecretStyle::Default:
            default: {

                return secp256k1_ecdh_hash_function_sha256;
            }
        }
    }();

    return 1 == ::secp256k1_ecdh(
                    context_,
                    out.as<unsigned char>(),
                    &key,
                    reinterpret_cast<const unsigned char*>(prv.data()),
                    function,
                    nullptr);
}

auto Secp256k1::Sign(
    const ReadView plaintext,
    const ReadView priv,
    const crypto::HashType type,
    Writer&& signature) const -> bool
{
    try {
        const auto digest = hash(type, plaintext);

        if (nullptr == priv.data() || 0 == priv.size()) {
            LogError()(OT_PRETTY_CLASS())("Missing private key").Flush();

            return false;
        }

        if (PrivateKeySize != priv.size()) {
            LogError()(OT_PRETTY_CLASS())("Invalid private key").Flush();

            return false;
        }

        if (priv == blank_private()) {
            LogError()(OT_PRETTY_CLASS())("Blank private key").Flush();

            return false;
        }

        const auto size = sizeof(secp256k1_ecdsa_signature);
        auto output = signature.Reserve(size);

        if (false == output.IsValid(size)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to allocate space for signature")
                .Flush();

            return false;
        }

        const bool signatureCreated = ::secp256k1_ecdsa_sign(
            context_,
            static_cast<::secp256k1_ecdsa_signature*>(output.data()),
            reinterpret_cast<const unsigned char*>(digest.data()),
            reinterpret_cast<const unsigned char*>(priv.data()),
            nullptr,
            nullptr);

        if (false == signatureCreated) {
            LogError()(OT_PRETTY_CLASS())(
                "Call to secp256k1_ecdsa_sign() failed.")
                .Flush();

            return false;
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Secp256k1::SignDER(
    ReadView plaintext,
    ReadView priv,
    crypto::HashType type,
    Writer&& result) const noexcept -> bool
{
    try {
        const auto digest = hash(type, plaintext);

        if (false == valid(priv)) {

            throw std::runtime_error{"missing private key"};
        }

        if (PrivateKeySize != priv.size()) {

            throw std::runtime_error{"invalid private key"};
        }

        if (priv == blank_private()) {

            throw std::runtime_error{"blank private key"};
        }

        auto sig = secp256k1_ecdsa_signature{};

        const bool signatureCreated = ::secp256k1_ecdsa_sign(
            context_,
            &sig,
            reinterpret_cast<const unsigned char*>(digest.data()),
            reinterpret_cast<const unsigned char*>(priv.data()),
            nullptr,
            nullptr);

        if (false == signatureCreated) {

            throw std::runtime_error{"secp256k1_ecdsa_sign() failed"};
        }

        constexpr auto maxBytes = 80_uz;
        auto output = result.Reserve(maxBytes);

        if (false == output.IsValid(maxBytes)) {

            throw std::runtime_error{"failed to allocate space for signature"};
        }

        auto allocated{output.size()};
        const auto wrote = ::secp256k1_ecdsa_signature_serialize_der(
            context_,
            reinterpret_cast<unsigned char*>(output.data()),
            &allocated,
            &sig);

        if (1 != wrote) {

            throw std::runtime_error{
                "secp256k1_ecdsa_signature_serialize_der() failed"};
        }

        if (false == result.Truncate(allocated)) {

            throw std::runtime_error{
                "failed to truncate output buffer to final size"};
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Secp256k1::Verify(
    const ReadView plaintext,
    const ReadView key,
    const ReadView signature,
    const crypto::HashType type) const -> bool
{
    try {
        const auto digest = hash(type, plaintext);
        const auto parsed = parsed_public_key(key);
        const auto sig = parsed_signature(signature);

        return 1 == ::secp256k1_ecdsa_verify(
                        context_,
                        &sig,
                        reinterpret_cast<const unsigned char*>(digest.data()),
                        &parsed);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Secp256k1::hash(const crypto::HashType type, const ReadView data) const
    noexcept(false) -> ByteArray
{
    auto output = ByteArray{};

    if (false == crypto_.Hash().Digest(type, data, output.WriteInto())) {
        throw std::runtime_error("Failed to obtain contract hash");
    }

    if (0 == output.size()) { throw std::runtime_error("Invalid hash"); }

    output.resize(32);

    OT_ASSERT(nullptr != output.data());
    OT_ASSERT(32 == output.size());

    return output;
}

void Secp256k1::Init()
{
    OT_ASSERT(false == Initialized_);

    auto seed = std::array<std::uint8_t, 32>{};
    ssl_.RandomizeMemory(seed.data(), seed.size());

    OT_ASSERT(nullptr != context_);

    const auto randomize = secp256k1_context_randomize(context_, seed.data());

    OT_ASSERT(1 == randomize);

    Initialized_ = true;
}

auto Secp256k1::parsed_public_key(const ReadView bytes) const noexcept(false)
    -> ::secp256k1_pubkey
{
    if (nullptr == bytes.data() || 0 == bytes.size()) {
        throw std::runtime_error("Missing public key");
    }

    auto output = ::secp256k1_pubkey{};

    if (1 != ::secp256k1_ec_pubkey_parse(
                 context_,
                 &output,
                 reinterpret_cast<const unsigned char*>(bytes.data()),
                 bytes.size())) {
        throw std::runtime_error("Invalid public key");
    }

    return output;
}

auto Secp256k1::parsed_signature(const ReadView bytes) const noexcept(false)
    -> ::secp256k1_ecdsa_signature
{
    auto output = ::secp256k1_ecdsa_signature{};

    if (sizeof(output.data) != bytes.size()) {
        throw std::runtime_error("Invalid signature");
    }

    std::memcpy(&output.data, bytes.data(), sizeof(output.data));

    return output;
}

Secp256k1::~Secp256k1()
{
    if (nullptr != context_) {
        secp256k1_context_destroy(context_);
        context_ = nullptr;
    }

    Initialized_ = false;
}
}  // namespace opentxs::crypto::implementation
