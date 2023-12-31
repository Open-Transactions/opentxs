// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/Blocks.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <opentxs/protobuf/BlockchainTransaction.pb.h>
#include <optional>
#include <span>
#include <string_view>

#include "internal/blockchain/block/Parser.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/blockchain/protocol/bitcoin/base/Bitcoin.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/protobuf/Types.internal.hpp"

namespace ottest
{
auto BlockchainBlocks::CheckBlock(
    opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::Hash& id,
    const opentxs::ReadView bytes) const noexcept -> bool
{
    using opentxs::blockchain::block::Parser;
    const auto check = Parser::Check(ot_.Crypto(), chain, id, bytes, {});

    if (check) {
        auto block = opentxs::blockchain::block::Block{};
        const auto construct =
            Parser::Construct(ot_.Crypto(), chain, bytes, block, {});

        EXPECT_TRUE(construct);
        EXPECT_TRUE(block.IsValid());

        if (block.IsValid()) {
            const auto expected = opentxs::ByteArray{bytes};
            auto s = opentxs::ByteArray{};

            EXPECT_EQ(id, block.Header().Hash());
            EXPECT_TRUE(block.Serialize(s.WriteInto()));
            EXPECT_EQ(expected, s);
        }
    }

    return check;
}

auto BlockchainBlocks::CheckGenesisBlock(
    opentxs::blockchain::Type chain) const noexcept -> bool
{
    try {
        const auto& params = opentxs::blockchain::params::get(chain);
        const auto& block = params.GenesisBlock(ot_.Crypto());
        const auto bytes = params.GenesisBlockSerialized();

        return CheckBlock(chain, block.ID(), bytes);
    } catch (...) {

        return false;
    }
}

auto BlockchainBlocks::CheckTxids(
    const opentxs::api::Session& api,
    opentxs::blockchain::Type chain,
    const opentxs::ReadView bytes) const noexcept -> bool
{
    using opentxs::blockchain::block::Hash;
    using opentxs::blockchain::block::Parser;
    using opentxs::blockchain::protocol::bitcoin::base::EncodedTransaction;
    using namespace opentxs::literals;
    using namespace std::literals;
    const auto& crypto = ot_.Crypto();
    auto block = opentxs::blockchain::block::Block{};
    const auto construct = Parser::Construct(crypto, chain, bytes, block, {});

    EXPECT_TRUE(construct);
    EXPECT_TRUE(block.IsValid());

    if (block.IsValid()) {
        auto count = -1_z;

        for (const auto& tx : block.get()) {
            ++count;

            EXPECT_TRUE(tx.IsValid());

            const auto& internal = tx.Internal().asBitcoin();
            const auto& txid = tx.asBitcoin().TXID();
            const auto& wtxid = tx.asBitcoin().WTXID();
            auto raw = EncodedTransaction{};

            EXPECT_TRUE(internal.Serialize(raw));
            EXPECT_TRUE(raw.CalculateIDs(crypto, chain, (0_z == count)));
            EXPECT_TRUE(txid == raw.txid_)
                << "txid for transaction at position " << std::to_string(count)
                << " expected " << txid.asHex() << " but calculated "
                << raw.txid_.asHex();
            EXPECT_TRUE(wtxid == raw.wtxid_)
                << "wtxid for transaction at position " << std::to_string(count)
                << " expected " << wtxid.asHex() << " but calculated "
                << raw.wtxid_.asHex();
            // FIXME EXPECT_TRUE(check_protobuf(api, tx));
        }

        return true;
    } else {

        return false;
    }
}

auto BlockchainBlocks::check_protobuf(
    const opentxs::api::Session& api,
    const opentxs::blockchain::block::Transaction& tx) const noexcept -> bool
{
    auto result{true};
    const auto proto = tx.Internal().asBitcoin().Serialize(api);

    EXPECT_TRUE(proto.has_value());

    if (false == proto.has_value()) { return false; }

    const auto restored =
        api.Factory().Internal().Session().BlockchainTransaction(*proto, {});

    EXPECT_TRUE(restored.IsValid());

    result &= restored.IsValid();
    const auto required = [&] {
        auto out = opentxs::ByteArray{};
        const auto rc = tx.Internal().asBitcoin().Serialize(out.WriteInto());

        EXPECT_TRUE(rc.has_value());

        result &= rc.has_value();

        return out;
    }();
    const auto obtained = [&] {
        auto out = opentxs::ByteArray{};
        const auto rc =
            restored.Internal().asBitcoin().Serialize(out.WriteInto());

        EXPECT_TRUE(rc.has_value());

        result &= rc.has_value();

        return out;
    }();

    EXPECT_EQ(required.asHex(), obtained.asHex());

    result &= (required == obtained);
    const auto proto2 = restored.Internal().asBitcoin().Serialize(api);

    EXPECT_TRUE(proto2.has_value());

    if (false == proto2.has_value()) { return false; }

    EXPECT_EQ(*proto, *proto2);

    result &= (*proto == *proto2);

    return result;
}
}  // namespace ottest
