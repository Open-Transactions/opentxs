// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "blockchain/block/pkt/Block.hpp"  // IWYU pragma: associated

#include <cstring>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <type_traits>

#include "blockchain/block/bitcoin/BlockParser.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto parse_pkt_block(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView in) noexcept(false)
    -> std::shared_ptr<blockchain::block::bitcoin::Block>
{
    using ReturnType = blockchain::block::pkt::Block;

    OT_ASSERT(
        (blockchain::Type::PKT == chain) ||
        (blockchain::Type::PKT_testnet == chain));

    auto it = ByteIterator{};
    auto expectedSize = std::size_t{};
    auto pHeader = parse_header(api, chain, in, it, expectedSize);

    OT_ASSERT(pHeader);

    const auto& header = *pHeader;
    const auto proofStart{it};
    auto proofs = ReturnType::Proofs{};

    while (true) {
        expectedSize += 1;

        if (in.size() < expectedSize) {
            throw std::runtime_error("Block size too short (proof type)");
        }

        auto& proof = proofs.emplace_back(*it, Space{});
        expectedSize += 1;
        std::advance(it, 1);

        if (in.size() < expectedSize) {
            throw std::runtime_error(
                "Block size too short (proof compact size)");
        }

        auto proofCS = network::blockchain::bitcoin::CompactSize{};

        if (false == network::blockchain::bitcoin::DecodeSize(
                         it, expectedSize, in.size(), proofCS)) {
            throw std::runtime_error("Failed to decode proof size");
        }

        const auto proofBytes{proofCS.Value()};
        expectedSize += proofBytes;

        if (in.size() < expectedSize) {
            throw std::runtime_error("Block size too short (proof)");
        }

        auto& [type, data] = proof;
        data = Space{it, it + proofBytes};
        std::advance(it, proofBytes);

        static constexpr auto terminalType = std::byte{0x0};

        if (type == terminalType) { break; }
    }

    const auto proofEnd{it};
    auto sizeData = ReturnType::CalculatedSize{
        in.size(), network::blockchain::bitcoin::CompactSize{}};
    auto [index, transactions] =
        parse_transactions(api, chain, in, header, sizeData, it, expectedSize);

    return std::make_shared<ReturnType>(
        api,
        chain,
        std::move(pHeader),
        std::move(proofs),
        std::move(index),
        std::move(transactions),
        static_cast<std::size_t>(std::distance(proofStart, proofEnd)),
        std::move(sizeData));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::pkt
{
Block::Block(
    const api::Session& api,
    const blockchain::Type chain,
    std::unique_ptr<const bitcoin::internal::Header> header,
    Proofs&& proofs,
    TxidIndex&& index,
    TransactionMap&& transactions,
    std::optional<std::size_t>&& proofBytes,
    std::optional<CalculatedSize>&& size) noexcept(false)
    : ot_super(
          api,
          chain,
          std::move(header),
          std::move(index),
          std::move(transactions),
          std::move(size))
    , proofs_(std::move(proofs))
    , proof_bytes_(std::move(proofBytes))
{
}

auto Block::extra_bytes() const noexcept -> std::size_t
{
    if (false == proof_bytes_.has_value()) {
        auto cb = [](const auto& previous, const auto& in) -> std::size_t {
            const auto& [type, proof] = in;
            const auto cs =
                network::blockchain::bitcoin::CompactSize{proof.size()};

            return previous + sizeof(type) + cs.Total();
        };
        proof_bytes_ = std::accumulate(
            std::begin(proofs_), std::end(proofs_), std::size_t{0}, cb);
    }

    OT_ASSERT(proof_bytes_.has_value());

    return proof_bytes_.value();
}

auto Block::serialize_post_header(ByteIterator& it, std::size_t& remaining)
    const noexcept -> bool
{
    for (const auto& [type, proof] : proofs_) {
        if (remaining < sizeof(type)) {
            LogError()(OT_PRETTY_CLASS())("Failed to serialize proof type")
                .Flush();

            return false;
        }

        {
            const auto size{sizeof(type)};
            std::memcpy(it, &type, size);
            remaining -= size;
            std::advance(it, size);
        }

        const auto cs = network::blockchain::bitcoin::CompactSize{proof.size()};

        if (false == cs.Encode(preallocated(remaining, it))) {
            LogError()(OT_PRETTY_CLASS())("Failed to serialize proof size")
                .Flush();

            return false;
        }

        {
            const auto size{cs.Size()};
            remaining -= size;
            std::advance(it, size);
        }

        if (remaining < cs.Value()) {
            LogError()(OT_PRETTY_CLASS())("Failed to serialize proof").Flush();

            return false;
        }

        {
            const auto size{proof.size()};

            if (0u < size) { std::memcpy(it, proof.data(), size); }

            remaining -= size;
            std::advance(it, size);
        }
    }

    return true;
}

Block::~Block() = default;
}  // namespace opentxs::blockchain::block::pkt
