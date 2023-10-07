// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::crypto::Hash

#include "api/crypto/Hash.hpp"  // IWYU pragma: associated

#include <smhasher/src/MurmurHash3.h>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/api/crypto/Factory.hpp"
#include "internal/crypto/library/Dash.hpp"
#include "internal/crypto/library/Factory.hpp"
#include "internal/crypto/library/HashingProvider.hpp"
#include "internal/crypto/library/Pbkdf2.hpp"
#include "internal/crypto/library/Ripemd160.hpp"
#include "internal/crypto/library/Scrypt.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/HashType.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Hasher.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto Hash(
    const crypto::HashingProvider& sha,
    const crypto::HashingProvider& blake,
    const crypto::Pbkdf2& pbkdf2,
    const crypto::Ripemd160& ripe,
    const crypto::Scrypt& scrypt) noexcept -> std::unique_ptr<api::crypto::Hash>
{
    using ReturnType = api::crypto::imp::Hash;

    return std::make_unique<ReturnType>(sha, blake, pbkdf2, ripe, scrypt);
}
}  // namespace opentxs::factory

namespace opentxs::api::crypto::imp
{
using Provider = opentxs::crypto::HashingProvider;

Hash::Hash(
    const Provider& sha,
    const Provider& blake,
    const opentxs::crypto::Pbkdf2& pbkdf2,
    const opentxs::crypto::Ripemd160& ripe,
    const opentxs::crypto::Scrypt& scrypt) noexcept
    : sha_(sha)
    , blake_(blake)
    , pbkdf2_(pbkdf2)
    , ripe_(ripe)
    , scrypt_(scrypt)
    , dash_(factory::Dash())
{
    assert_false(nullptr == dash_);
}

auto Hash::bitcoin_hash_160(const ReadView data, Writer&& destination)
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
            opentxs::crypto::HashType::Ripemd160,
            reader(temp),
            std::move(destination));
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto Hash::Digest(
    const opentxs::crypto::HashType type,
    const ReadView data,
    Writer&& destination) const noexcept -> bool
{
    using enum opentxs::crypto::HashType;

    switch (type) {
        case Sha1:
        case Sha256:
        case Sha512: {

            return sha_.Digest(type, data, std::move(destination));
        }
        case Blake2b160:
        case Blake2b256:
        case Blake2b512: {

            return blake_.Digest(type, data, std::move(destination));
        }
        case Ripemd160: {

            return ripe_.RIPEMD160(data, std::move(destination));
        }
        case Sha256D: {

            return sha_256_double(data, std::move(destination));
        }
        case Sha256DC: {

            return sha_256_double_checksum(data, std::move(destination));
        }
        case Bitcoin: {

            return bitcoin_hash_160(data, std::move(destination));
        }
        case X11: {

            return dash_->Digest(type, data, std::move(destination));
        }
        case Error:
        case None:
        case SipHash24:
        default: {
            LogError()()("Unsupported hash type.").Flush();

            return false;
        }
    }
}

auto Hash::Digest(
    const opentxs::crypto::HashType type,
    const opentxs::network::zeromq::Frame& data,
    Writer&& destination) const noexcept -> bool
{
    return Digest(type, data.Bytes(), std::move(destination));
}

auto Hash::HMAC(
    const opentxs::crypto::HashType type,
    const ReadView key,
    const ReadView data,
    Writer&& output) const noexcept -> bool
{
    using enum opentxs::crypto::HashType;

    switch (type) {
        case Sha256:
        case Sha512: {

            return sha_.HMAC(type, key, data, std::move(output));
        }
        case Blake2b160:
        case Blake2b256:
        case Blake2b512:
        case SipHash24: {

            return blake_.HMAC(type, key, data, std::move(output));
        }
        case X11: {

            return dash_->HMAC(type, key, data, std::move(output));
        }
        case Sha1:
        case Ripemd160:
        case Sha256D:
        case Sha256DC:
        case Bitcoin:
        case Error:
        case None:
        default: {
            LogError()()("Unsupported hash type.").Flush();

            return false;
        }
    }
}

auto Hash::Hasher(const opentxs::crypto::HashType hashType) const noexcept
    -> opentxs::crypto::Hasher
{
    return sha_.Hasher(hashType);
}

auto Hash::MurmurHash3_32(
    const std::uint32_t& key,
    const Data& data,
    std::uint32_t& output) const noexcept -> void
{
    const auto size = data.size();

    assert_true(size <= std::numeric_limits<int>::max());

    MurmurHash3_x86_32(
        data.data(), static_cast<int>(data.size()), key, &output);
}

auto Hash::PKCS5_PBKDF2_HMAC(
    ReadView input,
    ReadView salt,
    std::size_t iterations,
    opentxs::crypto::HashType hashType,
    std::size_t bytes,
    Writer&& output) const noexcept -> bool
{
    try {
        auto buf = output.Reserve(bytes);

        if (false == buf.IsValid(bytes)) {
            throw std::runtime_error{"failed to reserve space for hash"};
        }

        return pbkdf2_.PKCS5_PBKDF2_HMAC(
            input.data(),
            input.size(),
            salt.data(),
            salt.size(),
            iterations,
            hashType,
            bytes,
            buf.data());
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto Hash::Scrypt(
    const ReadView input,
    const ReadView salt,
    const std::uint64_t N,
    const std::uint32_t r,
    const std::uint32_t p,
    const std::size_t bytes,
    Writer&& writer) const noexcept -> bool
{
    return scrypt_.Generate(input, salt, N, r, p, bytes, std::move(writer));
}

auto Hash::sha_256_double(const ReadView data, Writer&& destination)
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
            opentxs::crypto::HashType::Sha256,
            reader(temp),
            std::move(destination));
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto Hash::sha_256_double_checksum(const ReadView data, Writer&& destination)
    const noexcept -> bool
{
    try {
        auto temp = Space{};

        if (false == sha_256_double(data, writer(temp))) {

            throw std::runtime_error{"failed to calculate checksum hash"};
        }

        return copy(reader(temp), std::move(destination), 4_uz);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

Hash::~Hash() = default;
}  // namespace opentxs::api::crypto::imp
