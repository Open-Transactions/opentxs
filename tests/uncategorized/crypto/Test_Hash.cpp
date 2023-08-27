// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <span>
#include <string_view>

#include "internal/util/P0330.hpp"
#include "ottest/data/crypto/Hashes.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/crypto/Hash.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

TEST_F(Test_Hash, MurmurHash3)
{
    for (const auto& [input, seed, expected] : Murmur()) {
        std::uint32_t calculated{0};
        const auto plaintext = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(input);
        crypto_.Hash().MurmurHash3_32(seed, plaintext, calculated);

        EXPECT_EQ(calculated, expected);
    }
}

TEST_F(Test_Hash, PKCS5_PBKDF2_HMAC_SHA1)
{
    for (const auto& [P, S, c, dkLen, DK] : PBKDF_sha1()) {
        const auto expected = ot::ByteArray{ot::IsHex, DK};
        auto output = ot::ByteArray{};

        EXPECT_TRUE(crypto_.Hash().PKCS5_PBKDF2_HMAC(
            P, S, c, ot::crypto::HashType::Sha1, dkLen, output.WriteInto()));
        EXPECT_EQ(output, expected);
    }
}

TEST_F(Test_Hash, PKCS5_PBKDF2_HMAC_SHA256)
{
    for (const auto& [P, S, c, dkLen, DK] : PBKDF_sha256()) {
        const auto expected = ot::ByteArray{ot::IsHex, DK};
        auto output = ot::ByteArray{};

        EXPECT_TRUE(crypto_.Hash().PKCS5_PBKDF2_HMAC(
            P, S, c, ot::crypto::HashType::Sha256, dkLen, output.WriteInto()));
        EXPECT_EQ(output, expected);
    }
}

TEST_F(Test_Hash, PKCS5_PBKDF2_HMAC_SHA512)
{
    for (const auto& [P, S, c, dkLen, DK] : PBKDF_sha512()) {
        const auto expected = ot::ByteArray{ot::IsHex, DK};
        auto output = ot::ByteArray{};

        EXPECT_TRUE(crypto_.Hash().PKCS5_PBKDF2_HMAC(
            P, S, c, ot::crypto::HashType::Sha512, dkLen, output.WriteInto()));
        EXPECT_EQ(output, expected);
    }
}

TEST_F(Test_Hash, HMAC_SHA2)
{
    for (const auto& [key, d, sha256, sha512] : rfc4231()) {
        const auto data = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(d);
        const auto dataPassword = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(key);
        const auto expected256 = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(sha256);
        const auto expected512 = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(sha512);
        auto output256 = ot::ByteArray{};
        auto output512 = ot::ByteArray{};

        EXPECT_TRUE(crypto_.Hash().HMAC(
            ot::crypto::HashType::Sha256,
            dataPassword.Bytes(),
            data.Bytes(),
            output256.WriteInto()));
        EXPECT_TRUE(crypto_.Hash().HMAC(
            ot::crypto::HashType::Sha512,
            dataPassword.Bytes(),
            data.Bytes(),
            output512.WriteInto()));

        EXPECT_EQ(output256, expected256);
        EXPECT_EQ(output512, expected512);
    }
}

TEST_F(Test_Hash, scrypt_rfc7914)
{
    for (const auto& [input, salt, N, r, p, size, hex] : rfc7914()) {
        const auto expected = [](const auto& bytes) {
            auto out = ot::ByteArray{};
            out.DecodeHex(bytes);
            return out;
        }(hex);
        auto hash = ot::ByteArray{};
        const auto success =
            crypto_.Hash().Scrypt(input, salt, N, r, p, size, hash.WriteInto());

        EXPECT_TRUE(success);
        EXPECT_EQ(hash, expected);
    }
}

