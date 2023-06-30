// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <optional>

#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/pkt/block/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Header;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class BlockPrivate;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
[[nodiscard]] auto PktBlock(
    const blockchain::Type chain,
    blockchain::pkt::block::Header header,
    blockchain::pkt::block::Proofs&& proofs,
    blockchain::pkt::block::TxidIndex&& ids,
    blockchain::pkt::block::TxidIndex&& hashes,
    blockchain::pkt::block::TransactionMap&& transactions,
    std::optional<std::size_t>&& proofBytes,
    std::optional<blockchain::pkt::block::CalculatedSize>&& size,
    alloc::Strategy alloc) noexcept -> blockchain::block::BlockPrivate*;
}  // namespace opentxs::factory
