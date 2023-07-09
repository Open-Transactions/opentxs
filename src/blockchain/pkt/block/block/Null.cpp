// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/pkt/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/block/block/BlockPrivate.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"

namespace opentxs::factory
{
auto PktBlock(
    const blockchain::Type,
    blockchain::pkt::block::Header,
    blockchain::pkt::block::Proofs&&,
    blockchain::pkt::block::TxidIndex&&,
    blockchain::pkt::block::TxidIndex&&,
    blockchain::pkt::block::TransactionMap&&,
    std::optional<std::size_t>&&,
    std::optional<blockchain::pkt::block::CalculatedSize>&&,
    alloc::Strategy alloc) noexcept -> blockchain::block::BlockPrivate*
{
    auto pmr = alloc::PMR<blockchain::block::BlockPrivate>{alloc.result_};
    auto* out = pmr.allocate(1_uz);
    pmr.construct(out);

    return out;
}
}  // namespace opentxs::factory