TEST_F(Test_Hash, scrypt_litecoin)
{
    for (const auto& [input, salt, N, r, p, size, hex] : ScryptLitecoin()) {
        const auto expected = [](const auto& bytes) {
            auto out = ot::ByteArray{};
            out.DecodeHex(bytes);
            return out;
        }(hex);
        const auto preimage = [](const auto& bytes) {
            auto out = ot::ByteArray{};
            out.DecodeHex(bytes);
            return out;
        }(input);
        auto hash = ot::ByteArray{};

        ASSERT_EQ(preimage.size(), 80);
        ASSERT_EQ(expected.size(), 32);

        const auto success = crypto_.Hash().Scrypt(
            preimage.Bytes(),
            preimage.Bytes(),
            N,
            r,
            p,
            size,
            hash.WriteInto());

        EXPECT_TRUE(success);
        EXPECT_EQ(hash, expected);
    }
}

TEST_F(Test_Hash, nist_short)
{
    for (const auto& [input, sha1, sha256, sha512] : NistBasic()) {
        const auto eSha1 = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(sha1);
        const auto eSha256 = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(sha256);
        const auto eSha512 = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(sha512);
        auto calculatedSha1 = ot::ByteArray{};
        auto calculatedSha256 = ot::ByteArray{};
        auto calculatedSha512 = ot::ByteArray{};
        using enum ot::crypto::HashType;

        EXPECT_TRUE(
            crypto_.Hash().Digest(Sha1, input, calculatedSha1.WriteInto()));
        EXPECT_TRUE(
            crypto_.Hash().Digest(Sha256, input, calculatedSha256.WriteInto()));
        EXPECT_TRUE(
            crypto_.Hash().Digest(Sha512, input, calculatedSha512.WriteInto()));

        EXPECT_EQ(calculatedSha1, eSha1);
        EXPECT_EQ(calculatedSha256, eSha256);
        EXPECT_EQ(calculatedSha512, eSha512);

        const auto streamAll =
            [](const auto& preimage, const auto& expected, auto hasher) {
                auto success{true};
                auto result = ot::ByteArray{};
                const auto hash = hasher(preimage);
                const auto finalize = hasher(result.WriteInto());
                const auto compare = result == expected;
                success &= hash;
                success &= finalize;
                success &= compare;

                return success;
            };
        const auto streamBytes =
            [](const auto& preimage, const auto& expected, auto hasher) {
                auto success{true};
                auto result = ot::ByteArray{};

                for (auto n = 0_uz; n < preimage.size(); ++n) {
                    const auto hash = hasher(preimage.substr(n, 1_uz));
                    success &= hash;
                }

                const auto finalize = hasher(result.WriteInto());
                const auto compare = result == expected;
                success &= finalize;
                success &= compare;

                return success;
            };

        EXPECT_TRUE(streamAll(input, eSha1, crypto_.Hash().Hasher(Sha1)));
        EXPECT_TRUE(streamAll(input, eSha256, crypto_.Hash().Hasher(Sha256)));
        EXPECT_TRUE(streamAll(input, eSha512, crypto_.Hash().Hasher(Sha512)));
        EXPECT_TRUE(streamBytes(input, eSha1, crypto_.Hash().Hasher(Sha1)));
        EXPECT_TRUE(streamBytes(input, eSha256, crypto_.Hash().Hasher(Sha256)));
        EXPECT_TRUE(streamBytes(input, eSha512, crypto_.Hash().Hasher(Sha512)));
        EXPECT_FALSE(streamAll(input, eSha512, crypto_.Hash().Hasher(Sha1)));
        EXPECT_FALSE(streamAll(input, eSha1, crypto_.Hash().Hasher(Sha256)));
        EXPECT_FALSE(streamAll(input, eSha256, crypto_.Hash().Hasher(Sha512)));
        EXPECT_FALSE(streamBytes(input, eSha512, crypto_.Hash().Hasher(Sha1)));
        EXPECT_FALSE(streamBytes(input, eSha1, crypto_.Hash().Hasher(Sha256)));
        EXPECT_FALSE(
            streamBytes(input, eSha256, crypto_.Hash().Hasher(Sha512)));
    }
}

