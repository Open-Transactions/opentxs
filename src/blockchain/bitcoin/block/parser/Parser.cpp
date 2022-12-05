// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "blockchain/bitcoin/block/parser/Parser.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/block/Block.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::bitcoin::block
{
Parser::Parser(const api::Crypto& crypto, blockchain::Type type) noexcept
    : bitcoin::block::ParserBase(crypto, type)
{
}

auto Parser::construct_block(std::shared_ptr<Block>& out) noexcept -> bool
{
    try {
        const auto count = transactions_.size();
        auto tx = get_transactions();
        using Type = implementation::Block;
        out = std::make_shared<Type>(
            chain_,
            std::move(header_),
            std::move(txids_),
            std::move(tx),
            Type::CalculatedSize{bytes_, CompactSize{count}});
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    return out.operator bool();
}
}  // namespace opentxs::blockchain::bitcoin::block
