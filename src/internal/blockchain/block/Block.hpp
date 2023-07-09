// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/Transaction.hpp"

#pragma once

#include <cstddef>
#include <span>

#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
class Session;
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
}  // namespace internal
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Hash;
class Header;
class Transaction;
}  // namespace block
}  // namespace blockchain

class Log;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::internal
{
class Block
{
public:
    virtual auto asBitcoin() const noexcept
        -> const bitcoin::block::internal::Block&;
    virtual auto CalculateSize() const noexcept -> std::size_t;
    virtual auto ContainsHash(const TransactionHash& hash) const noexcept
        -> bool;
    virtual auto ContainsID(const TransactionHash& id) const noexcept -> bool;
    virtual auto ExtractElements(
        const cfilter::Type style,
        alloc::Strategy alloc) const noexcept -> Elements;
    virtual auto FindByHash(const TransactionHash& hash) const noexcept
        -> const block::Transaction&;
    virtual auto FindByID(const TransactionHash& id) const noexcept
        -> const block::Transaction&;
    virtual auto FindMatches(
        const api::Session& api,
        const cfilter::Type type,
        const Patterns& txos,
        const Patterns& elements,
        const Log& log,
        alloc::Strategy alloc) const noexcept -> Matches;
    virtual auto get() const noexcept -> std::span<const block::Transaction>;
    virtual auto Header() const noexcept -> const block::Header&;
    virtual auto ID() const noexcept -> const block::Hash&;
    virtual auto IsValid() const noexcept -> bool;
    virtual auto Print(const api::Crypto& crypto) const noexcept
        -> UnallocatedCString;
    virtual auto Print(const api::Crypto& crypto, alloc::Strategy alloc)
        const noexcept -> CString;
    virtual auto Serialize(Writer&& bytes) const noexcept -> bool;
    virtual auto size() const noexcept -> std::size_t;

    virtual auto asBitcoin() noexcept -> bitcoin::block::internal::Block&;

    virtual ~Block() = default;
};
}  // namespace opentxs::blockchain::block::internal
