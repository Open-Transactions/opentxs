// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::Crypto

#include "blockchain/bitcoin/block/block/Imp.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <optional>
#include <span>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#include "TBB.hpp"
#include "blockchain/block/block/BlockPrivate.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Container.hpp"

namespace opentxs::blockchain::bitcoin::block
{
auto CalculateMerkleHash(
    const api::Crypto& crypto,
    const Type chain,
    const Data& lhs,
    const Data& rhs,
    Writer&& out) noexcept(false) -> bool
{
    auto preimage = std::array<std::byte, 64_uz>{};
    constexpr auto chunk = preimage.size() / 2_uz;

    if (chunk != lhs.size()) {
        throw std::runtime_error("Invalid lhs hash size");
    }
    if (chunk != rhs.size()) {
        throw std::runtime_error("Invalid rhs hash size");
    }

    auto* it = preimage.data();
    std::memcpy(it, lhs.data(), chunk);
    std::advance(it, chunk);
    std::memcpy(it, rhs.data(), chunk);

    return MerkleHash(
        crypto,
        chain,
        {reinterpret_cast<const char*>(preimage.data()), preimage.size()},
        std::move(out));
}

auto CalculateMerkleRow(
    const api::Crypto& crypto,
    const Type chain,
    const std::span<const TransactionHash> in,
    Vector<TransactionHash>& out) noexcept(false) -> bool
{
    const auto count{in.size()};
    out.clear();

    for (auto i = 0_uz; i < count; i += 2_uz) {
        const auto offset = (1_uz == (count - i)) ? 0_uz : 1_uz;
        auto& next = out.emplace_back();
        const auto hashed = CalculateMerkleHash(
            crypto, chain, in[i], in[i + offset], next.WriteInto());

        if (false == hashed) { return false; }
    }

    return true;
}

auto CalculateMerkleValue(
    const api::Crypto& crypto,
    const Type chain,
    const std::span<const TransactionHash> txids) noexcept(false) -> block::Hash
{
    const auto count = txids.size();

    switch (count) {
        case 0_uz: {

            return {};
        }
        case 1_uz: {

            return txids.front().Bytes();
        }
        default: {
            // TODO allocator
            auto a = Vector<TransactionHash>{};
            auto b = Vector<TransactionHash>{};
            a.reserve(count);
            b.reserve(count);
            auto counter{0};
            CalculateMerkleRow(crypto, chain, txids, a);

            if (1_uz == a.size()) { return a.front().Bytes(); }

            while (true) {
                const auto dir = (1 == (++counter % 2));
                const auto& src = dir ? a : b;
                auto& dst = dir ? b : a;
                CalculateMerkleRow(crypto, chain, src, dst);

                if (1_uz == dst.size()) { return dst.front().Bytes(); }
            }
        }
    }
}
}  // namespace opentxs::blockchain::bitcoin::block

