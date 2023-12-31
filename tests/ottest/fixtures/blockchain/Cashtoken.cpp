// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/Cashtoken.hpp"  // IWYU pragma: associated

#include <boost/json.hpp>
#include <opentxs/opentxs.hpp>
#include <opentxs/protobuf/BlockchainTransaction.pb.h>  // IWYU pragma: keep
#include <optional>

#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
Cashtoken::Cashtoken()
    : api_(OTTestEnvironment::GetOT().StartClientSession(
          ot::Options{}.SetBlockchainWalletEnabled(false),
          0))
{
}

auto Cashtoken::check_parsing_and_serialization(ot::ReadView raw) const noexcept
    -> bool
{
    using enum ot::blockchain::Type;
    const auto tx1 = api_.Factory().BlockchainTransaction(
        BitcoinCash, raw, false, ot::Clock::now(), {});

    EXPECT_TRUE(tx1);

    const auto raw2 = [&] {
        auto out = ot::ByteArray{};

        EXPECT_TRUE(tx1.Internal().asBitcoin().Serialize(out.WriteInto()));

        return out;
    }();

    EXPECT_EQ(raw, raw2.Bytes());

    const auto proto = tx1.Internal().asBitcoin().Serialize(api_);

    EXPECT_TRUE(proto);

    if (false == proto.has_value()) { return false; }

    const auto tx2 =
        api_.Factory().Internal().Session().BlockchainTransaction(*proto, {});

    EXPECT_TRUE(tx2);

    const auto raw3 = [&] {
        auto out = ot::ByteArray{};

        EXPECT_TRUE(tx2.Internal().asBitcoin().Serialize(out.WriteInto()));

        return out;
    }();

    EXPECT_EQ(raw, raw3.Bytes());

    return true;
}

auto Cashtoken::ParseBlocks(std::string_view in) const noexcept -> bool
{
    const auto json = [&] {
        auto parser = boost::json::stream_parser{};
        parser.write(in.data(), in.size());

        return parser.release();
    }();

    for (const auto& data : json.as_array()) {
        const auto& val = data.as_array();
        const auto id = std::string{val[0].as_string().c_str()};
        const auto raw = ot::ByteArray{ot::IsHex, val[4].as_string().c_str()};

        EXPECT_TRUE(check_parsing_and_serialization(raw.Bytes()))
            << "failed test: " << id;
    }

    return true;
}
}  // namespace ottest
