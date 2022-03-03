// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "1_Internal.hpp"       // IWYU pragma: associated
#include "api/crypto/Hash.hpp"  // IWYU pragma: associated

#include <cstring>
#include <limits>
#include <memory>
#include <string_view>

#include "internal/api/crypto/Factory.hpp"
#include "internal/crypto/library/Pbkdf2.hpp"
#include "internal/crypto/library/Ripemd160.hpp"
#include "internal/crypto/library/Scrypt.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/library/HashingProvider.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "smhasher/src/MurmurHash3.h"

namespace opentxs::factory
{
auto Hash(
    const api::crypto::Encode& encode,
    const crypto::HashingProvider& sha,
    const crypto::HashingProvider& blake,
    const crypto::Pbkdf2& pbkdf2,
    const crypto::Ripemd160& ripe,
    const crypto::Scrypt& scrypt) noexcept -> std::unique_ptr<api::crypto::Hash>
{
    using ReturnType = api::crypto::imp::Hash;

    return std::make_unique<ReturnType>(
        encode, sha, blake, pbkdf2, ripe, scrypt);
}
}  // namespace opentxs::factory

namespace opentxs::api::crypto::imp
{
using Provider = opentxs::crypto::HashingProvider;

Hash::Hash(
    const api::crypto::Encode& encode,
    const Provider& sha,
    const Provider& blake,
    const opentxs::crypto::Pbkdf2& pbkdf2,
    const opentxs::crypto::Ripemd160& ripe,
    const opentxs::crypto::Scrypt& scrypt) noexcept
    : encode_(encode)
    , sha_(sha)
    , blake_(blake)
    , pbkdf2_(pbkdf2)
    , ripe_(ripe)
    , scrypt_(scrypt)
{
}

auto Hash::allocate(
    const opentxs::crypto::HashType type,
    const AllocateOutput destination) noexcept -> WritableView
{
    if (false == bool(destination)) { return {}; }

    return destination(Provider::HashSize(type));
}

auto Hash::bitcoin_hash_160(
    const void* input,
    const std::size_t size,
    void* output) const noexcept -> bool
{
    auto temp = space(Provider::HashSize(opentxs::crypto::HashType::Sha256));

    if (false ==
        digest(opentxs::crypto::HashType::Sha256, input, size, temp.data())) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate intermediate hash.")
            .Flush();

        return false;
    }

    return digest(
        opentxs::crypto::HashType::Ripemd160, temp.data(), temp.size(), output);
}

auto Hash::Digest(
    const opentxs::crypto::HashType type,
    const ReadView data,
    const AllocateOutput destination) const noexcept -> bool
{
    auto view = allocate(type, destination);

    if (false == view.valid()) {
        LogError()(OT_PRETTY_CLASS())("Unable to allocate output space.")
            .Flush();

        return false;
    }

    return digest(type, data.data(), data.size(), view);
}

auto Hash::Digest(
    const opentxs::crypto::HashType type,
    const opentxs::network::zeromq::Frame& data,
    const AllocateOutput destination) const noexcept -> bool
{
    auto view = allocate(type, destination);

    if (false == view.valid()) {
        LogError()(OT_PRETTY_CLASS())("Unable to allocate output space.")
            .Flush();

        return false;
    }

    return digest(type, data.data(), data.size(), view);
}

auto Hash::Digest(
    const std::uint32_t hash,
    const ReadView data,
    const AllocateOutput destination) const noexcept -> bool
{
    if (false == bool(destination)) {
        LogError()(OT_PRETTY_CLASS())("Invalid output allocator").Flush();

        return false;
    }

    const auto type = static_cast<opentxs::crypto::HashType>(hash);
    auto temp =
        Data::Factory();  // FIXME IdentifierEncode should accept ReadView
    auto view = allocate(type, temp->WriteInto());

    if (false == view.valid()) {
        LogError()(OT_PRETTY_CLASS())("Unable to allocate temp space").Flush();

        return false;
    }

    if (digest(type, data.data(), data.size(), view)) {
        const auto encoded = encode_.IdentifierEncode(temp);
        auto output = destination(encoded.size());

        if (false == output.valid(encoded.size())) {
            LogError()(OT_PRETTY_CLASS())(
                "Unable to allocate encoded output space")
                .Flush();

            return false;
        }

        std::memcpy(output, encoded.data(), output);

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate digest").Flush();

        return false;
    }
}

auto Hash::digest(
    const opentxs::crypto::HashType type,
    const void* input,
    const std::size_t size,
    void* output) const noexcept -> bool
{
    switch (type) {
        case opentxs::crypto::HashType::Sha1:
        case opentxs::crypto::HashType::Sha256:
        case opentxs::crypto::HashType::Sha512: {
            return sha_.Digest(
                type,
                static_cast<const std::uint8_t*>(input),
                size,
                static_cast<std::uint8_t*>(output));
        }
        case opentxs::crypto::HashType::Blake2b160:
        case opentxs::crypto::HashType::Blake2b256:
        case opentxs::crypto::HashType::Blake2b512: {
            return blake_.Digest(
                type,
                static_cast<const std::uint8_t*>(input),
                size,
                static_cast<std::uint8_t*>(output));
        }
        case opentxs::crypto::HashType::Ripemd160: {
            return ripe_.RIPEMD160(
                static_cast<const std::uint8_t*>(input),
                size,
                static_cast<std::uint8_t*>(output));
        }
        case opentxs::crypto::HashType::Sha256D: {
            return sha_256_double(input, size, output);
        }
        case opentxs::crypto::HashType::Sha256DC: {
            return sha_256_double_checksum(input, size, output);
        }
        case opentxs::crypto::HashType::Bitcoin: {
            return bitcoin_hash_160(input, size, output);
        }
        default: {
        }
    }

    LogError()(OT_PRETTY_CLASS())("Unsupported hash type.").Flush();

    return false;
}

