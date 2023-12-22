// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <string_view>

#include "ottest/fixtures/blockchain/Basic.hpp"
#include "ottest/fixtures/blockchain/BlockHeader.hpp"

namespace b = ot::blockchain;

namespace ottest
{
TEST_F(Test_BlockHeader, init_opentxs) {}

TEST_F(Test_BlockHeader, btc_genesis_block_hash_oracle)
{
    const auto expectedHash = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(btc_genesis_hash_);
    const auto& genesisHash = GenesisHash(b::Type::Bitcoin);

    EXPECT_EQ(expectedHash, genesisHash);
}

TEST_F(Test_BlockHeader, ltc_genesis_block_hash_oracle)
{
    const auto expectedHash = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(ltc_genesis_hash_);
    const auto& genesisHash = GenesisHash(b::Type::Litecoin);

    EXPECT_EQ(expectedHash, genesisHash);
}

TEST_F(Test_BlockHeader, btc_genesis_block_header)
{
    const auto blankHash = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(blank_hash_);
    const auto expectedHash = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(btc_genesis_hash_);
    const ot::UnallocatedCString numericHash{btc_genesis_hash_numeric_};
    const auto& header = GenesisHeader(b::Type::Bitcoin);

    EXPECT_TRUE(CheckState(header));
    EXPECT_EQ(expectedHash, header.Hash());
    EXPECT_EQ(header.Height(), 0);
    EXPECT_EQ(numericHash, header.NumericHash().asHex());
    EXPECT_EQ(header.ParentHash(), blankHash);

    const auto [height, hash] = header.Position();

    EXPECT_EQ(header.Hash(), hash);
    EXPECT_EQ(header.Height(), height);
}

TEST_F(Test_BlockHeader, ltc_genesis_block_header)
{
    const auto blankHash = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(blank_hash_);
    const auto expectedHash = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(ltc_genesis_hash_);
    const ot::UnallocatedCString numericHash{ltc_genesis_hash_numeric_};
    const auto& header = GenesisHeader(b::Type::Litecoin);

    EXPECT_TRUE(CheckState(header));
    EXPECT_EQ(expectedHash, header.Hash());
    EXPECT_EQ(header.Height(), 0);
    EXPECT_EQ(numericHash, header.NumericHash().asHex());
    EXPECT_EQ(header.ParentHash(), blankHash);

    const auto [height, hash] = header.Position();

    EXPECT_EQ(header.Hash(), hash);
    EXPECT_EQ(header.Height(), height);
}

TEST_F(Test_BlockHeader, serialize_deserialize)
{
    const auto expectedHash = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(btc_genesis_hash_);
    const auto& header = GenesisHeader(b::Type::Bitcoin);

    auto bytes = ot::Space{};
    EXPECT_TRUE(header.Serialize(ot::writer(bytes), false));
    auto restored =
        api_.Factory().BlockHeaderFromProtobuf(ot::reader(bytes), {});

    EXPECT_TRUE(restored.IsValid());
    EXPECT_EQ(expectedHash, restored.Hash());
    EXPECT_TRUE(IsEqual(restored, header));
}
}  // namespace ottest
