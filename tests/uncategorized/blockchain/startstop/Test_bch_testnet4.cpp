// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

#include "ottest/fixtures/blockchain/StartStop.hpp"

namespace ottest
{
TEST_F(Test_StartStop, init_opentxs) {}

TEST_F(Test_StartStop, bch_testnet4)
{
    constexpr auto chain = opentxs::blockchain::Type::BitcoinCash_testnet4;

    EXPECT_TRUE(api_.Network().Blockchain().Start(chain));
    EXPECT_TRUE(api_.Network().Blockchain().Stop(chain));
}
}  // namespace ottest
