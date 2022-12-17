// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <iterator>
#include <memory>
#include <span>

#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "ottest/data/blockchain/Headers.hpp"
#include "ottest/fixtures/blockchain/BlockHeaderListener.hpp"
#include "ottest/fixtures/blockchain/HeaderOracle.hpp"

namespace ottest
{
TEST_F(HeaderOracle_btc, receive)
{
    auto headers = [this] {
        const auto in = BitcoinHeaders();
        auto out = ot::Vector<bb::Header>{};
        out.reserve(in.size());
        out.clear();
        std::transform(
            in.begin(),
            in.end(),
            std::back_inserter(out),
            [&](const auto& hex) {
                return api_.Factory().BlockHeaderFromNative(
                    type_, ot::ByteArray{ot::IsHex, hex}.Bytes(), {});
            }

        );

        return out;
    }();
    const auto target = static_cast<bb::Height>(headers.size());
    auto future = blocks_.GetFuture(target);

    EXPECT_TRUE(header_oracle_.Internal().AddHeaders(headers));
    EXPECT_EQ(future.get().height_, target);

    const auto [height, hash] = header_oracle_.BestChain();

    EXPECT_EQ(height, target);

    const auto header = header_oracle_.LoadHeader(hash);

    EXPECT_TRUE(header.IsValid());

    const auto expectedWork = std::to_string(target + 1);

    EXPECT_EQ(expectedWork, header.Work().Decimal());
}
}  // namespace ottest
