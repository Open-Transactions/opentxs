// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "internal/blockchain/protocol/ethereum/base/RLP.hpp"
#include "ottest/data/blockchain/RLP.hpp"
#include "ottest/fixtures/common/OneClientSession.hpp"

namespace ottest
{
TEST_F(OneClientSession, EncodedSize)
{
    const auto& data = GetRLPVectors(client_1_);

    EXPECT_EQ(data.size(), 28);

    for (const auto& item : data) {
        EXPECT_EQ(item.node_.EncodedSize(client_1_), item.encoded_.size())
            << "Invalid encoded size for " << item.name_.c_str();
    }
}

TEST_F(OneClientSession, Encode)
{
    for (const auto& item : GetRLPVectors(client_1_)) {
        auto encoded = client_1_.Factory().Data();

        EXPECT_TRUE(item.node_.Encode(client_1_, encoded.WriteInto()))
            << "Failed to encode " << item.name_.c_str();
        EXPECT_EQ(encoded.size(), item.encoded_.size())
            << "Item " << item.name_.c_str()
            << " encoded to incorrect number of bytes";
        EXPECT_EQ(encoded.asHex(), item.encoded_.asHex())
            << "Incorrect encoding for item " << item.name_.c_str();
    }
}

TEST_F(OneClientSession, Decode)
{
    namespace rlp = ot::blockchain::protocol::ethereum::base::rlp;

    for (const auto& item : GetRLPVectors(client_1_)) {
        const auto node = rlp::Node::Decode(client_1_, item.encoded_.Bytes());
        auto reencoded = client_1_.Factory().Data();

        EXPECT_EQ(node, item.node_) << "Decoded item " << item.name_.c_str()
                                    << " does not match test vector";
        EXPECT_TRUE(item.node_.Encode(client_1_, reencoded.WriteInto()))
            << "Failed to encode " << item.name_.c_str();
        EXPECT_EQ(reencoded.asHex(), item.encoded_.asHex())
            << "Incorrect encoding for item " << item.name_.c_str();
    }
}
}  // namespace ottest