TEST_F(Test_Hash, two_part_hash)
{
    for (const auto& [input, sha1, sha256, sha512] : NistBasic()) {
        const auto eSha256 = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(sha256);
        auto calculatedSha256d = ot::ByteArray{};
        auto calculatedBitcoin = ot::ByteArray{};
        using enum ot::crypto::HashType;

        EXPECT_TRUE(crypto_.Hash().Digest(
            Sha256, eSha256.Bytes(), calculatedSha256d.WriteInto()));
        EXPECT_TRUE(crypto_.Hash().Digest(
            Bitcoin, input, calculatedBitcoin.WriteInto()));

        const auto stream =
            [](const auto& preimage, const auto& expected, auto hasher) {
                auto success{true};
                auto result = ot::ByteArray{};
                const auto hash = hasher(preimage);
                const auto finalize = hasher(result.WriteInto());
                const auto compare = result == expected;
                success &= hash;
                success &= finalize;
                success &= compare;

                return success;
            };

        EXPECT_TRUE(
            stream(input, calculatedSha256d, crypto_.Hash().Hasher(Sha256D)));
        EXPECT_TRUE(
            stream(input, calculatedBitcoin, crypto_.Hash().Hasher(Bitcoin)));
    }
}

TEST_F(Test_Hash, nist_million_characters)
{
    const auto& [input, sha1, sha256, sha512] = NistMillion();
    const auto eSha1 = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(sha1);
    const auto eSha256 = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(sha256);
    const auto eSha512 = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(sha512);
    auto calculatedSha1 = ot::ByteArray{};
    auto calculatedSha256 = ot::ByteArray{};
    auto calculatedSha512 = ot::ByteArray{};
    constexpr auto copies = 1000000_uz;
    const auto& character = input.at(0);
    const ot::UnallocatedVector<char> preimage(copies, character);
    const auto view = ot::ReadView{preimage.data(), preimage.size()};
    using enum ot::crypto::HashType;

    ASSERT_EQ(preimage.size(), copies);
    ASSERT_EQ(preimage.at(0), character);
    ASSERT_EQ(preimage.at(copies - 1u), character);

    EXPECT_TRUE(crypto_.Hash().Digest(Sha1, view, calculatedSha1.WriteInto()));
    EXPECT_TRUE(
        crypto_.Hash().Digest(Sha256, view, calculatedSha256.WriteInto()));
    EXPECT_TRUE(
        crypto_.Hash().Digest(Sha512, view, calculatedSha512.WriteInto()));

    EXPECT_EQ(calculatedSha1, eSha1);
    EXPECT_EQ(calculatedSha256, eSha256);
    EXPECT_EQ(calculatedSha512, eSha512);

    const auto stream = [](const auto& in, const auto& expected, auto hasher) {
        auto success{true};
        auto result = ot::ByteArray{};

        for (auto n = 0_uz; n < copies; ++n) {
            const auto hash = hasher(in);
            success &= hash;
        }

        const auto finalize = hasher(result.WriteInto());
        const auto compare = result == expected;
        success &= finalize;
        success &= compare;

        return success;
    };

    EXPECT_TRUE(stream(input, eSha1, crypto_.Hash().Hasher(Sha1)));
    EXPECT_TRUE(stream(input, eSha256, crypto_.Hash().Hasher(Sha256)));
    EXPECT_TRUE(stream(input, eSha512, crypto_.Hash().Hasher(Sha512)));
    EXPECT_FALSE(stream(input, eSha512, crypto_.Hash().Hasher(Sha1)));
    EXPECT_FALSE(stream(input, eSha1, crypto_.Hash().Hasher(Sha256)));
    EXPECT_FALSE(stream(input, eSha256, crypto_.Hash().Hasher(Sha512)));
}

