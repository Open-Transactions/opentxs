// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/Blocks.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <span>

#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/block/Parser.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/opentxs.hpp"

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
    opentxs::blockchain::Type chain,
    const opentxs::ReadView bytes) const noexcept -> bool
{
    using opentxs::blockchain::bitcoin::EncodedTransaction;
    using opentxs::blockchain::block::Hash;
    using opentxs::blockchain::block::Parser;
    using namespace opentxs::literals;
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
        }

        return true;
    } else {

        return false;
    }
}
}  // namespace ottest
