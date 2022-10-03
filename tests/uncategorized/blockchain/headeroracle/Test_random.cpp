// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <memory>
#include <random>
#include <utility>

#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "ottest/fixtures/blockchain/HeaderOracle.hpp"

ot::Vector<std::unique_ptr<bb::Header>> headers_{};

namespace ottest
{
TEST_F(Test_HeaderOracle_btc, init_opentxs) {}

TEST_F(Test_HeaderOracle_btc, stage_headers)
{
    for (const auto& hex : bitcoin_) {
        const auto raw = [&]() {
            auto out = ot::ByteArray{};
            out.DecodeHex(hex);
            return out;
        }();
        auto pHeader = api_.Factory().BlockHeader(type_, raw.Bytes());

        ASSERT_TRUE(pHeader);

        headers_.emplace_back(std::move(pHeader));
    }

    std::shuffle(
        headers_.begin(), headers_.end(), std::mt19937(std::random_device()()));
}

TEST_F(Test_HeaderOracle_btc, receive)
{
    EXPECT_TRUE(header_oracle_.Internal().AddHeaders(headers_));

    const auto [height, hash] = header_oracle_.BestChain();

    EXPECT_EQ(height, bitcoin_.size());

    const auto header = header_oracle_.LoadHeader(hash);

    ASSERT_TRUE(header);

    const auto expectedWork = std::to_string(bitcoin_.size() + 1);

    EXPECT_EQ(expectedWork, header->Work().Decimal());
}

TEST_F(Test_HeaderOracle_btc, shutdown) { Shutdown(); }
}  // namespace ottest
