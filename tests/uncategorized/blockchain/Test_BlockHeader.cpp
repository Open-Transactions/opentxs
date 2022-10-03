// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/Header.hpp"
#include "ottest/fixtures/blockchain/Basic.hpp"

namespace b = ot::blockchain;
namespace bb = b::block;
namespace bc = b::node;

namespace ottest
{
class Test_BlockHeader : public ::testing::Test
{
public:
    const ot::api::session::Client& api_;

    Test_BlockHeader()
        : api_(ot::Context().StartClientSession(0))
    {
    }
};

TEST_F(Test_BlockHeader, init_opentxs) {}

TEST_F(Test_BlockHeader, btc_genesis_block_hash_oracle)
{
    const auto expectedHash = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(btc_genesis_hash_);
    const auto& genesisHash = b::params::get(b::Type::Bitcoin).GenesisHash();

    EXPECT_EQ(expectedHash, genesisHash);
}

TEST_F(Test_BlockHeader, ltc_genesis_block_hash_oracle)
{
    const auto expectedHash = [](const auto& hex) {
        auto out = ot::ByteArray{};
        out.DecodeHex(hex);
        return out;
    }(ltc_genesis_hash_);
    const auto& genesisHash = b::params::get(b::Type::Litecoin).GenesisHash();

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
    const auto& header =
        ot::blockchain::params::get(b::Type::Bitcoin).GenesisBlock().Header();

    EXPECT_EQ(
        header.Internal().EffectiveState(),
        bb::internal::Header::Status::Normal);
    EXPECT_EQ(expectedHash, header.Hash());
    EXPECT_EQ(header.Height(), 0);
    EXPECT_EQ(
        header.Internal().InheritedState(),
        bb::internal::Header::Status::Normal);
    EXPECT_FALSE(header.Internal().IsBlacklisted());
    EXPECT_FALSE(header.Internal().IsDisconnected());
    EXPECT_EQ(
        header.Internal().LocalState(),
        bb::internal::Header::Status::Checkpoint);
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
    const auto& header =
        ot::blockchain::params::get(b::Type::Litecoin).GenesisBlock().Header();

    EXPECT_EQ(
        header.Internal().EffectiveState(),
        bb::internal::Header::Status::Normal);
    EXPECT_EQ(expectedHash, header.Hash());
    EXPECT_EQ(header.Height(), 0);
    EXPECT_EQ(
        header.Internal().InheritedState(),
        bb::internal::Header::Status::Normal);
    EXPECT_FALSE(header.Internal().IsBlacklisted());
    EXPECT_FALSE(header.Internal().IsDisconnected());
    EXPECT_EQ(
        header.Internal().LocalState(),
        bb::internal::Header::Status::Checkpoint);
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
    const auto& header =
        ot::blockchain::params::get(b::Type::Bitcoin).GenesisBlock().Header();

    auto bytes = ot::Space{};
    EXPECT_TRUE(header.Serialize(ot::writer(bytes), false));
    auto restored = api_.Factory().BlockHeader(ot::reader(bytes));

    ASSERT_TRUE(restored);
    EXPECT_EQ(expectedHash, restored->Hash());

    EXPECT_EQ(restored->Difficulty(), header.Difficulty());
    EXPECT_EQ(
        restored->Internal().EffectiveState(),
        header.Internal().EffectiveState());
    EXPECT_EQ(restored->Hash(), header.Hash());
    EXPECT_EQ(restored->Height(), header.Height());
    EXPECT_EQ(restored->IncrementalWork(), header.IncrementalWork());
    EXPECT_EQ(
        restored->Internal().InheritedState(),
        header.Internal().InheritedState());
    EXPECT_EQ(
        restored->Internal().IsBlacklisted(),
        header.Internal().IsBlacklisted());
    EXPECT_EQ(
        restored->Internal().IsDisconnected(),
        header.Internal().IsDisconnected());
    EXPECT_EQ(
        restored->Internal().LocalState(), header.Internal().LocalState());
    EXPECT_EQ(restored->NumericHash(), header.NumericHash());
    EXPECT_EQ(restored->ParentHash(), header.ParentHash());
    EXPECT_EQ(restored->ParentWork(), header.ParentWork());
    EXPECT_EQ(restored->Position(), header.Position());
    EXPECT_EQ(restored->Print(), header.Print());
    EXPECT_EQ(restored->Target(), header.Target());
    EXPECT_EQ(restored->Type(), header.Type());
    EXPECT_EQ(restored->Valid(), header.Valid());
    EXPECT_EQ(restored->Work(), header.Work());
}
}  // namespace ottest
