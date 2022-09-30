// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "api/crypto/Hash.hpp"  // IWYU pragma: associated

#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string_view>

#include "internal/api/crypto/Factory.hpp"
#include "internal/crypto/library/Pbkdf2.hpp"
#include "internal/crypto/library/Ripemd160.hpp"
#include "internal/crypto/library/Scrypt.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/library/HashingProvider.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
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

auto Hash::bitcoin_hash_160(
    const ReadView data,
    const AllocateOutput destination) const noexcept -> bool
{
    try {
        auto temp = Space{};
        const auto rc =
            Digest(opentxs::crypto::HashType::Sha256, data, writer(temp));

        if (false == rc) {

            throw std::runtime_error{"failed to calculate intermediate hash"};
        }

        return Digest(
            opentxs::crypto::HashType::Ripemd160, reader(temp), destination);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Hash::Digest(
    const opentxs::crypto::HashType type,
    const ReadView data,
    const AllocateOutput destination) const noexcept -> bool
{
    using Type = opentxs::crypto::HashType;

    switch (type) {
        case Type::Sha1:
        case Type::Sha256:
        case Type::Sha512: {

            return sha_.Digest(type, data, destination);
        }
        case Type::Blake2b160:
        case Type::Blake2b256:
        case Type::Blake2b512: {

            return blake_.Digest(type, data, destination);
        }
        case Type::Ripemd160: {

            return ripe_.RIPEMD160(data, destination);
        }
        case Type::Sha256D: {

            return sha_256_double(data, destination);
        }
        case Type::Sha256DC: {

            return sha_256_double_checksum(data, destination);
        }
        case Type::Bitcoin: {

            return bitcoin_hash_160(data, destination);
        }
        case Type::Error:
        case Type::None:
        case Type::SipHash24:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported hash type.").Flush();

            return false;
        }
    }
}

auto Hash::Digest(
    const opentxs::crypto::HashType type,
    const opentxs::network::zeromq::Frame& data,
    const AllocateOutput destination) const noexcept -> bool
{
    return Digest(type, data.Bytes(), destination);
}

auto Hash::Digest(
    const std::uint32_t hash,
    const ReadView data,
    const AllocateOutput destination) const noexcept -> bool
{
    const auto type = static_cast<opentxs::crypto::HashType>(hash);
    auto temp = ByteArray{};

    try {
        if (false == Digest(type, data, temp.WriteInto())) {

            throw std::runtime_error{"failed to calculate hash"};
        }

        const auto encoded = encode_.IdentifierEncode(temp.Bytes());
        auto output = destination(encoded.size());

        if (false == output.valid(encoded.size())) {
            throw std::runtime_error{"unable to allocate encoded output space"};
        }

        std::memcpy(output, encoded.data(), output);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Hash::HMAC(
    const opentxs::crypto::HashType type,
    const ReadView key,
    const ReadView data,
    const AllocateOutput output) const noexcept -> bool
{
    using Type = opentxs::crypto::HashType;

    switch (type) {
        case Type::Sha256:
        case Type::Sha512: {

            return sha_.HMAC(type, key, data, output);
        }
        case Type::Blake2b160:
        case Type::Blake2b256:
        case Type::Blake2b512:
        case Type::SipHash24: {

            return blake_.HMAC(type, key, data, output);
        }
        case Type::Sha1:
        case Type::Ripemd160:
        case Type::Sha256D:
        case Type::Sha256DC:
        case Type::Bitcoin:
        case Type::Error:
        case Type::None:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unsupported hash type.").Flush();

            return false;
        }
    }
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

auto Hash::sha_256_double(const ReadView data, const AllocateOutput destination)
    const noexcept -> bool
{
    try {
        auto temp = Space{};
        const auto rc =
            Digest(opentxs::crypto::HashType::Sha256, data, writer(temp));

        if (false == rc) {

            throw std::runtime_error{"failed to calculate intermediate hash"};
        }

        return Digest(
            opentxs::crypto::HashType::Sha256, reader(temp), destination);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Hash::sha_256_double_checksum(
    const ReadView data,
    const AllocateOutput destination) const noexcept -> bool
{
    try {
        auto temp = Space{};
        const auto rc = sha_256_double(data, writer(temp));

        if (false == rc) {

            throw std::runtime_error{"failed to calculate intermediate hash"};
        }

        if (false == destination.operator bool()) {
            throw std::runtime_error{"invalid output"};
        }

        static constexpr auto size = 4_uz;
        auto buf = destination(size);

        if (false == buf.valid(size)) {
            throw std::runtime_error{"failed to allocate space for output"};
        }

        std::memcpy(buf.data(), temp.data(), size);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::api::crypto::imp
