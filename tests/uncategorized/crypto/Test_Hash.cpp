// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>

#include "internal/util/P0330.hpp"

namespace ot = opentxs;

namespace ottest
{
using namespace opentxs::literals;

struct Nist {
    ot::UnallocatedCString input_{};
    ot::UnallocatedCString sha_1_{};
    ot::UnallocatedCString sha_2_256_{};
    ot::UnallocatedCString sha_2_512_{};
};

class Test_Hash : public ::testing::Test
{
public:
    using HMACVector = std::tuple<
        ot::UnallocatedCString,
        ot::UnallocatedCString,
        ot::UnallocatedCString,
        ot::UnallocatedCString>;
    using MurmurVector =
        std::tuple<ot::UnallocatedCString, std::uint32_t, std::uint32_t>;
    // password, salt, iterations, bytes, expected hex
    using PbkdfVector = std::tuple<
        ot::UnallocatedCString,
        ot::UnallocatedCString,
        std::size_t,
        std::size_t,
        ot::UnallocatedCString>;
    using SiphashVector = std::tuple<
        int,
        int,
        ot::UnallocatedCString,
        ot::UnallocatedCString,
        std::uint64_t>;
    // input, salt, N, r, p, size, expected hex
    using ScryptVector = std::tuple<
        ot::UnallocatedCString,
        ot::UnallocatedCString,
        std::uint64_t,
        std::uint32_t,
        std::uint32_t,
        std::size_t,
        ot::UnallocatedCString>;
    // iterations, memory, threads, input, salt, result hex
    using ArgonVector = std::tuple<
        std::uint32_t,
        std::uint32_t,
        std::uint32_t,
        ot::UnallocatedCString,
        ot::UnallocatedCString,
        ot::UnallocatedCString>;

    static const ot::UnallocatedVector<HMACVector> hmac_sha2_;
    static const ot::UnallocatedVector<MurmurVector> murmur_;
    static const ot::UnallocatedVector<PbkdfVector> pbkdf_sha1_;
    static const ot::UnallocatedVector<PbkdfVector> pbkdf_sha256_;
    static const ot::UnallocatedVector<PbkdfVector> pbkdf_sha512_;
    static const ot::UnallocatedVector<ScryptVector> scrypt_rfc7914_;
    static const ot::UnallocatedVector<ScryptVector> scrypt_litecoin_;
    static const ot::UnallocatedVector<Nist> nist_hashes_;
    static const Nist nist_one_million_;
    static const Nist nist_one_gigabyte_;
    static const ot::UnallocatedVector<ArgonVector> argon_2i_;
    static const ot::UnallocatedVector<ArgonVector> argon_2id_;

    const ot::api::Crypto& crypto_;

