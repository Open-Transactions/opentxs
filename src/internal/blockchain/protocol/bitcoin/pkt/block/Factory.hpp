// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <optional>

#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "internal/blockchain/protocol/bitcoin/pkt/block/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.internal.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class BlockPrivate;
}  // namespace block

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
class Header;
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
[[nodiscard]] auto PktBlock(
    const blockchain::Type chain,
    blockchain::protocol::bitcoin::pkt::block::Header header,
    blockchain::protocol::bitcoin::pkt::block::Proofs&& proofs,
    blockchain::protocol::bitcoin::pkt::block::TxidIndex&& ids,
    blockchain::protocol::bitcoin::pkt::block::TxidIndex&& hashes,
    blockchain::protocol::bitcoin::pkt::block::TransactionMap&& transactions,
    std::optional<std::size_t>&& proofBytes,
    std::optional<blockchain::protocol::bitcoin::pkt::block::CalculatedSize>&&
        size,
    alloc::Strategy alloc) noexcept -> blockchain::block::BlockPrivate*;
}  // namespace opentxs::factory
