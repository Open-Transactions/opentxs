// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "crypto/library/dash/Dash.hpp"  // IWYU pragma: associated

#include <crypto/sph_blake.h>
#include <crypto/sph_bmw.h>
#include <crypto/sph_cubehash.h>
#include <crypto/sph_echo.h>
#include <crypto/sph_groestl.h>
#include <crypto/sph_jh.h>
#include <crypto/sph_keccak.h>
#include <crypto/sph_luffa.h>
#include <crypto/sph_shavite.h>
#include <crypto/sph_simd.h>
#include <crypto/sph_skein.h>
#include <array>
#include <utility>

#include "core/FixedByteArray.tpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::crypto::implementation
{
auto Dash::Digest(
    const crypto::HashType hashType,
    const ReadView data,
    Writer&& output) const noexcept -> bool
{
    auto hashes = std::array<FixedByteArray<64>, 11>{};
    auto& [blake, bmw, groestl, skein, jh, keccak, luffa, cubehash, shavite, simd, echo] =
        hashes;

    if (data.empty()) {
        static const auto empty = ByteArray{
            IsHex,
            "51b572209083576ea221c27e62b4e22063257571ccb6cc3dc3cd17eb67584eba"};

        return copy(empty.Bytes(), std::move(output));
    }

    {
        auto context = sph_blake512_context{};
        auto* c = std::addressof(context);
        sph_blake512_init(c);
        sph_blake512(c, data.data(), data.size());
        sph_blake512_close(c, blake.data());
    }
    {
        auto context = sph_bmw512_context{};
        auto* c = std::addressof(context);
        const auto& previous = blake;
        sph_bmw512_init(c);
        sph_bmw512(c, previous.data(), previous.size());
        sph_bmw512_close(c, bmw.data());
    }
    {
        auto context = sph_groestl512_context{};
        auto* c = std::addressof(context);
        const auto& previous = bmw;
        sph_groestl512_init(c);
        sph_groestl512(c, previous.data(), previous.size());
        sph_groestl512_close(c, groestl.data());
    }
    {
        auto context = sph_skein512_context{};
        auto* c = std::addressof(context);
        const auto& previous = groestl;
        sph_skein512_init(c);
        sph_skein512(c, previous.data(), previous.size());
        sph_skein512_close(c, skein.data());
    }
    {
        auto context = sph_jh512_context{};
        auto* c = std::addressof(context);
        const auto& previous = skein;
        sph_jh512_init(c);
        sph_jh512(c, previous.data(), previous.size());
        sph_jh512_close(c, jh.data());
    }
    {
        auto context = sph_keccak512_context{};
        auto* c = std::addressof(context);
        const auto& previous = jh;
        sph_keccak512_init(c);
        sph_keccak512(c, previous.data(), previous.size());
        sph_keccak512_close(c, keccak.data());
    }
    {
        auto context = sph_luffa512_context{};
        auto* c = std::addressof(context);
        const auto& previous = keccak;
        sph_luffa512_init(c);
        sph_luffa512(c, previous.data(), previous.size());
        sph_luffa512_close(c, luffa.data());
    }
    {
        auto context = sph_cubehash512_context{};
        auto* c = std::addressof(context);
        const auto& previous = luffa;
        sph_cubehash512_init(c);
        sph_cubehash512(c, previous.data(), previous.size());
        sph_cubehash512_close(c, cubehash.data());
    }
    {
        auto context = sph_shavite512_context{};
        auto* c = std::addressof(context);
        const auto& previous = cubehash;
        sph_shavite512_init(c);
        sph_shavite512(c, previous.data(), previous.size());
        sph_shavite512_close(c, shavite.data());
    }
    {
        auto context = sph_simd512_context{};
        auto* c = std::addressof(context);
        const auto& previous = shavite;
        sph_simd512_init(c);
        sph_simd512(c, previous.data(), previous.size());
        sph_simd512_close(c, simd.data());
    }
    {
        auto context = sph_echo512_context{};
        auto* c = std::addressof(context);
        const auto& previous = simd;
        sph_echo512_init(c);
        sph_echo512(c, previous.data(), previous.size());
        sph_echo512_close(c, echo.data());
    }

    return copy(echo.Bytes().substr(0_uz, 32_uz), std::move(output));
}

auto Dash::HMAC(
    const crypto::HashType,
    const ReadView,
    const ReadView,
    Writer&&) const noexcept -> bool
{
    return false;
}
}  // namespace opentxs::crypto::implementation