TEST_F(Test_Hash, nist_gigabyte_string)
{
    const auto& [input, sha1, sha256, sha512] = NistGigabyte();
    // TODO c++20
    const auto eSha1 = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(sha1);
    // TODO c++20
    const auto eSha256 = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(sha256);
    // TODO c++20
    const auto eSha512 = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(sha512);
    auto calculatedSha1 = ot::ByteArray{};
    auto calculatedSha256 = ot::ByteArray{};
    auto calculatedSha512 = ot::ByteArray{};
    constexpr auto copies = 16777216_uz;
    constexpr auto size = 1073741824_uz;
    auto preimage = ot::UnallocatedVector<char>{};
    preimage.reserve(size);
    const auto* const start = input.data();
    const auto* const end = input.data() + input.size();
    using enum ot::crypto::HashType;

    ASSERT_EQ(size, copies * input.size());

    for (auto count = 0_uz; count < copies; ++count) {
        preimage.insert(preimage.end(), start, end);
    }

    const auto view = ot::ReadView{preimage.data(), preimage.size()};

    ASSERT_EQ(preimage.size(), size);
    EXPECT_TRUE(crypto_.Hash().Digest(Sha1, view, calculatedSha1.WriteInto()));
    EXPECT_TRUE(
        crypto_.Hash().Digest(Sha256, view, calculatedSha256.WriteInto()));
    EXPECT_TRUE(
        crypto_.Hash().Digest(Sha512, view, calculatedSha512.WriteInto()));
    EXPECT_EQ(calculatedSha1, eSha1);
    EXPECT_EQ(calculatedSha256, eSha256);
    EXPECT_EQ(calculatedSha512, eSha512);

    const auto stream = [](const auto& in, const auto& expected, auto hasher) {
        auto success{true};
        auto result = ot::ByteArray{};

        for (auto n = 0_uz; n < copies; ++n) {
            const auto hash = hasher(in);
            success &= hash;
        }

        const auto finalize = hasher(result.WriteInto());
        const auto compare = result == expected;
        success &= finalize;
        success &= compare;

        return success;
    };

    EXPECT_TRUE(stream(input, eSha1, crypto_.Hash().Hasher(Sha1)));
    EXPECT_TRUE(stream(input, eSha256, crypto_.Hash().Hasher(Sha256)));
    EXPECT_TRUE(stream(input, eSha512, crypto_.Hash().Hasher(Sha512)));
}

TEST_F(Test_Hash, argon2i)
{
    static constexpr auto bytes{32_uz};
    const auto& ot = OTTestEnvironment::GetOT();
    const auto& api = ot.StartClientSession(0);
    const auto reason = api.Factory().PasswordPrompt(__func__);

    for (const auto& [iterations, memory, threads, input, salt, hex] :
         Argon2i()) {
        const auto key = api.Crypto().Symmetric().Key(
            ot.Factory().SecretFromText(input),
            salt,
            iterations,
            memory,
            threads,
            bytes,
            opentxs::crypto::symmetric::Source::Argon2i);
        const auto hash = GetHash(api, key, reason, bytes);

        EXPECT_EQ(hash.asHex(), hex);
    }
}

TEST_F(Test_Hash, argon2id)
{
    static constexpr auto bytes{32_uz};
    const auto& ot = OTTestEnvironment::GetOT();
    const auto& api = ot.StartClientSession(0);
    const auto reason = api.Factory().PasswordPrompt(__func__);

    for (const auto& [iterations, memory, threads, input, salt, hex] :
         Argon2id()) {
        const auto key = api.Crypto().Symmetric().Key(
            ot.Factory().SecretFromText(input),
            salt,
            iterations,
            memory,
            threads,
            bytes,
            opentxs::crypto::symmetric::Source::Argon2id);
        const auto hash = GetHash(api, key, reason, bytes);

        EXPECT_EQ(hash.asHex(), hex);
    }
}

TEST_F(Test_Hash, X11)
{
    const auto& ot = OTTestEnvironment::GetOT();
    const auto& api = ot.StartClientSession(0);
    const auto reason = api.Factory().PasswordPrompt(__func__);

    for (const auto& [preimage, val] : X11Vectors()) {
        const auto expected = ot::ByteArray{ot::IsHex, val};
        auto output = ot::ByteArray{};

        EXPECT_TRUE(crypto_.Hash().Digest(
            ot::crypto::HashType::X11, preimage, output.WriteInto()));
        EXPECT_EQ(output.asHex(), expected.asHex())
            << "Failed input: " << preimage;
    }
}
}  // namespace ottest
