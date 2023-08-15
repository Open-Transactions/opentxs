// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/protocol/bitcoin/pkt/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/block/block/BlockPrivate.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Header.hpp"

namespace opentxs::factory
{
auto PktBlock(
    const blockchain::Type,
    blockchain::protocol::bitcoin::pkt::block::Header,
    blockchain::protocol::bitcoin::pkt::block::Proofs&&,
    blockchain::protocol::bitcoin::pkt::block::TxidIndex&&,
    blockchain::protocol::bitcoin::pkt::block::TxidIndex&&,
    blockchain::protocol::bitcoin::pkt::block::TransactionMap&&,
    std::optional<std::size_t>&&,
    std::optional<blockchain::protocol::bitcoin::pkt::block::CalculatedSize>&&,
    alloc::Default alloc) noexcept -> blockchain::block::BlockPrivate*
{
    return pmr::default_construct<blockchain::block::BlockPrivate>(alloc);
}
}  // namespace opentxs::factory
