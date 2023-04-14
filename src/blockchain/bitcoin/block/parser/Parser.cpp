// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/bitcoin/block/parser/Parser.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::bitcoin::block
{
Parser::Parser(
    const api::Crypto& crypto,
    blockchain::Type type,
    alloc::Default alloc) noexcept
    : bitcoin::block::ParserBase(crypto, type, alloc)
{
}

auto Parser::construct_block(blockchain::block::Block& out) noexcept -> bool
{
    const auto count = transactions_.size();
    out = {factory::BitcoinBlock(
        chain_,
        std::move(header_),
        make_index(txids_),
        make_index(wtxids_),
        get_transactions(),
        CalculatedSize{bytes_, CompactSize{count}},
        alloc_)};

    return out.IsValid();
}
}  // namespace opentxs::blockchain::bitcoin::block