namespace opentxs::blockchain::bitcoin::block::implementation
{
Block::Block(
    const blockchain::Type chain,
    bitcoin::block::Header header,
    TxidIndex&& ids,
    TxidIndex&& hashes,
    TransactionMap&& transactions,
    std::optional<CalculatedSize> size,
    allocator_type alloc) noexcept(false)
    : blockchain::block::BlockPrivate(alloc)
    , blockchain::block::implementation::Block(
          std::move(header),
          std::move(ids),
          std::move(hashes),
          std::move(transactions),
          alloc)
    , BlockPrivate(alloc)
    , size_(std::move(size))
{
    if (id_index_.size() != transactions_.size()) {
        throw std::runtime_error("Invalid transaction index");
    }

    if (false == header_.IsValid()) {
        throw std::runtime_error("Invalid header");
    }

    for (const auto& tx : transactions_) {
        if (false == tx.IsValid()) {
            throw std::runtime_error("Invalid transaction");
        }
    }
}

Block::Block(const Block& rhs, allocator_type alloc) noexcept
    : Block(
          rhs.header_.Type(),
          rhs.header_.asBitcoin(),
          TxidIndex{rhs.id_index_},    // TODO allocator
          TxidIndex{rhs.hash_index_},  // TODO allocator
          TransactionMap{rhs.transactions_, alloc},
          rhs.size_,
          alloc)
{
}

auto Block::calculate_size() const noexcept -> CalculatedSize
{
    auto output = std::make_pair(
        0_uz, network::blockchain::bitcoin::CompactSize(transactions_.size()));
    auto& [bytes, cs] = output;
    bytes = calculate_size(cs);

    return output;
}

auto Block::calculate_size(const network::blockchain::bitcoin::CompactSize& cs)
    const noexcept -> std::size_t
{
    using Range = tbb::blocked_range<const blockchain::block::Transaction*>;
    const auto& data = transactions_;

    return header_bytes_ + cs.Size() + extra_bytes() +
           tbb::parallel_reduce(
               Range{data.data(), std::next(data.data(), data.size())},
               0_uz,
               [](const Range& r, std::size_t init) {
                   for (const auto& i : r) {
                       init += i.Internal().asBitcoin().CalculateSize();
                   }

                   return init;
               },
               [](std::size_t lhs, std::size_t rhs) { return lhs + rhs; });
}

auto Block::ExtractElements(const cfilter::Type style, alloc::Default alloc)
    const noexcept -> Elements
{
    auto output = Elements{alloc};
    LogTrace()(OT_PRETTY_CLASS())("processing ")(transactions_.size())(
        " transactions")
        .Flush();

    for (const auto& tx : transactions_) {
        tx.Internal().asBitcoin().ExtractElements(style, output);
    }

    LogTrace()(OT_PRETTY_CLASS())("extracted ")(output.size())(" elements")
        .Flush();
    std::sort(output.begin(), output.end());

    return output;
}

auto Block::FindMatches(
    const api::Session& api,
    const cfilter::Type style,
    const Patterns& outpoints,
    const Patterns& patterns,
    const Log& log,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> Matches
{
    if (0 == (outpoints.size() + patterns.size())) { return {}; }

    log(OT_PRETTY_CLASS())("Verifying ")(patterns.size() + outpoints.size())(
        " potential matches in ")(transactions_.size())(
        " transactions of block ")
        .asHex(ID())
        .Flush();
    auto output = std::make_pair(InputMatches{alloc}, OutputMatches{alloc});
    const auto parsed = ParsedPatterns{patterns, monotonic};

    for (const auto& tx : transactions_) {
        tx.Internal().asBitcoin().FindMatches(
            api, style, outpoints, parsed, log, output, monotonic);
    }

    auto& [inputs, outputs] = output;
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
    return Print({}).c_str();
}

auto Block::Print(allocator_type alloc) const noexcept -> CString
{
    auto out = std::stringstream{};
    out << "header" << '\n' << header_.Print();
    auto count{0};
    const auto total = size();

    for (const auto& tx : get()) {
        out << "transaction " << std::to_string(++count);
        out << " of " << std::to_string(total) << '\n';
        out << tx.asBitcoin().Print();
    }

    return {out.str().c_str(), alloc};
}

auto Block::Serialize(Writer&& bytes) const noexcept -> bool
{
    try {
        const auto [size, txCount] = get_or_calculate_size();
        auto buf = reserve(std::move(bytes), size, "block");

        if (false == header_.Serialize(buf.Write(header_bytes_))) {

            throw std::runtime_error{"failed to serialize header"};
        }

        if (false == serialize_aux_pow(buf)) {

            throw std::runtime_error{"failed to serialize aux pow"};
        }

        serialize_compact_size(txCount, buf, "transaction count");

        for (const auto& tx : transactions_) {
            const auto& txid = tx.ID();
            const auto& internal = tx.Internal().asBitcoin();
            const auto expected = internal.CalculateSize();
            const auto wrote = internal.Serialize(buf.Write(expected));

            if ((false == wrote.has_value()) || (*wrote != expected)) {
                const auto error =
                    UnallocatedCString{"failed to serialize transaction "}
                        .append(txid.asHex());

                throw std::runtime_error{error};
            }
        }

        check_finished(buf);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Block::serialize_aux_pow(WriteBuffer&) const noexcept -> bool
{
    return true;
}

Block::~Block() = default;
}  // namespace opentxs::blockchain::bitcoin::block::implementation
