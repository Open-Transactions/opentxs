// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "internal/blockchain/pkt/block/Factory.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/block/block/BlockPrivate.hpp"
#include "blockchain/pkt/block/block/Imp.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto PktBlock(
    const blockchain::Type chain,
    blockchain::pkt::block::Header header,
    blockchain::pkt::block::Proofs&& proofs,
    blockchain::pkt::block::TxidIndex&& ids,
    blockchain::pkt::block::TxidIndex&& hashes,
    blockchain::pkt::block::TransactionMap&& transactions,
    std::optional<std::size_t>&& proofBytes,
    std::optional<blockchain::pkt::block::CalculatedSize>&& size,
    alloc::Default alloc) noexcept -> blockchain::block::BlockPrivate*
{
    using ReturnType = blockchain::pkt::block::implementation::Block;
    using BlankType = blockchain::bitcoin::block::BlockPrivate;
    auto pmr = alloc::PMR<ReturnType>{alloc};
    ReturnType* out = {nullptr};

    try {
        out = pmr.allocate(1_uz);
        pmr.construct(
            out,
            chain,
            std::move(header),
            std::move(proofs),
            std::move(ids),
            std::move(hashes),
            std::move(transactions),
            std::move(proofBytes),
            std::nullopt);

        return out;
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        if (nullptr != out) { pmr.deallocate(out, 1_uz); }

        auto fallback = alloc::PMR<BlankType>{alloc};
        auto* blank = fallback.allocate(1_uz);

        OT_ASSERT(nullptr != blank);

        fallback.construct(blank);

        return blank;
    }
}
}  // namespace opentxs::factory
