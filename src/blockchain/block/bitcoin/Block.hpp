// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "blockchain/block/Block.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
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

class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Block : public bitcoin::Block, public block::implementation::Block
{
public:
    using CalculatedSize =
        std::pair<std::size_t, network::blockchain::bitcoin::CompactSize>;
    using TxidIndex = std::pmr::vector<Space>;
    using TransactionMap = std::pmr::map<ReadView, value_type>;

    static const std::size_t header_bytes_;

    template <typename HashType>
    static auto calculate_merkle_hash(
        const api::Session& api,
        const Type chain,
        const HashType& lhs,
        const HashType& rhs,
        AllocateOutput out) -> bool;
    template <typename InputContainer, typename OutputContainer>
    static auto calculate_merkle_row(
        const api::Session& api,
        const Type chain,
        const InputContainer& in,
        OutputContainer& out) -> bool;
    static auto calculate_merkle_value(
        const api::Session& api,
        const Type chain,
        const TxidIndex& txids) -> block::pHash;

    auto at(const std::size_t index) const noexcept -> const value_type& final;
    auto at(const ReadView txid) const noexcept -> const value_type& final;
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
    auto CalculateSize() const noexcept -> std::size_t final
    {
        return get_or_calculate_size().first;
    }
    auto cbegin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const noexcept -> const_iterator final
    {
        return const_iterator(this, index_.size());
    }
    auto end() const noexcept -> const_iterator final { return cend(); }
    auto ExtractElements(const filter::Type style) const noexcept
        -> std::pmr::vector<Space> final;
    auto FindMatches(
        const filter::Type type,
        const Patterns& outpoints,
        const Patterns& scripts) const noexcept -> Matches final;
    auto Print() const noexcept -> std::string override;
    auto Serialize(AllocateOutput bytes) const noexcept -> bool final;
    auto size() const noexcept -> std::size_t final { return index_.size(); }

    Block(
        const api::Session& api,
        const blockchain::Type chain,
        std::unique_ptr<const internal::Header> header,
        TxidIndex&& index,
        TransactionMap&& transactions,
        std::optional<CalculatedSize>&& size = {}) noexcept(false);
    ~Block() override;

protected:
    using ByteIterator = std::byte*;

private:
    static const value_type null_tx_;

    const std::unique_ptr<const internal::Header> header_p_;
    const internal::Header& header_;
    const TxidIndex index_;
    const TransactionMap transactions_;
    mutable std::optional<CalculatedSize> size_;

    auto calculate_size() const noexcept -> CalculatedSize;
    virtual auto extra_bytes() const noexcept -> std::size_t { return 0; }
    auto get_or_calculate_size() const noexcept -> CalculatedSize;
    virtual auto serialize_post_header(ByteIterator& it, std::size_t& remaining)
        const noexcept -> bool;

    Block() = delete;
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
