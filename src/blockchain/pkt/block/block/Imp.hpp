// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>

#include "blockchain/bitcoin/block/block/Imp.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/pkt/block/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Block;
class Header;
}  // namespace internal

class Block;
class Header;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class BlockPrivate;
}  // namespace block
}  // namespace blockchain

class WriteBuffer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::pkt::block::implementation
{
class Block final : public bitcoin::block::implementation::Block
{
public:
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> Block* final;
    auto GetProofs() const noexcept -> const Proofs& { return proofs_; }

    [[nodiscard]] auto get_deleter() noexcept -> std::function<void()> final;

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
}  // namespace opentxs::blockchain::pkt::block::implementation