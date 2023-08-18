// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

#include "internal/api/crypto/Hash.hpp"
#include "opentxs/crypto/Hasher.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace crypto
{
class Dash;
class HashingProvider;
class Pbkdf2;
class Ripemd160;
class Scrypt;
}  // namespace crypto

namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network

class Data;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::crypto::imp
{
class Hash final : public internal::Hash
{
public:
    auto Digest(
        const opentxs::crypto::HashType hashType,
        const ReadView data,
        Writer&& destination) const noexcept -> bool final;
    auto Digest(
        const opentxs::crypto::HashType hashType,
        const opentxs::network::zeromq::Frame& data,
        Writer&& destination) const noexcept -> bool final;
    auto HMAC(
        const opentxs::crypto::HashType type,
        const ReadView key,
        const ReadView data,
        Writer&& output) const noexcept -> bool final;
    auto Hasher(const opentxs::crypto::HashType hashType) const noexcept
        -> opentxs::crypto::Hasher final;
    auto MurmurHash3_32(
        const std::uint32_t& key,
        const Data& data,
        std::uint32_t& output) const noexcept -> void final;
    auto PKCS5_PBKDF2_HMAC(
        ReadView input,
        ReadView salt,
        std::size_t iterations,
        opentxs::crypto::HashType hashType,
        std::size_t bytes,
        Writer&& output) const noexcept -> bool final;
    auto Scrypt(
        const ReadView input,
        const ReadView salt,
        const std::uint64_t N,
        const std::uint32_t r,
        const std::uint32_t p,
        const std::size_t bytes,
        Writer&& writer) const noexcept -> bool final;

    Hash(
        const opentxs::crypto::HashingProvider& sha,
        const opentxs::crypto::HashingProvider& blake,
        const opentxs::crypto::Pbkdf2& pbkdf2,
        const opentxs::crypto::Ripemd160& ripe,
        const opentxs::crypto::Scrypt& scrypt) noexcept;
    Hash(const Hash&) = delete;
    Hash(Hash&&) = delete;
    auto operator=(const Hash&) -> Hash& = delete;
    auto operator=(Hash&&) -> Hash& = delete;

    ~Hash() final;

private:
    const opentxs::crypto::HashingProvider& sha_;
    const opentxs::crypto::HashingProvider& blake_;
    const opentxs::crypto::Pbkdf2& pbkdf2_;
    const opentxs::crypto::Ripemd160& ripe_;
    const opentxs::crypto::Scrypt& scrypt_;
    const std::unique_ptr<opentxs::crypto::Dash> dash_;

    auto bitcoin_hash_160(const ReadView data, Writer&& destination)
        const noexcept -> bool;
    auto sha_256_double(const ReadView data, Writer&& destination)
        const noexcept -> bool;
    auto sha_256_double_checksum(const ReadView data, Writer&& destination)
        const noexcept -> bool;
};
}  // namespace opentxs::api::crypto::imp
