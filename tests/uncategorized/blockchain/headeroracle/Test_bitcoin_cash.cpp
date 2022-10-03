// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>
#include <utility>

#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "ottest/fixtures/blockchain/HeaderOracle.hpp"

ot::Vector<std::unique_ptr<bb::Header>> headers_btc_{};
ot::Vector<std::unique_ptr<bb::Header>> headers_bch_{};

namespace ottest
{
TEST_F(Test_HeaderOracle_btc, init_opentxs) {}

TEST_F(Test_HeaderOracle_btc, stage_headers)
{
    for (const auto& hex : bitcoin_) {
        const auto raw = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(hex);
        auto pHeader = api_.Factory().BlockHeader(type_, raw.Bytes());

        ASSERT_TRUE(pHeader);

        headers_btc_.emplace_back(std::move(pHeader));
    }

    for (const auto& hex : bitcoin_) {
        const auto raw = [](const auto& hex) {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }(hex);
        auto pHeader = api_.Factory().BlockHeader(
            ot::blockchain::Type::BitcoinCash, raw.Bytes());

        ASSERT_TRUE(pHeader);

        headers_bch_.emplace_back(std::move(pHeader));
    }
}

TEST_F(Test_HeaderOracle_btc, receive_btc)
{
    EXPECT_TRUE(header_oracle_.Internal().AddHeaders(headers_btc_));

    const auto [height, hash] = header_oracle_.BestChain();

    EXPECT_EQ(height, bitcoin_.size());

    const auto header = header_oracle_.LoadHeader(hash);

    ASSERT_TRUE(header);

    const auto expectedWork = std::to_string(bitcoin_.size() + 1);

    EXPECT_EQ(expectedWork, header->Work().Decimal());
}

TEST_F(Test_HeaderOracle_btc, receive_bch)
{
    api_.Network().Blockchain().Start(b::Type::BitcoinCash);
    const auto handle =
        api_.Network().Blockchain().GetChain(b::Type::BitcoinCash);
    const auto& network = handle.get();
    auto& oracle = const_cast<bc::HeaderOracle&>(network.HeaderOracle());

    EXPECT_TRUE(oracle.Internal().AddHeaders(headers_bch_));

    const auto [height, hash] = oracle.BestChain();

    EXPECT_EQ(height, bitcoin_.size());

    const auto header = oracle.LoadHeader(hash);

    ASSERT_TRUE(header);

    const auto expectedWork = std::to_string(bitcoin_.size() + 1);

    EXPECT_EQ(expectedWork, header->Work().Decimal());
}

TEST_F(Test_HeaderOracle_btc, shutdown) { Shutdown(); }
}  // namespace ottest
