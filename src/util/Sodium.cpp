// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"     // IWYU pragma: associated
#include "util/Sodium.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <algorithm>
#include <cassert>
#include <cstring>

#include "internal/util/P0330.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::sodium
{
auto ExpandSeed(
    const ReadView seed,
    Writer&& privateKey,
    Writer&& publicKey) noexcept -> bool
{
    assert(-1 != ::sodium_init());

    if ((nullptr == seed.data()) || (0 == seed.size())) {
        LogError()(__func__)(": Invalid provided seed").Flush();

        return false;
    }

    auto hashed = ByteArray{};
    auto nSeed{seed};

    if (crypto_sign_SEEDBYTES != seed.size()) {
        auto allocator = hashed.WriteInto();
        auto output = allocator.Reserve(crypto_sign_SEEDBYTES);

        if (false == output.IsValid(crypto_sign_SEEDBYTES)) {
            LogError()(__func__)(": Failed to allocate space for hashed seed")
                .Flush();

            return false;
        }

        if (0 != ::crypto_generichash(
                     output.as<unsigned char>(),
                     output,
                     reinterpret_cast<const unsigned char*>(seed.data()),
                     seed.size(),
                     nullptr,
                     0)) {
            LogError()(__func__)(": Failed to normalize seed").Flush();

            return false;
        }

        nSeed = hashed.Bytes();
    }

    if ((nullptr == nSeed.data()) || (crypto_sign_SEEDBYTES != nSeed.size())) {
        LogError()(__func__)(": Invalid normalized seed").Flush();

        return false;
    }

    auto prv = privateKey.Reserve(crypto_sign_SECRETKEYBYTES);
    auto pub = publicKey.Reserve(crypto_sign_PUBLICKEYBYTES);

    if (false == prv.IsValid(crypto_sign_SECRETKEYBYTES)) {
        LogError()(__func__)(": Failed to allocate space for private key")
            .Flush();

        return false;
    }

    if (false == pub.IsValid(crypto_sign_PUBLICKEYBYTES)) {
        LogError()(__func__)(": Failed to allocate space for public key")
            .Flush();

        return false;
    }

    return 0 == ::crypto_sign_seed_keypair(
                    pub.as<unsigned char>(),
                    prv.as<unsigned char>(),
                    reinterpret_cast<const unsigned char*>(nSeed.data()));
}

auto MakeSiphashKey(const ReadView data) noexcept -> SiphashKey
{
    auto out = SiphashKey{};

    static_assert(sizeof(out) == crypto_shorthash_KEYBYTES);

    if (valid(data)) {
        std::memcpy(out.data(), data.data(), std::min(data.size(), out.size()));
    }

    return out;
}

auto Randomize(WriteBuffer buffer) noexcept -> bool
{
    assert(-1 != ::sodium_init());

    if (0_uz < buffer.size()) {
        ::randombytes_buf(buffer.data(), buffer.size());
    }

    return true;
}

auto Siphash(const SiphashKey& key, const ReadView data) noexcept -> std::size_t
{
    assert(-1 != ::sodium_init());

    auto output = 0_uz;
    auto hash = std::array<unsigned char, crypto_shorthash_BYTES>{};
    ::crypto_shorthash(
        hash.data(),
        reinterpret_cast<const unsigned char*>(data.data()),
        data.size(),
        key.data());
    std::memcpy(&output, hash.data(), std::min(sizeof(output), hash.size()));

    return output;
}

auto ToCurveKeypair(
    const ReadView edPrivate,
    const ReadView edPublic,
    Writer&& curvePrivate,
    Writer&& curvePublic) noexcept -> bool
{
    assert(-1 != ::sodium_init());

    if (nullptr == edPrivate.data() ||
        crypto_sign_SECRETKEYBYTES != edPrivate.size()) {
        LogError()(__func__)(": Invalid ed25519 private key").Flush();

        return false;
    }

    if (nullptr == edPublic.data() ||
        crypto_sign_PUBLICKEYBYTES != edPublic.size()) {
        LogError()(__func__)(": Invalid ed25519 public key").Flush();

        return false;
    }

    auto prv = curvePrivate.Reserve(crypto_scalarmult_curve25519_BYTES);
    auto pub = curvePublic.Reserve(crypto_scalarmult_curve25519_BYTES);

    if (false == prv.IsValid(crypto_scalarmult_curve25519_BYTES)) {
        LogError()(__func__)(": Failed to allocate space for private key")
            .Flush();

        return false;
    }

    if (false == pub.IsValid(crypto_scalarmult_curve25519_BYTES)) {
        LogError()(__func__)(": Failed to allocate space for public key")
            .Flush();

        return false;
    }

    if (0 != ::crypto_sign_ed25519_sk_to_curve25519(
                 prv.as<unsigned char>(),
                 reinterpret_cast<const unsigned char*>(edPrivate.data()))) {
        LogError()(__func__)(
            ": Failed to convert private key from ed25519 to curve25519.")
            .Flush();

        return false;
    }

    if (0 != ::crypto_sign_ed25519_pk_to_curve25519(
                 pub.as<unsigned char>(),
                 reinterpret_cast<const unsigned char*>(edPublic.data()))) {
        LogError()(__func__)(
            ": Failed to convert public key from ed25519 to curve25519.")
            .Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::crypto::sodium
