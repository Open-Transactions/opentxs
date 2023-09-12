// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/crypto/BitcoinProviders.hpp"

namespace ot = opentxs;

namespace ottest
{
TEST_F(Bitcoin_Providers, Common)
{
    EXPECT_TRUE(test_base58_encode());
    EXPECT_TRUE(test_base58_decode());
    EXPECT_TRUE(test_ripemd160());
    EXPECT_TRUE(test_bip39(crypto_.BIP32()));

    if (have_hd_) {
        EXPECT_TRUE(test_bip32_seed(crypto_.BIP32()));
        EXPECT_TRUE(test_bip32_child_key(crypto_.BIP32()));
    }
}
}  // namespace ottest
