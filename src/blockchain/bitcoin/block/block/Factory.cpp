// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/block/block/BlockPrivate.hpp"
#include "blockchain/bitcoin/block/block/Imp.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/block/Parser.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto BitcoinBlock(
    const api::Crypto& crypto,
    const blockchain::block::Header& previous,
    blockchain::bitcoin::block::Transaction gen,
    const std::uint32_t nBits,
    std::span<blockchain::bitcoin::block::Transaction> extra,
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
        auto ids = blockchain::bitcoin::block::TxidIndex{alloc.result_};
        ids.reserve(count);
        ids.clear();
        auto hashes = blockchain::bitcoin::block::TxidIndex{alloc.result_};
        hashes.reserve(count);
        hashes.clear();
        auto map = blockchain::bitcoin::block::TransactionMap{alloc.result_};
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
        auto header = blockchain::bitcoin::block::Header{BitcoinBlockHeader(
            crypto,
            previous,
            nBits,
            version,
            blockchain::bitcoin::block::CalculateMerkleValue(
                crypto, chain, merkle),
            abort,
            alloc)};

        if (false == header.IsValid()) {

            throw std::runtime_error{"Failed to create block header"};
        }

        using enum blockchain::Type;

        switch (chain) {
            case Bitcoin:
            case Bitcoin_testnet3:
            case BitcoinCash:
            case BitcoinCash_testnet3:
            case BitcoinCash_testnet4:
            case Litecoin:
            case Litecoin_testnet4:
            case BitcoinSV:
            case BitcoinSV_testnet3:
            case eCash:
            case eCash_testnet3:
            case Dash:
            case Dash_testnet3:
            case UnitTest: {

                return BitcoinBlock(
                    chain,
                    std::move(header),
                    std::move(ids),
                    std::move(hashes),
                    std::move(map),
                    std::nullopt,
                    alloc);
            }
            case PKT:
            case PKT_testnet:
            case UnknownBlockchain:
            case Ethereum_frontier:
            case Ethereum_ropsten:
            default: {
                const auto error =
                    UnallocatedCString{"unsupported chain "}.append(
                        print(chain));

                throw std::runtime_error{error};
            }
        }
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

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
        LogError()("opentxs::factory::")(__func__)(": failed to deserialize ")(
            print(chain))(" block")
            .Flush();

        return {alloc.result_};
    }

    return out;
}

auto BitcoinBlock(
    const blockchain::Type chain,
    blockchain::bitcoin::block::Header header,
    blockchain::bitcoin::block::TxidIndex&& ids,
    blockchain::bitcoin::block::TxidIndex&& hashes,
    blockchain::bitcoin::block::TransactionMap&& transactions,
    std::optional<blockchain::bitcoin::block::CalculatedSize>&& size,
    alloc::Strategy alloc) noexcept -> blockchain::block::BlockPrivate*
{
    using ReturnType = blockchain::bitcoin::block::implementation::Block;
    using BlankType = blockchain::bitcoin::block::BlockPrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc.result_};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);
        pmr.construct(
            out,
            chain,
            std::move(header),
            std::move(ids),
            std::move(hashes),
            std::move(transactions),
            std::nullopt);

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc.result_};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}
}  // namespace opentxs::factory
