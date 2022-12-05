// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "blockchain/pkt/block/Block.hpp"  // IWYU pragma: associated

#include <iterator>
#include <numeric>
#include <stdexcept>

#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::pkt::block
{
Block::Block(
    const blockchain::Type chain,
    std::unique_ptr<const blockchain::bitcoin::block::Header> header,
    Proofs&& proofs,
    TxidIndex&& index,
    TransactionMap&& transactions,
    std::optional<std::size_t>&& proofBytes,
    std::optional<CalculatedSize>&& size) noexcept(false)
    : ot_super(
          chain,
          std::move(header),
          std::move(index),
          std::move(transactions),
          std::move(size))
    , proofs_(std::move(proofs))
    , proof_bytes_(std::move(proofBytes))
{
}

Block::Block(const Block& rhs) noexcept
    : ot_super(rhs)
    , proofs_(rhs.proofs_)
    , proof_bytes_(rhs.proof_bytes_)
{
}

auto Block::clone_bitcoin() const noexcept
    -> std::unique_ptr<bitcoin::block::internal::Block>
{
    return std::make_unique<Block>(*this);
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
        proof_bytes_ =
            std::accumulate(std::begin(proofs_), std::end(proofs_), 0_uz, cb);
    }

    OT_ASSERT(proof_bytes_.has_value());

    return proof_bytes_.value();
}

auto Block::serialize_post_header(WriteBuffer& out) const noexcept -> bool
{
    try {
        for (const auto& [type, proof] : proofs_) {
            serialize_object(type, out, "proof type");
            serialize_compact_size(proof.size(), out, "proof size");
            copy(reader(proof), out, "proof");
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

Block::~Block() = default;
}  // namespace opentxs::blockchain::pkt::block
