// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "blockchain/block/bitcoin/BlockParser.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <limits>
#include <stdexcept>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::factory
{
auto parse_header(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView in,
    ByteIterator& it,
    std::size_t& expectedSize)
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Header>
{
    if ((nullptr == in.data()) || (0 == in.size())) {
        throw std::runtime_error("Invalid block input");
    }

    it = reinterpret_cast<ByteIterator>(in.data());
    expectedSize = std::size_t{BlockReturnType::header_bytes_};

    if (in.size() < expectedSize) {
        throw std::runtime_error("Block size too short (header)");
    }

    auto pHeader = BitcoinBlockHeader(
        api,
        chain,
        {reinterpret_cast<const char*>(it), BlockReturnType::header_bytes_});

    if (false == bool(pHeader)) {
        throw std::runtime_error("Invalid block header");
    }

    std::advance(it, BlockReturnType::header_bytes_);

    return pHeader;
}

auto parse_transactions(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView in,
    const blockchain::block::bitcoin::Header& header,
    BlockReturnType::CalculatedSize& sizeData,
    ByteIterator& it,
    std::size_t& expectedSize) -> ParsedTransactions
{
    expectedSize += 1;

    if (in.size() < expectedSize) {
        throw std::runtime_error("Block size too short (transaction count)");
    }

    auto& [size, txCount] = sizeData;

    if (false == network::blockchain::bitcoin::DecodeSize(
                     it, expectedSize, in.size(), txCount)) {
        throw std::runtime_error("Failed to decode transaction count");
    }

    const auto transactionCount = txCount.Value();

    if (0 == transactionCount) { throw std::runtime_error("Empty block"); }

    if (transactionCount > std::numeric_limits<int>::max()) {
        throw std::runtime_error("too many transactions");
    }

    auto counter = int{-1};
    auto output = ParsedTransactions{};
    auto& [index, transactions] = output;

    while (transactions.size() < transactionCount) {
        auto data = blockchain::bitcoin::EncodedTransaction::Deserialize(
            api,
            chain,
            ReadView{
                reinterpret_cast<const char*>(it), in.size() - expectedSize});
        const auto txBytes = data.size();
        std::advance(it, txBytes);
        expectedSize += txBytes;
        auto& txid = index.emplace_back(data.txid_);
        transactions.emplace(
            reader(txid),
            BitcoinTransaction(
                api, chain, ++counter, header.Timestamp(), std::move(data)));
    }

    const auto merkle =
        BlockReturnType::calculate_merkle_value(api, chain, index);

    if (header.MerkleRoot() != merkle) {
        throw std::runtime_error("Invalid merkle hash");
    }

    return output;
}
}  // namespace opentxs::factory
