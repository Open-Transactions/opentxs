// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/Transaction.hpp"

#pragma once

#include <compare>
#include <cstddef>
#include <span>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

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
class Block;
}  // namespace block
}  // namespace bitcoin

namespace block
{
namespace internal
{
class Block;
}  // namespace internal

class Block;
class BlockPrivate;
class Hash;
class Header;
class Transaction;
class TransactionHash;
}  // namespace block
}  // namespace blockchain

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::blockchain::block::Block> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::blockchain::block::Block& data)
        const noexcept -> std::size_t;
};
}  // namespace std

namespace opentxs::blockchain::block
{
OPENTXS_EXPORT auto operator==(const Block&, const Block&) noexcept -> bool;
OPENTXS_EXPORT auto operator<=>(const Block&, const Block&) noexcept
    -> std::strong_ordering;
OPENTXS_EXPORT auto swap(Block&, Block&) noexcept -> void;
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block
{
class OPENTXS_EXPORT Block : virtual public opentxs::Allocated
{
public:
    OPENTXS_NO_EXPORT static auto Blank() noexcept -> Block&;

    [[nodiscard]] operator bool() const noexcept { return IsValid(); }
    operator std::span<const Transaction>() const noexcept { return get(); }

    auto asBitcoin() const& noexcept -> const bitcoin::block::Block&;
    auto ContainsHash(const TransactionHash& hash) const noexcept -> bool;
    auto ContainsID(const TransactionHash& id) const noexcept -> bool;
    auto FindByHash(const TransactionHash& hash) const noexcept
        -> const Transaction&;
    auto FindByID(const TransactionHash& id) const noexcept
        -> const Transaction&;
    auto get() const noexcept -> std::span<const Transaction>;
    auto get_allocator() const noexcept -> allocator_type final;
    auto Header() const noexcept -> const block::Header&;
    auto ID() const noexcept -> const block::Hash&;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Block&;
    [[nodiscard]] auto IsValid() const noexcept -> bool;
    auto Print(const api::Crypto& crypto) const noexcept -> UnallocatedCString;
    auto Print(const api::Crypto& crypto, allocator_type alloc) const noexcept
        -> CString;
    auto Serialize(Writer&& bytes) const noexcept -> bool;
    auto size() const noexcept -> std::size_t;

    auto asBitcoin() & noexcept -> bitcoin::block::Block&;
    auto asBitcoin() && noexcept -> bitcoin::block::Block;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Block&;
    auto swap(Block& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Block(BlockPrivate* imp) noexcept;
    Block(allocator_type alloc = {}) noexcept;
    Block(const Block& rhs, allocator_type alloc = {}) noexcept;
    Block(Block&& rhs) noexcept;
    Block(Block&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Block& rhs) noexcept -> Block&;
    auto operator=(Block&& rhs) noexcept -> Block&;

    ~Block() override;

protected:
    friend BlockPrivate;

    BlockPrivate* imp_;
};
}  // namespace opentxs::blockchain::block
