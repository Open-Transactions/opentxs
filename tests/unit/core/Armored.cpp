// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <span>

#include "ottest/data/core/Data.hpp"
#include "ottest/fixtures/core/Armored.hpp"

namespace ottest
{
TEST_F(Armored, encode_and_decode)
{
    for (const auto& input : HexWithoutPrefix()) {
        const auto value = ot::ByteArray{ot::IsHex, input};

        EXPECT_TRUE(Check(value));
    }
}
}  // namespace ottest