    Test_Hash()
        : crypto_(ot::Context().Crypto())
    {
    }
};

// https://tools.ietf.org/html/rfc4231
const ot::UnallocatedVector<Test_Hash::HMACVector> Test_Hash::hmac_sha2_{
    {"0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b",
     "0x4869205468657265",
     "0xb0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7",
     "0x87aa7cdea5ef619d4ff0b4241a1d6cb02379f4e2ce4ec2787ad0b30545e17cdedaa833b"
     "7d6b8a702038b274eaea3f4e4be9d914eeb61f1702e696c203a126854"},
    {"0x4a656665",
     "0x7768617420646f2079612077616e7420666f72206e6f7468696e673f",
     "0x5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843",
     "0x164b7a7bfcf819e2e395fbe73b56e0a387bd64222e831fd610270cd7ea2505549758bf7"
     "5c05a994a6d034f65f8f0e6fdcaeab1a34d4a6b4b636e070a38bce737"},
    {"0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
     "0xddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd"
     "ddddddddddddddddddddddddddddd",
     "0x773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe",
     "0xfa73b0089d56a284efb0f0756c890be9b1b5dbdd8ee81a3655f83e33b2279d39bf3e848"
     "279a722c806b485a47e67c807b946a337bee8942674278859e13292fb"},
    {"0x0102030405060708090a0b0c0d0e0f10111213141516171819",
     "0xcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdc"
     "dcdcdcdcdcdcdcdcdcdcdcdcdcdcd",
     "0x82558a389a443c0ea4cc819899f2083a85f0faa3e578f8077a2e3ff46729665b",
     "0xb0ba465637458c6990e5a8c5f61d4af7e576d97ff94b872de76f8050361ee3dba91ca5c"
     "11aa25eb4d679275cc5788063a5f19741120c4f2de2adebeb10a298dd"},
    {"0xaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
     "0x54657374205573696e67204c6172676572205468616e20426c6f636b2d53697a65204b6"
     "579202d2048617368204b6579204669727374",
     "0x60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54",
     "0x80b24263c7c1a3ebb71493c1dd7be8b49b46d1f41b4aeec1121b013783f8f3526b56d03"
     "7e05f2598bd0fd2215d6a1e5295e64f73f63f0aec8b915a985d786598"},
};

// https://stackoverflow.com/a/31929528
const ot::UnallocatedVector<Test_Hash::MurmurVector> Test_Hash::murmur_{
    {"", 0, 0},
    {"", 1, 1364076727},
    {"", 4294967295, 2180083513},
    {"0xffffffff", 0, 1982413648},
    {"0x21436587", 0, 4116402539},
    {"0x21436587", 1350757870, 593689054},
    {"0x214365", 0, 2118813236},
    {"0x2143", 0, 2700587130},
    {"0x21", 0, 1919294708},
    {"0x00000000", 0, 593689054},
    {"0x000000", 0, 2247144487},
    {"0x0000", 0, 821347078},
    {"0x00", 0, 1364076727},
};

// https://tools.ietf.org/html/rfc6070
const ot::UnallocatedVector<Test_Hash::PbkdfVector> Test_Hash::pbkdf_sha1_{
    {"password", "salt", 1, 20, "0x0c60c80f961f0e71f3a9b524af6012062fe037a6"},
    {"password", "salt", 2, 20, "0xea6c014dc72d6f8ccd1ed92ace1d41f0d8de8957"},
    {"password",
     "salt",
     4096,
     20,
     "0x4b007901b765489abead49d926f721d065a429c1"},
    {"password",
     "salt",
     16777216,
     20,
     "0xeefe3d61cd4da4e4e9945b3d6ba2158c2634e984"},
    {"passwordPASSWORDpassword",
     "saltSALTsaltSALTsaltSALTsaltSALTsalt",
     4096,
     25,
     "0x3d2eec4fe41c849b80c8d83662c0e44a8b291a964cf2f07038"},
};

// https://github.com/Anti-weakpasswords/PBKDF2-Test-Vectors/releases
const ot::UnallocatedVector<Test_Hash::PbkdfVector> Test_Hash::pbkdf_sha256_{
    {"password",
     "salt",
     1,
     32,
     "120FB6CFFCF8B32C43E7225256C4F837A86548C92CCC35480805987CB70BE17B"},
    {"password",
     "salt",
     2,
     32,
     "AE4D0C95AF6B46D32D0ADFF928F06DD02A303F8EF3C251DFD6E2D85A95474C43"},
    {"password",
     "salt",
     4096,
     32,
     "C5E478D59288C841AA530DB6845C4C8D962893A001CE4E11A4963873AA98134A"},
    {"passwordPASSWORDpassword",
     "saltSALTsaltSALTsaltSALTsaltSALTsalt",
     4096,
     40,
     "348C89DBCBD32B2F32D814B8116E84CF2B17347EBC1800181C4E2A1FB8DD53E1C635518C7"
     "DAC47E9"},
    {"password",
     "salt",
     16777216,
     32,
     "CF81C66FE8CFC04D1F31ECB65DAB4089F7F179E89B3B0BCB17AD10E3AC6EBA46"},
};

// https://github.com/Anti-weakpasswords/PBKDF2-Test-Vectors/releases
const ot::UnallocatedVector<Test_Hash::PbkdfVector> Test_Hash::pbkdf_sha512_{
    {"password",
     "salt",
     1,
     64,
     "867F70CF1ADE02CFF3752599A3A53DC4AF34C7A669815AE5D513554E1C8CF252C02D470A2"
     "85A0501BAD999BFE943C08F050235D7D68B1DA55E63F73B60A57FCE"},
    {"password",
     "salt",
     2,
     64,
     "E1D9C16AA681708A45F5C7C4E215CEB66E011A2E9F0040713F18AEFDB866D53CF76CAB286"
     "8A39B9F7840EDCE4FEF5A82BE67335C77A6068E04112754F27CCF4E"},
    {"password",
     "salt",
     4096,
     64,
     "D197B1B33DB0143E018B12F3D1D1479E6CDEBDCC97C5C0F87F6902E072F457B5143F30602"
     "641B3D55CD335988CB36B84376060ECD532E039B742A239434AF2D5"},
    {"passwordPASSWORDpassword",
     "saltSALTsaltSALTsaltSALTsaltSALTsalt",
     4096,
     64,
     "8C0511F4C6E597C6AC6315D8F0362E225F3C501495BA23B868C005174DC4EE71115B59F9E"
     "60CD9532FA33E0F75AEFE30225C583A186CD82BD4DAEA9724A3D3B8"},
};

// https://tools.ietf.org/html/rfc7914
const ot::UnallocatedVector<Test_Hash::ScryptVector> Test_Hash::scrypt_rfc7914_{
    {"",
     "",
     16,
     1,
     1,
     64,
     "77d6576238657b203b19ca42c18a0497f16b4844e3074ae8dfdffa3fede21442fcd0069de"
     "d0948f8326a753a0fc81f17e8d3e0fb2e0d3628cf35e20c38d18906"},
    {"password",
     "NaCl",
     1024,
     8,
     16,
     64,
     "fdbabe1c9d3472007856e7190d01e9fe7c6ad7cbc8237830e77376634b3731622eaf30d92"
     "e22a3886ff109279d9830dac727afb94a83ee6d8360cbdfa2cc0640"},
    {"pleaseletmein",
     "SodiumChloride",
     16384,
     8,
     1,
     64,
     "7023bdcb3afd7348461c06cd81fd38ebfda8fbba904f8e3ea9b543f6545da1f2d54329556"
     "13f0fcf62d49705242a9af9e61e85dc0d651e40dfcf017b45575887"},
    {"pleaseletmein",
     "SodiumChloride",
     1048576,
     8,
     1,
     64,
     "2101cb9b6a511aaeaddbbe09cf70f881ec568d574a2ffd4dabe5ee9820adaa478e56fd8f4"
     "ba5d09ffa1c6d927c40f4c337304049e8a952fbcbf45c6fa77a41a4"},
};

// https://www.litecoin.info/index.php/Block_hashing_algorithm
const ot::UnallocatedVector<Test_Hash::ScryptVector>
    Test_Hash::scrypt_litecoin_{
        {"01000000ae178934851bfa0e83ccb6a3fc4bfddff3641e104b6c4680c31509074e699"
         "be2b"
         "d672d8d2199ef37a59678f92443083e3b85edef8b45c71759371f823bab59a9712661"
         "4f44"
         "d5001d45920180",
         "",
         1024,
         1,
         1,
         32,
         "01796dae1f78a72dfb09356db6f027cd884ba0201e6365b72aa54b3b00000000"},
    };

// https://www.di-mgt.com.au/sha_testvectors.html
const ot::UnallocatedVector<Nist> Test_Hash::nist_hashes_{
    {"abc",
     "a9993e364706816aba3e25717850c26c9cd0d89d",
     "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
     "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a2"
     "74fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f"},
    {"",
     "da39a3ee5e6b4b0d3255bfef95601890afd80709",
     "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
     "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5"
     "d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e"},
    {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
     "84983e441c3bd26ebaae4aa1f95129e5e54670f1",
     "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1",
     "204a8fc6dda82f0a0ced7beb8e08a41657c16ef468b228a8279be331a703c33596fd15c13"
     "b1b07f9aa1d3bea57789ca031ad85c7a71dd70354ec631238ca3445"},
    {"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopj"
     "klmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
     "a49b2446a02c645bf419f995b67091253a04a259",
     "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1",
     "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501d289e4"
     "900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909"},
};
const Nist Test_Hash::nist_one_million_{
    "a",
    "34aa973cd4c4daa4f61eeb2bdbad27316534016f",
    "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0",
    "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973ebde0ff24487"
    "7ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b"};
const Nist Test_Hash::nist_one_gigabyte_{
    "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno",
    "7789f0c9ef7bfc40d93311143dfbe69e2017f592",
    "50e72a0e26442fe2552dc3938ac58658228c0cbfb1d2ca872ae435266fcd055e",
    "b47c933421ea2db149ad6e10fce6c7f93d0752380180ffd7f4629a712134831d77be6091b8"
    "19ed352c2967a2e2d4fa5050723c9630691f1a05a7281dbe6c1086"};
// https://github.com/P-H-C/phc-winner-argon2/blob/master/src/test.c
// Modified salt length and iteration count to correspond to libsodium
// requirements Checked against https://argon2.online/
const ot::UnallocatedVector<Test_Hash::ArgonVector> Test_Hash::argon_2i_{
    {4,
     (1 << 16) << 10,  // 65536 KiB
     1,
     "password",
     "somesaltsomesalt",
     "eba00eb46e3219304995c15da7a4d5b90a79b2712c4776c1038413dd07b2f33a"},
    {4,
     (1 << 8) << 10,  // 256 KiB
     1,
     "password",
     "somesaltsomesalt",
     "797ebc52b990ebdde9adc36a11330f48f292c770ad4fb4e77e5883964d1f7ebb"},
    {4,
     (1 << 8) << 10,  // 256 KiB
     2,
     "password",
     "somesaltsomesalt",
     "26e3e3e4e1d020f2d3bac77177b498752ed847a606da04b5594dc4e4e4a003ca"},
    {3,
     (1 << 16) << 10,  // 65536 KiB
     1,
     "password",
     "somesaltsomesalt",
     "7d1b1163d3c0b791fea802ae5d1ccbd3fe896c54a1b0277ad96e5a1f311293f7"},
    {6,
     (1 << 16) << 10,  // 65536 KiB
     1,
     "password",
     "somesaltsomesalt",
     "bc4b50127a30b18377f81d26754a8ad3652554da170df1f5ff9576bcb76a6cca"},
    {4,
     (1 << 16) << 10,  // 65536 KiB
     1,
     "differentpassword",
     "somesaltsomesalt",
     "58267b9e980abacb368cc1512cc959607ee18de7dce74c0e8bb991c1ead63f41"},
    {4,
     (1 << 16) << 10,  // 65536 KiB
     1,
     "password",
     "diffsaltdiffsalt",
     "3c71a874e9b82570c3d584e4379c06bf5e7f4af5357b62889b0915a0601ef2bd"},
};
// https://github.com/P-H-C/phc-winner-argon2/blob/master/src/test.c
// Modified salt length to correspond to libsodium requirements
// Checked against https://argon2.online/
const ot::UnallocatedVector<Test_Hash::ArgonVector> Test_Hash::argon_2id_{
    {2,
     (1 << 16) << 10,  // 65536 KiB
     1,
     "password",
     "somesaltsomesalt",
     "fc33b78139231d34b71626bd6245c1d72efa190ad605c3d8166a72adcedfa2c2"},
    {2,
     (1 << 8) << 10,  // 256 KiB
     1,
     "password",
     "somesaltsomesalt",
     "cab746b4621993fdc91ec50787980b414a90a692f0bc68dfe19f9c25b3cba9ec"},
    {2,
     (1 << 8) << 10,  // 256 KiB
     2,
     "password",
     "somesaltsomesalt",
     "c112d2ee6b9d514413f806243187952186e8f19cacab80f20a823d549f111d2c"},
    {1,
     (1 << 16) << 10,  // 65536 KiB
     1,
     "password",
     "somesaltsomesalt",
     "ec2b46acb6f8aec6804bf8df88feeca36a4412df3bea8d2cc99c08a9e8977a72"},
    {4,
     (1 << 16) << 10,  // 65536 KiB
     1,
     "password",
     "somesaltsomesalt",
     "34a6f651a0ab14f3b15f86115f0ae5532e1029365d8c218f47d24f66dab2688f"},
    {2,
     (1 << 16) << 10,  // 65536 KiB
     1,
     "differentpassword",
     "somesaltsomesalt",
     "b2a645d33be125cd75f64b157793464041c0fd196b848a9f78508e1a0e5ce4c0"},
    {2,
     (1 << 16) << 10,  // 65536 KiB
     1,
     "password",
     "diffsaltdiffsalt",
     "c22e7f0935f83ed7a0163bfd4f09a2014b94c9aa19dc681e7e781170dcfd1659"},
};

TEST_F(Test_Hash, MurmurHash3)
{
    for (const auto& [input, seed, expected] : murmur_) {
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
    for (const auto& [P, S, c, dkLen, DK] : pbkdf_sha1_) {
        const auto salt = ot::ByteArray{S.data(), S.size()};
        const auto expected = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(DK);
        auto output = ot::ByteArray{};

        EXPECT_TRUE(crypto_.Hash().PKCS5_PBKDF2_HMAC(
            P, salt, c, ot::crypto::HashType::Sha1, dkLen, output));
        EXPECT_EQ(output, expected);
    }
}

TEST_F(Test_Hash, PKCS5_PBKDF2_HMAC_SHA256)
{
    for (const auto& [P, S, c, dkLen, DK] : pbkdf_sha256_) {
        const auto salt = ot::ByteArray{S.data(), S.size()};
        const auto expected = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(DK);
        auto output = ot::ByteArray{};

        EXPECT_TRUE(crypto_.Hash().PKCS5_PBKDF2_HMAC(
            P, salt, c, ot::crypto::HashType::Sha256, dkLen, output));
        EXPECT_EQ(output, expected);
    }
}

TEST_F(Test_Hash, PKCS5_PBKDF2_HMAC_SHA512)
{
    for (const auto& [P, S, c, dkLen, DK] : pbkdf_sha512_) {
        const auto salt = ot::ByteArray{S.data(), S.size()};
        const auto expected = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(DK);
        auto output = ot::ByteArray{};

        EXPECT_TRUE(crypto_.Hash().PKCS5_PBKDF2_HMAC(
            P, salt, c, ot::crypto::HashType::Sha512, dkLen, output));
        EXPECT_EQ(output, expected);
    }
}

TEST_F(Test_Hash, HMAC_SHA2)
{
    for (const auto& [key, d, sha256, sha512] : hmac_sha2_) {
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
    for (const auto& [input, salt, N, r, p, size, hex] : scrypt_rfc7914_) {
        const auto expected = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
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
    for (const auto& [input, salt, N, r, p, size, hex] : scrypt_litecoin_) {
        const auto expected = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(hex);
        const auto preimage = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
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
    for (const auto& [input, sha1, sha256, sha512] : nist_hashes_) {
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

        EXPECT_TRUE(crypto_.Hash().Digest(
            ot::crypto::HashType::Sha1, input, calculatedSha1.WriteInto()));
        EXPECT_TRUE(crypto_.Hash().Digest(
            ot::crypto::HashType::Sha256, input, calculatedSha256.WriteInto()));
        EXPECT_TRUE(crypto_.Hash().Digest(
            ot::crypto::HashType::Sha512, input, calculatedSha512.WriteInto()));

        EXPECT_EQ(calculatedSha1, eSha1);
        EXPECT_EQ(calculatedSha256, eSha256);
        EXPECT_EQ(calculatedSha512, eSha512);
    }
}

TEST_F(Test_Hash, nist_million_characters)
{
    const auto& [input, sha1, sha256, sha512] = nist_one_million_;
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

    ASSERT_EQ(preimage.size(), copies);
    ASSERT_EQ(preimage.at(0), character);
    ASSERT_EQ(preimage.at(copies - 1u), character);

    EXPECT_TRUE(crypto_.Hash().Digest(
        ot::crypto::HashType::Sha1, view, calculatedSha1.WriteInto()));
    EXPECT_TRUE(crypto_.Hash().Digest(
        ot::crypto::HashType::Sha256, view, calculatedSha256.WriteInto()));
    EXPECT_TRUE(crypto_.Hash().Digest(
        ot::crypto::HashType::Sha512, view, calculatedSha512.WriteInto()));

    EXPECT_EQ(calculatedSha1, eSha1);
    EXPECT_EQ(calculatedSha256, eSha256);
    EXPECT_EQ(calculatedSha512, eSha512);
}

TEST_F(Test_Hash, nist_gigabyte_string)
{
    const auto& [input, sha1, sha256, sha512] = nist_one_gigabyte_;
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

    ASSERT_EQ(size, copies * input.size());

    for (auto count = 0_uz; count < copies; ++count) {
        preimage.insert(preimage.end(), start, end);
    }

    const auto view = ot::ReadView{preimage.data(), preimage.size()};

    ASSERT_EQ(preimage.size(), size);
    EXPECT_TRUE(crypto_.Hash().Digest(
        ot::crypto::HashType::Sha1, view, calculatedSha1.WriteInto()));
    EXPECT_TRUE(crypto_.Hash().Digest(
        ot::crypto::HashType::Sha256, view, calculatedSha256.WriteInto()));
    EXPECT_TRUE(crypto_.Hash().Digest(
        ot::crypto::HashType::Sha512, view, calculatedSha512.WriteInto()));
    EXPECT_EQ(calculatedSha1, eSha1);
    EXPECT_EQ(calculatedSha256, eSha256);
    EXPECT_EQ(calculatedSha512, eSha512);
}

TEST_F(Test_Hash, argon2i)
{
    static constexpr auto bytes{32u};
    const auto& ot = ot::Context();
    const auto& api = ot.StartClientSession(0);
    const auto reason = api.Factory().PasswordPrompt(__func__);

    for (const auto& [iterations, memory, threads, input, salt, hex] :
         argon_2i_) {
        const auto key = api.Crypto().Symmetric().Key(
            ot.Factory().SecretFromText(input),
            salt,
            iterations,
            memory,
            threads,
            bytes,
            opentxs::crypto::key::symmetric::Source::Argon2i);
        const auto hash = [&] {
            auto secret = api.Factory().Secret(bytes);
            key->RawKey(reason, secret);

            return api.Factory().DataFromBytes(secret.Bytes());
        }();

        EXPECT_EQ(hash.asHex(), hex);
    }
}

TEST_F(Test_Hash, argon2id)
{
    static constexpr auto bytes{32u};
    const auto& ot = ot::Context();
    const auto& api = ot.StartClientSession(0);
    const auto reason = api.Factory().PasswordPrompt(__func__);

    for (const auto& [iterations, memory, threads, input, salt, hex] :
         argon_2id_) {
        const auto key = api.Crypto().Symmetric().Key(
            ot.Factory().SecretFromText(input),
            salt,
            iterations,
            memory,
            threads,
            bytes,
            opentxs::crypto::key::symmetric::Source::Argon2id);
        const auto hash = [&] {
            auto secret = api.Factory().Secret(bytes);
            key->RawKey(reason, secret);

            return api.Factory().DataFromBytes(secret.Bytes());
        }();

        EXPECT_EQ(hash.asHex(), hex);
    }
}
}  // namespace ottest
