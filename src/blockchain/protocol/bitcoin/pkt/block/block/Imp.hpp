// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <optional>

#include "blockchain/block/block/BlockPrivate.hpp"
#include "blockchain/protocol/bitcoin/base/block/block/Imp.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "internal/blockchain/protocol/bitcoin/pkt/block/Types.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
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

class WriteBuffer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::pkt::block::implementation
{
class Block final
    : public blockchain::protocol::bitcoin::base::block::implementation::Block
{
public:
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> blockchain::block::BlockPrivate* final
    {
        return pmr::clone_as<blockchain::block::BlockPrivate>(this, {alloc});
    }
    auto GetProofs() const noexcept -> const Proofs& { return proofs_; }

    [[nodiscard]] auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Block(
        const blockchain::Type chain,
        block::Header header,
        Proofs&& proofs,
        TxidIndex&& ids,
        TxidIndex&& hashes,
        TransactionMap&& transactions,
        std::optional<std::size_t>&& proofBytes,
        std::optional<CalculatedSize>&& size,
        allocator_type alloc) noexcept(false);
    Block() = delete;
    Block(const Block& rhs, allocator_type alloc) noexcept;
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;

    ~Block() final;

private:
    const Proofs proofs_;
    mutable std::optional<std::size_t> proof_bytes_;

    auto extra_bytes() const noexcept -> std::size_t final;
    auto serialize_aux_pow(WriteBuffer& out) const noexcept -> bool final;
};
}  // namespace
   // opentxs::blockchain::protocol::bitcoin::pkt::block::implementation
