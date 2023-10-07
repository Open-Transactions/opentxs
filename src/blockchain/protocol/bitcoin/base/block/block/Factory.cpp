// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>

#include "blockchain/protocol/bitcoin/base/block/block/BlockPrivate.hpp"
#include "blockchain/protocol/bitcoin/base/block/block/Imp.hpp"
#include "internal/blockchain/block/Parser.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Category.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Header.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinBlock(
    const api::Crypto& crypto,
    const blockchain::block::Header& previous,
    blockchain::protocol::bitcoin::base::block::Transaction gen,
    const std::uint32_t nBits,
    std::span<blockchain::protocol::bitcoin::base::block::Transaction> extra,
    const std::int32_t version,
    const AbortFunction abort,
    alloc::Strategy alloc) noexcept -> blockchain::block::Block
{
    try {
        if (false == gen.IsValid()) {
            throw std::runtime_error{"Invalid generation transaction"};
        }

        const auto count = 1_uz + extra.size();
        gen.Internal().asBitcoin().SetPosition(0);
        auto merkle = Vector<blockchain::block::TransactionHash>{alloc.work_};
        merkle.reserve(count);
        merkle.clear();
        auto ids = blockchain::protocol::bitcoin::base::block::TxidIndex{
            alloc.result_};
        ids.reserve(count);
        ids.clear();
        auto hashes = blockchain::protocol::bitcoin::base::block::TxidIndex{
            alloc.result_};
        hashes.reserve(count);
        hashes.clear();
        auto map = blockchain::protocol::bitcoin::base::block::TransactionMap{
            alloc.result_};
        map.reserve(count);
        map.clear();
        auto position = 0_z;

        {
            if (false == gen.IsValid()) {
                throw std::runtime_error{"Invalid generation transaction"};
            }

            const auto& id = gen.ID();
            merkle.emplace_back(id);
            ids.try_emplace(id, position);
            hashes.try_emplace(gen.Hash(), position);
            map.emplace_back(std::move(gen));
        }

        for (auto& tx : extra) {
            if (false == tx.IsValid()) {
                throw std::runtime_error{"Invalid transaction"};
            }

            tx.Internal().asBitcoin().SetPosition(++position);
            const auto& id = tx.ID();
            merkle.emplace_back(id);
            ids.emplace(id, position);
            hashes.try_emplace(tx.Hash(), position);
            map.emplace_back(std::move(tx));
        }

        const auto chain = previous.Type();
        auto header = blockchain::protocol::bitcoin::base::block::Header{
            BitcoinBlockHeader(
                crypto,
                previous,
                nBits,
                version,
                blockchain::protocol::bitcoin::base::block::
                    CalculateMerkleValue(crypto, chain, merkle),
                abort,
                alloc)};

        if (false == header.IsValid()) {

            throw std::runtime_error{"Failed to create block header"};
        }

        using enum blockchain::Category;

        switch (category(chain)) {
            case output_based: {

                return BitcoinBlock(
                    chain,
                    std::move(header),
                    std::move(ids),
                    std::move(hashes),
                    std::move(map),
                    std::nullopt,
                    alloc);
            }
            case unknown_category:
            case balance_based:
            default: {
                const auto error =
                    UnallocatedCString{"unsupported chain "}.append(
                        print(chain));

                throw std::runtime_error{error};
            }
        }
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {alloc.result_};
    }
}

auto BitcoinBlock(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView in,
    alloc::Strategy alloc) noexcept -> blockchain::block::Block
{
    auto out = blockchain::block::Block{alloc.result_};
    using blockchain::block::Parser;

    if (false == Parser::Construct(crypto, chain, in, out, alloc)) {
        LogError()()("failed to deserialize ")(print(chain))(" block").Flush();

        return {alloc.result_};
    }

    return out;
}

auto BitcoinBlock(
    const blockchain::Type chain,
    blockchain::protocol::bitcoin::base::block::Header header,
    blockchain::protocol::bitcoin::base::block::TxidIndex&& ids,
    blockchain::protocol::bitcoin::base::block::TxidIndex&& hashes,
    blockchain::protocol::bitcoin::base::block::TransactionMap&& transactions,
    std::optional<blockchain::protocol::bitcoin::base::block::CalculatedSize>&&
        size,
    alloc::Strategy alloc) noexcept -> blockchain::block::BlockPrivate*
{
    using ReturnType =
        blockchain::protocol::bitcoin::base::block::implementation::Block;
    using BlankType = blockchain::protocol::bitcoin::base::block::BlockPrivate;

    try {

        return pmr::construct<ReturnType>(
            alloc.result_,
            chain,
            std::move(header),
            std::move(ids),
            std::move(hashes),
            std::move(transactions),
            std::nullopt);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}
}  // namespace opentxs::factory
