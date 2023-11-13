// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/protocol/bitcoin/base/block/parser/Parser.hpp"  // IWYU pragma: associated

#include <optional>
#include <span>
#include <utility>

#include "internal/blockchain/protocol/bitcoin/base/Bitcoin.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
Parser::Parser(
    const api::Crypto& crypto,
    blockchain::Type type,
    alloc::Strategy alloc) noexcept
    : protocol::bitcoin::base::block::ParserBase(crypto, type, alloc)
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
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block
