// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/block/bitcoin/Block.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <type_traits>

#include "blockchain/block/Block.hpp"
#include "blockchain/block/bitcoin/BlockParser.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Container.hpp"

namespace be = boost::endian;

namespace opentxs::factory
{
auto BitcoinBlock(
    const api::Session& api,
    const opentxs::blockchain::block::Header& previous,
    const Transaction_p pGen,
    const std::uint32_t nBits,
    const UnallocatedVector<Transaction_p>& extra,
    const std::int32_t version,
    const AbortFunction abort) noexcept
    -> std::shared_ptr<const opentxs::blockchain::block::bitcoin::Block>
{
    try {
        using Block = blockchain::block::bitcoin::implementation::Block;

        if (false == bool(pGen)) {
            throw std::runtime_error{"Invalid generation transaction"};
        }

        const auto& gen = *pGen;
        using Tx = blockchain::block::bitcoin::Transaction;

        {
            auto& mGen = const_cast<Tx&>(gen);
            mGen.Internal().SetPosition(0);
        }

        auto index = Block::TxidIndex{};
        auto map = Block::TransactionMap{};
        auto position = std::size_t{0};

        {
            const auto& id = gen.ID();
            index.emplace_back(id.begin(), id.end());
            const auto& item = index.back();
            map.emplace(reader(item), pGen);
        }

        for (const auto& tx : extra) {
            if (false == bool(tx)) {
                throw std::runtime_error{"Invalid transaction"};
            }

            {
                auto& mTx = const_cast<Tx&>(*tx);
                mTx.Internal().SetPosition(++position);
            }

            const auto& id = tx->ID();
            index.emplace_back(id.begin(), id.end());
            const auto& item = index.back();
            map.emplace(reader(item), tx);
        }

        const auto chain = previous.Type();
        auto header = BitcoinBlockHeader(
            api,
            previous,
            nBits,
            version,
            Block::calculate_merkle_value(api, chain, index),
            abort);

        if (false == bool(header)) {
            throw std::runtime_error{"Failed to create block header"};
        }

        switch (chain) {
            case blockchain::Type::Bitcoin:
            case blockchain::Type::Bitcoin_testnet3:
            case blockchain::Type::BitcoinCash:
            case blockchain::Type::BitcoinCash_testnet3:
            case blockchain::Type::Litecoin:
            case blockchain::Type::Litecoin_testnet4:
            case blockchain::Type::UnitTest: {

                return std::make_shared<Block>(
                    api,
                    chain,
                    std::move(header),
                    std::move(index),
                    std::move(map));
            }
            case blockchain::Type::PKT:
            case blockchain::Type::PKT_testnet: {
                // TODO
                return {};
            }
            case blockchain::Type::Unknown:
            case blockchain::Type::Ethereum_frontier:
            case blockchain::Type::Ethereum_ropsten:
            default: {
                LogError()("opentxs::factory::")(__func__)(
                    ": Unsupported type (")(static_cast<std::uint32_t>(chain))(
                    ")")
                    .Flush();

                return {};
            }
        }
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
auto BitcoinBlock(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView in) noexcept
    -> std::shared_ptr<blockchain::block::bitcoin::Block>
{
    try {
        switch (chain) {
            case blockchain::Type::Bitcoin:
            case blockchain::Type::Bitcoin_testnet3:
            case blockchain::Type::BitcoinCash:
            case blockchain::Type::BitcoinCash_testnet3:
            case blockchain::Type::Litecoin:
            case blockchain::Type::Litecoin_testnet4:
            case blockchain::Type::UnitTest: {
                return parse_normal_block(api, chain, in);
            }
            case blockchain::Type::PKT:
            case blockchain::Type::PKT_testnet: {
                return parse_pkt_block(api, chain, in);
            }
            case blockchain::Type::Unknown:
            case blockchain::Type::Ethereum_frontier:
            case blockchain::Type::Ethereum_ropsten:
            default: {
                LogError()("opentxs::factory::")(__func__)(
                    ": Unsupported type (")(static_cast<std::uint32_t>(chain))(
                    ")")
                    .Flush();

                return {};
            }
        }
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto parse_normal_block(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView in) noexcept(false)
    -> std::shared_ptr<blockchain::block::bitcoin::Block>
{
    OT_ASSERT(
        (blockchain::Type::PKT != chain) &&
        (blockchain::Type::PKT_testnet != chain));

    auto it = ByteIterator{};
    auto expectedSize = std::size_t{};
    auto pHeader = parse_header(api, chain, in, it, expectedSize);

    OT_ASSERT(pHeader);

    const auto& header = *pHeader;
    auto sizeData = BlockReturnType::CalculatedSize{
        in.size(), network::blockchain::bitcoin::CompactSize{}};
    auto [index, transactions] =
        parse_transactions(api, chain, in, header, sizeData, it, expectedSize);

    return std::make_shared<BlockReturnType>(
        api,
        chain,
        std::move(pHeader),
        std::move(index),
        std::move(transactions),
        std::move(sizeData));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::bitcoin::implementation
{
const std::size_t Block::header_bytes_{80};
const Block::value_type Block::null_tx_{};

Block::Block(
    const api::Session& api,
    const blockchain::Type chain,
    std::unique_ptr<const internal::Header> header,
    TxidIndex&& index,
    TransactionMap&& transactions,
    std::optional<CalculatedSize>&& size) noexcept(false)
    : block::implementation::Block(api, *header)
    , header_p_(std::move(header))
    , header_(*header_p_)
    , index_(std::move(index))
    , transactions_(std::move(transactions))
    , size_(std::move(size))
{
    if (index_.size() != transactions_.size()) {
        throw std::runtime_error("Invalid transaction index");
    }

    if (false == bool(header_p_)) {
        throw std::runtime_error("Invalid header");
    }

    for (const auto& [txid, tx] : transactions_) {
        if (false == bool(tx)) {
            throw std::runtime_error("Invalid transaction");
        }
    }
}

auto Block::at(const std::size_t index) const noexcept -> const value_type&
{
    try {
        if (index_.size() <= index) {
            throw std::out_of_range("invalid index " + std::to_string(index));
        }

        return at(reader(index_.at(index)));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return null_tx_;
    }
}

auto Block::at(const ReadView txid) const noexcept -> const value_type&
{
    try {

        return transactions_.at(txid);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("transaction ")(
            api_.Factory().Data(txid)->asHex())(" not found in block ")(
            header_.Hash().asHex())
            .Flush();

        return null_tx_;
    }
}

template <typename HashType>
auto Block::calculate_merkle_hash(
    const api::Session& api,
    const Type chain,
    const HashType& lhs,
    const HashType& rhs,
    AllocateOutput out) -> bool
{
    auto preimage = std::array<std::byte, 64>{};
    constexpr auto chunk = preimage.size() / 2u;

    if (chunk != lhs.size()) {
        throw std::runtime_error("Invalid lhs hash size");
    }
    if (chunk != rhs.size()) {
        throw std::runtime_error("Invalid rhs hash size");
    }

    auto it = preimage.data();
    std::memcpy(it, lhs.data(), chunk);
    std::advance(it, chunk);
    std::memcpy(it, rhs.data(), chunk);

    return MerkleHash(
        api,
        chain,
        {reinterpret_cast<const char*>(preimage.data()), preimage.size()},
        out);
}

template <typename InputContainer, typename OutputContainer>
auto Block::calculate_merkle_row(
    const api::Session& api,
    const Type chain,
    const InputContainer& in,
    OutputContainer& out) -> bool
{
    out.clear();
    const auto count{in.size()};

    for (auto i = std::size_t{0}; i < count; i += 2u) {
        const auto offset = std::size_t{(1u == (count - i)) ? 0u : 1u};
        auto& next = out.emplace_back();
        const auto hashed = calculate_merkle_hash(
            api,
            chain,
            in.at(i),
            in.at(i + offset),
            preallocated(next.size(), next.data()));

        if (false == hashed) { return false; }
    }

    return true;
}

auto Block::calculate_merkle_value(
    const api::Session& api,
    const Type chain,
    const TxidIndex& txids) -> block::pHash
{
    using Hash = std::array<std::byte, 32>;

    if (0 == txids.size()) {
        constexpr auto blank = Hash{};

        return api.Factory().Data(ReadView{
            reinterpret_cast<const char*>(blank.data()), blank.size()});
    }

    if (1 == txids.size()) { return api.Factory().Data(txids.at(0)); }

    auto a = UnallocatedVector<Hash>{};
    auto b = UnallocatedVector<Hash>{};
    a.reserve(txids.size());
    b.reserve(txids.size());
    auto counter{0};
    calculate_merkle_row(api, chain, txids, a);

    if (1u == a.size()) { return api.Factory().Data(reader(a.at(0))); }

    while (true) {
        const auto& src = (1 == (++counter % 2)) ? a : b;
        auto& dst = (0 == (counter % 2)) ? a : b;
        calculate_merkle_row(api, chain, src, dst);

        if (1u == dst.size()) { return api.Factory().Data(reader(dst.at(0))); }
    }
}

auto Block::calculate_size() const noexcept -> CalculatedSize
{
    auto output = CalculatedSize{
        0, network::blockchain::bitcoin::CompactSize(transactions_.size())};
    auto& [bytes, cs] = output;
    auto cb = [](const auto& previous, const auto& in) -> std::size_t {
        return previous + in.second->Internal().CalculateSize();
    };
    bytes = std::accumulate(
        std::begin(transactions_),
        std::end(transactions_),
        header_bytes_ + cs.Size() + extra_bytes(),
        cb);

    return output;
}

auto Block::ExtractElements(const filter::Type style) const noexcept
    -> UnallocatedVector<Space>
{
    auto output = UnallocatedVector<Space>{};
    LogTrace()(OT_PRETTY_CLASS())("processing ")(transactions_.size())(
        " transactions")
        .Flush();

    for (const auto& [txid, tx] : transactions_) {
        auto temp = tx->Internal().ExtractElements(style);
        output.insert(
            output.end(),
            std::make_move_iterator(temp.begin()),
            std::make_move_iterator(temp.end()));
    }

    LogTrace()(OT_PRETTY_CLASS())("extracted ")(output.size())(" elements")
        .Flush();
    std::sort(output.begin(), output.end());

    return output;
}

auto Block::FindMatches(
    const filter::Type style,
    const Patterns& outpoints,
    const Patterns& patterns) const noexcept -> Matches
{
    if (0 == (outpoints.size() + patterns.size())) { return {}; }

    LogTrace()(OT_PRETTY_CLASS())("Verifying ")(
        patterns.size() + outpoints.size())(" potential matches in ")(
        transactions_.size())(" transactions")
        .Flush();
    auto output = Matches{};
    auto& [inputs, outputs] = output;
    const auto parsed = block::ParsedPatterns{patterns};

    for (const auto& [txid, tx] : transactions_) {
        auto temp = tx->Internal().FindMatches(style, outpoints, parsed);
        inputs.insert(
            inputs.end(),
            std::make_move_iterator(temp.first.begin()),
            std::make_move_iterator(temp.first.end()));
        outputs.insert(
            outputs.end(),
            std::make_move_iterator(temp.second.begin()),
            std::make_move_iterator(temp.second.end()));
    }

    dedup(inputs);
    dedup(outputs);

    return output;
}

auto Block::get_or_calculate_size() const noexcept -> CalculatedSize
{
    if (false == size_.has_value()) { size_ = calculate_size(); }

    OT_ASSERT(size_.has_value());

    return size_.value();
}

auto Block::Print() const noexcept -> UnallocatedCString
{
    auto out = std::stringstream{};
    out << "header" << '\n' << header_.Print();
    auto count{0};
    const auto total = size();

    for (const auto& tx : *this) {
        out << "transaction " << std::to_string(++count);
        out << " of " << std::to_string(total) << '\n';
        out << tx->Print();
    }

    return out.str();
}

auto Block::Serialize(AllocateOutput bytes) const noexcept -> bool
{
    if (false == bool(bytes)) {
        LogError()(OT_PRETTY_CLASS())("Invalid output allocator").Flush();

        return false;
    }

    const auto [size, txCount] = get_or_calculate_size();
    const auto out = bytes(size);

    if (false == out.valid(size)) {
        LogError()(OT_PRETTY_CLASS())("Failed to allocate output").Flush();

        return false;
    }

    LogInsane()(OT_PRETTY_CLASS())("Serializing ")(txCount.Value())(
        " transactions into ")(size)(" bytes.")
        .Flush();
    auto remaining = std::size_t{size};
    auto it = static_cast<std::byte*>(out.data());

    if (false == header_.Serialize(preallocated(remaining, it))) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize header").Flush();

        return false;
    }

    remaining -= header_bytes_;
    std::advance(it, header_bytes_);

    if (false == serialize_post_header(it, remaining)) {
        LogError()(OT_PRETTY_CLASS())("Failed to extra data (post header)")
            .Flush();

        return false;
    }

    if (false == txCount.Encode(preallocated(remaining, it))) {
        LogError()(OT_PRETTY_CLASS())("Failed to serialize transaction count")
            .Flush();

        return false;
    }

    remaining -= txCount.Size();
    std::advance(it, txCount.Size());

    for (const auto& txid : index_) {
        try {
            const auto& pTX = transactions_.at(reader(txid));

            OT_ASSERT(pTX);

            const auto& tx = *pTX;
            const auto encoded =
                tx.Internal().Serialize(preallocated(remaining, it));

            if (false == encoded.has_value()) {
                LogError()(OT_PRETTY_CLASS())(
                    "failed to serialize transaction ")(tx.ID().asHex())
                    .Flush();

                return false;
            }

            remaining -= encoded.value();
            std::advance(it, encoded.value());
        } catch (...) {
            LogError()(OT_PRETTY_CLASS())("missing transaction").Flush();

            return false;
        }
    }

    if (0 != remaining) {
        LogError()(OT_PRETTY_CLASS())("Extra bytes: ")(remaining).Flush();

        return false;
    }

    return true;
}

auto Block::serialize_post_header(
    [[maybe_unused]] ByteIterator& it,
    [[maybe_unused]] std::size_t& remaining) const noexcept -> bool
{
    return true;
}

Block::~Block() = default;
}  // namespace opentxs::blockchain::block::bitcoin::implementation
