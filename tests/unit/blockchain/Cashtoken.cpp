// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/Cashtoken.hpp"

#include <memory>

#include "ottest/data/blockchain/Cashtoken.hpp"

namespace ottest
{
TEST_F(Cashtoken, init) {}

TEST_F(Cashtoken, bch_vmb_tests_before_chip_cashtokens_nonstandard)
{
    EXPECT_TRUE(
        ParseBlocks(bch_vmb_tests_before_chip_cashtokens_nonstandard_json()));
}

TEST_F(Cashtoken, bch_vmb_tests_before_chip_cashtokens_standard)
{
    EXPECT_TRUE(
        ParseBlocks(bch_vmb_tests_before_chip_cashtokens_standard_json()));
}

TEST_F(Cashtoken, bch_vmb_tests_chip_cashtokens_nonstandard)
{
    EXPECT_TRUE(ParseBlocks(bch_vmb_tests_chip_cashtokens_nonstandard_json()));
}

TEST_F(Cashtoken, bch_vmb_tests_chip_cashtokens_standard)
{
    EXPECT_TRUE(ParseBlocks(bch_vmb_tests_chip_cashtokens_standard_json()));
}
}  // namespace ottest
