// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Header;
}  // namespace internal

class Block;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::block::pkt
{
class Block final : public bitcoin::implementation::Block
{
public:
    using Proof = std::pair<std::byte, Space>;
    using Proofs = std::pmr::vector<Proof>;

    auto GetProofs() const noexcept -> const Proofs& { return proofs_; }

    Block(
        const api::Session& api,
        const blockchain::Type chain,
        std::unique_ptr<const bitcoin::internal::Header> header,
        Proofs&& proofs,
        TxidIndex&& index,
        TransactionMap&& transactions,
        std::optional<std::size_t>&& proofBytes = {},
        std::optional<CalculatedSize>&& size = {}) noexcept(false);

    ~Block() final;

private:
    using ot_super = bitcoin::implementation::Block;

    const Proofs proofs_;
    mutable std::optional<std::size_t> proof_bytes_;

    auto extra_bytes() const noexcept -> std::size_t final;
    auto serialize_post_header(ByteIterator& it, std::size_t& remaining)
        const noexcept -> bool final;

    Block() = delete;
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;
};
}  // namespace opentxs::blockchain::block::pkt
