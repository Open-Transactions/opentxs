// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/pkt/block/block/Imp.hpp"  // IWYU pragma: associated

#include <iterator>
#include <numeric>
#include <stdexcept>
#include <utility>

#include "blockchain/block/block/BlockPrivate.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::pkt::block::implementation
{
Block::Block(
    const blockchain::Type chain,
    blockchain::bitcoin::block::Header header,
    Proofs&& proofs,
    TxidIndex&& ids,
    TxidIndex&& hashes,
    TransactionMap&& transactions,
    std::optional<std::size_t>&& proofBytes,
    std::optional<CalculatedSize>&& size,
    allocator_type alloc) noexcept(false)
    : blockchain::block::BlockPrivate(alloc)
    , bitcoin::block::implementation::Block(
          chain,
          std::move(header),
          std::move(ids),
          std::move(hashes),
          std::move(transactions),
          std::move(size),
          alloc)
    , proofs_(std::move(proofs), alloc)
    , proof_bytes_(std::move(proofBytes))
{
}

Block::Block(const Block& rhs, allocator_type alloc) noexcept
    : blockchain::block::BlockPrivate(rhs, alloc)
    , bitcoin::block::implementation::Block(rhs, alloc)
    , proofs_(rhs.proofs_, alloc)
    , proof_bytes_(rhs.proof_bytes_)
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
        proof_bytes_ =
            std::accumulate(std::begin(proofs_), std::end(proofs_), 0_uz, cb);
    }

    OT_ASSERT(proof_bytes_.has_value());

    return proof_bytes_.value();
}

auto Block::serialize_aux_pow(WriteBuffer& out) const noexcept -> bool
{
    try {
        for (const auto& [type, proof] : proofs_) {
            serialize_object(type, out, "proof type");
            serialize_compact_size(proof.size(), out, "proof size");
            copy(proof.Bytes(), out, "proof");
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

Block::~Block() = default;
}  // namespace opentxs::blockchain::pkt::block::implementation
