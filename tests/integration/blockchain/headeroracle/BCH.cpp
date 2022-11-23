// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>
#include <span>
#include <utility>

#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/util/LogMacros.hpp"
#include "ottest/data/blockchain/Headers.hpp"
#include "ottest/fixtures/blockchain/BlockHeaderListener.hpp"
#include "ottest/fixtures/blockchain/HeaderOracle.hpp"

namespace ottest
{
TEST_F(HeaderOracle_bch, receive)
{
    auto headers = [this] {
        auto out = ot::Vector<std::unique_ptr<bb::Header>>{};

        for (const auto& hex : BitcoinHeaders()) {
            const auto raw = ot::ByteArray{ot::IsHex, hex};
            auto pHeader = api_.Factory().BlockHeader(type_, raw.Bytes());

            OT_ASSERT(pHeader);

            out.emplace_back(std::move(pHeader));
        }

        return out;
    }();
    const auto target = static_cast<bb::Height>(headers.size());
    auto future = blocks_.GetFuture(target);

    EXPECT_TRUE(header_oracle_.Internal().AddHeaders(headers));
    EXPECT_EQ(future.get().height_, target);

    const auto [height, hash] = header_oracle_.BestChain();

    EXPECT_EQ(height, target);

    const auto header = header_oracle_.LoadHeader(hash);

    ASSERT_TRUE(header);

    const auto expectedWork = std::to_string(target + 1);

    EXPECT_EQ(expectedWork, header->Work().Decimal());
}
}  // namespace ottest
