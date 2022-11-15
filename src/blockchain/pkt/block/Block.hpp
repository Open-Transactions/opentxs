// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>

#include "blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
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
}  // namespace blockchain

class WriteBuffer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::pkt::block
{
class Block final : public blockchain::bitcoin::block::implementation::Block
{
public:
    using Proof = std::pair<std::byte, Space>;
    using Proofs = Vector<Proof>;

    auto clone_bitcoin() const noexcept
        -> std::unique_ptr<bitcoin::block::internal::Block> final;
    auto GetProofs() const noexcept -> const Proofs& { return proofs_; }

    Block(
        const blockchain::Type chain,
        std::unique_ptr<const blockchain::bitcoin::block::Header> header,
        Proofs&& proofs,
        TxidIndex&& index,
        TransactionMap&& transactions,
        std::optional<std::size_t>&& proofBytes = {},
        std::optional<CalculatedSize>&& size = {}) noexcept(false);
    Block() = delete;
    Block(const Block& rhs) noexcept;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;

    ~Block() final;

private:
    using ot_super = blockchain::bitcoin::block::implementation::Block;

    const Proofs proofs_;
    mutable std::optional<std::size_t> proof_bytes_;

    auto extra_bytes() const noexcept -> std::size_t final;
    auto serialize_post_header(WriteBuffer& out) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::pkt::block
