// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"
#include "internal/blockchain/block/Block.hpp"

#define BTC_GENESIS_HASH_NUMERIC                                               \
    "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"
#define BTC_GENESIS_HASH                                                       \
    "0x6fe28c0ab6f1b372c1a6a246ae63f74f931e8365e15a089c68d6190000000000"
#define BLANK_HASH                                                             \
    "0x0000000000000000000000000000000000000000000000000000000000000000"

namespace b = ot::blockchain;
namespace bb = b::block;
namespace bc = b::client;

namespace
{
class Test_BlockHeader : public ::testing::Test
{
public:
    const ot::api::client::internal::Manager& api_;

    Test_BlockHeader()
        : api_(dynamic_cast<const ot::api::client::internal::Manager&>(
              ot::Context().StartClient(OTTestEnvironment::test_args_, 0)))
    {
    }
};

TEST_F(Test_BlockHeader, init_opentxs) {}

TEST_F(Test_BlockHeader, genesis_block_hash_oracle)
{
    const auto expectedHash =
        ot::Data::Factory(BTC_GENESIS_HASH, ot::Data::Mode::Hex);
    const auto& genesisHash =
        bc::HeaderOracle::GenesisBlockHash(b::Type::Bitcoin);

    EXPECT_EQ(expectedHash.get(), genesisHash);
}

TEST_F(Test_BlockHeader, genesis_block_header)
{
    const auto blankHash =
        ot::Data::Factory(std::string(BLANK_HASH), ot::Data::Mode::Hex);
    const auto expectedHash =
        ot::Data::Factory(BTC_GENESIS_HASH, ot::Data::Mode::Hex);
    const std::string numericHash{BTC_GENESIS_HASH_NUMERIC};
    std::unique_ptr<const bb::Header> pHeader{
        ot::factory::GenesisBlockHeader(api_, b::Type::Bitcoin)};

    ASSERT_TRUE(pHeader);

    const auto& header = *pHeader;

    EXPECT_EQ(header.EffectiveState(), bb::Header::Status::Normal);
    EXPECT_EQ(expectedHash.get(), header.Hash());
    EXPECT_EQ(header.Height(), 0);
    EXPECT_EQ(header.InheritedState(), bb::Header::Status::Normal);
    EXPECT_FALSE(header.IsBlacklisted());
    EXPECT_FALSE(header.IsDisconnected());
    EXPECT_EQ(header.LocalState(), bb::Header::Status::Checkpoint);
    EXPECT_EQ(numericHash, header.NumericHash()->asHex());
    EXPECT_EQ(header.ParentHash(), blankHash.get());

    const auto [height, hash] = header.Position();

    EXPECT_EQ(header.Hash(), hash.get());
    EXPECT_EQ(header.Height(), height);
}

TEST_F(Test_BlockHeader, Serialize)
{
    std::unique_ptr<const bb::Header> pHeader{
        ot::factory::GenesisBlockHeader(api_, b::Type::Bitcoin)};

    ASSERT_TRUE(pHeader);

    const auto& header = *pHeader;
    const auto serialized = header.Serialize();
    const auto& local = serialized.local();
    const auto& bitcoin = serialized.bitcoin();

    EXPECT_EQ(serialized.version(), 1);
    EXPECT_EQ(serialized.type(), static_cast<std::uint32_t>(b::Type::Bitcoin));
    EXPECT_EQ(local.version(), 1);
    EXPECT_EQ(local.height(), header.Height());
    EXPECT_EQ(local.status(), static_cast<std::uint32_t>(header.LocalState()));
    EXPECT_EQ(
        local.inherit_status(),
        static_cast<std::uint32_t>(header.InheritedState()));
    EXPECT_EQ(bitcoin.version(), 1);
    EXPECT_EQ(bitcoin.previous_header(), header.ParentHash().str());
}

TEST_F(Test_BlockHeader, Deserialize)
{
    const auto expectedHash =
        ot::Data::Factory(BTC_GENESIS_HASH, ot::Data::Mode::Hex);
    std::unique_ptr<const bb::Header> pHeader{
        ot::factory::GenesisBlockHeader(api_, b::Type::Bitcoin)};

    ASSERT_TRUE(pHeader);

    const auto serialized = pHeader->Serialize();

    pHeader = api_.Factory().BlockHeader(serialized);

    ASSERT_TRUE(pHeader);
    EXPECT_EQ(expectedHash.get(), pHeader->Hash());
}
}  // namespace