auto Hash::HMAC(
    const opentxs::crypto::HashType type,
    const ReadView key,
    const ReadView& data,
    const AllocateOutput digest) const noexcept -> bool
{
    auto output = allocate(type, digest);

    if (false == output.valid()) {
        LogError()(OT_PRETTY_CLASS())("Unable to allocate output space.")
            .Flush();

        return false;
    }

    return HMAC(
        type,
        reinterpret_cast<const std::uint8_t*>(data.data()),
        data.size(),
        reinterpret_cast<const std::uint8_t*>(key.data()),
        key.size(),
        output.as<std::uint8_t>());
}

auto Hash::HMAC(
    const opentxs::crypto::HashType type,
    const std::uint8_t* input,
    const std::size_t size,
    const std::uint8_t* key,
    const std::size_t keySize,
    std::uint8_t* output) const noexcept -> bool
{
    switch (type) {
        case opentxs::crypto::HashType::Sha256:
        case opentxs::crypto::HashType::Sha512: {
            return sha_.HMAC(type, input, size, key, keySize, output);
        }
        case opentxs::crypto::HashType::Blake2b160:
        case opentxs::crypto::HashType::Blake2b256:
        case opentxs::crypto::HashType::Blake2b512:
        case opentxs::crypto::HashType::SipHash24: {
            return blake_.HMAC(type, input, size, key, keySize, output);
        }
        default: {
        }
    }

    LogError()(OT_PRETTY_CLASS())("Unsupported hash type.").Flush();

    return false;
}

auto Hash::MurmurHash3_32(
    const std::uint32_t& key,
    const Data& data,
    std::uint32_t& output) const noexcept -> void
{
    const auto size = data.size();

    OT_ASSERT(size <= std::numeric_limits<int>::max());

    MurmurHash3_x86_32(
        data.data(), static_cast<int>(data.size()), key, &output);
}

auto Hash::PKCS5_PBKDF2_HMAC(
    const Data& input,
    const Data& salt,
    const std::size_t iterations,
    const opentxs::crypto::HashType type,
    const std::size_t bytes,
    Data& output) const noexcept -> bool
{
    output.SetSize(bytes);

    return pbkdf2_.PKCS5_PBKDF2_HMAC(
        input.data(),
        input.size(),
        salt.data(),
        salt.size(),
        iterations,
        type,
        bytes,
        output.data());
}

auto Hash::PKCS5_PBKDF2_HMAC(
    const Secret& input,
    const Data& salt,
    const std::size_t iterations,
    const opentxs::crypto::HashType type,
    const std::size_t bytes,
    Data& output) const noexcept -> bool
{
    output.SetSize(bytes);
    const auto data = input.Bytes();

    return pbkdf2_.PKCS5_PBKDF2_HMAC(
        data.data(),
        data.size(),
        salt.data(),
        salt.size(),
        iterations,
        type,
        bytes,
        output.data());
}

auto Hash::PKCS5_PBKDF2_HMAC(
    const UnallocatedCString& input,
    const Data& salt,
    const std::size_t iterations,
    const opentxs::crypto::HashType type,
    const std::size_t bytes,
    Data& output) const noexcept -> bool
{
    output.SetSize(bytes);

    return pbkdf2_.PKCS5_PBKDF2_HMAC(
        input.data(),
        input.size(),
        salt.data(),
        salt.size(),
        iterations,
        type,
        bytes,
        output.data());
}

auto Hash::Scrypt(
    const ReadView input,
    const ReadView salt,
    const std::uint64_t N,
    const std::uint32_t r,
    const std::uint32_t p,
    const std::size_t bytes,
    AllocateOutput writer) const noexcept -> bool
{
    return scrypt_.Generate(input, salt, N, r, p, bytes, writer);
}

auto Hash::sha_256_double(
    const void* input,
    const std::size_t size,
    void* output) const noexcept -> bool
{
    auto temp = space(Provider::HashSize(opentxs::crypto::HashType::Sha256));

    if (false ==
        digest(opentxs::crypto::HashType::Sha256, input, size, temp.data())) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate intermediate hash.")
            .Flush();

        return false;
    }

    return digest(
        opentxs::crypto::HashType::Sha256, temp.data(), temp.size(), output);
}

auto Hash::sha_256_double_checksum(
    const void* input,
    const std::size_t size,
    void* output) const noexcept -> bool
{
    auto temp = space(Provider::HashSize(opentxs::crypto::HashType::Sha256));

    if (false == sha_256_double(input, size, temp.data())) {
        LogError()(OT_PRETTY_CLASS())("Failed to calculate intermediate hash.")
            .Flush();

        return false;
    }

    std::memcpy(output, temp.data(), 4);

    return true;
}
}  // namespace opentxs::api::crypto::imp
