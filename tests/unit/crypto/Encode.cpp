// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <array>
#include <string_view>

#include "ottest/data/crypto/Z85.hpp"
#include "ottest/fixtures/common/LowLevel.hpp"

namespace ottest
{
using namespace std::literals;

TEST_F(LowLevel, encode_z85_success)
{
    auto encoded = ""s;
    auto rc = ot_.Crypto().Encode().Z85Encode(
        z85_plaintext_view_, opentxs::writer(encoded));

    EXPECT_TRUE(rc);
    EXPECT_EQ(encoded, z85_encoded_view_);
}

TEST_F(LowLevel, encode_z85_preallocated)
{
    auto encoded = std::array<char, 10>{};
    auto rc = ot_.Crypto().Encode().Z85Encode(
        z85_plaintext_view_,
        opentxs::preallocated(encoded.size(), encoded.data()));
    const auto view = std::string_view{encoded.data(), encoded.size()};

    EXPECT_TRUE(rc);
    EXPECT_EQ(view, z85_encoded_view_);
}

TEST_F(LowLevel, encode_z85_wrong_size)
{
    auto encoded = ""s;
    auto rc = ot_.Crypto().Encode().Z85Encode(
        z85_encoded_view_, opentxs::writer(encoded));

    EXPECT_FALSE(rc);
}

TEST_F(LowLevel, decode_z85_success)
{
    auto decoded = ""s;
    auto rc = ot_.Crypto().Encode().Z85Decode(
        z85_encoded_view_, opentxs::writer(decoded));

    EXPECT_TRUE(rc);
    EXPECT_EQ(decoded, z85_plaintext_view_);
}

TEST_F(LowLevel, decode_z85_preallocated)
{
    auto decoded = std::array<char, 8>{};
    auto rc = ot_.Crypto().Encode().Z85Decode(
        z85_encoded_view_,
        opentxs::preallocated(decoded.size(), decoded.data()));
    const auto view = std::string_view{decoded.data(), decoded.size()};

    EXPECT_TRUE(rc);
    EXPECT_EQ(view, z85_plaintext_view_);
}

TEST_F(LowLevel, decode_z85_wrong_size)
{
    auto decoded = ""s;
    auto rc = ot_.Crypto().Encode().Z85Decode(
        z85_plaintext_view_, opentxs::writer(decoded));

    EXPECT_FALSE(rc);
}
}  // namespace ottest
