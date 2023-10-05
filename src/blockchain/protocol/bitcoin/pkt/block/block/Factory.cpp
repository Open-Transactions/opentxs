// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/protocol/bitcoin/pkt/block/Factory.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "blockchain/protocol/bitcoin/base/block/block/BlockPrivate.hpp"
#include "blockchain/protocol/bitcoin/pkt/block/block/Imp.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Header.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto PktBlock(
    const blockchain::Type chain,
    blockchain::protocol::bitcoin::pkt::block::Header header,
    blockchain::protocol::bitcoin::pkt::block::Proofs&& proofs,
    blockchain::protocol::bitcoin::pkt::block::TxidIndex&& ids,
    blockchain::protocol::bitcoin::pkt::block::TxidIndex&& hashes,
    blockchain::protocol::bitcoin::pkt::block::TransactionMap&& transactions,
    std::optional<std::size_t>&& proofBytes,
    std::optional<blockchain::protocol::bitcoin::pkt::block::CalculatedSize>&&
        size,
    alloc::Strategy alloc) noexcept -> blockchain::block::BlockPrivate*
{
    using ReturnType =
        blockchain::protocol::bitcoin::pkt::block::implementation::Block;
    using BlankType = blockchain::protocol::bitcoin::base::block::BlockPrivate;

    try {

        return pmr::construct<ReturnType>(
            alloc.result_,
            chain,
            std::move(header),
            std::move(proofs),
            std::move(ids),
            std::move(hashes),
            std::move(transactions),
            std::move(proofBytes),
            std::nullopt);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return pmr::default_construct<BlankType>(alloc.result_);
    }
}
}  // namespace opentxs::factory
