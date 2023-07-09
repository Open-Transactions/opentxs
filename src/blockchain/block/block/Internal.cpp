// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/block/Block.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::block::internal
{
auto Block::asBitcoin() const noexcept -> const bitcoin::block::internal::Block&
{
    return bitcoin::block::internal::Block::Blank();
}

auto Block::asBitcoin() noexcept -> bitcoin::block::internal::Block&
{
    return bitcoin::block::internal::Block::Blank();
}

auto Block::CalculateSize() const noexcept -> std::size_t { return {}; }

auto Block::ContainsHash(const TransactionHash&) const noexcept -> bool
{
    return {};
}

auto Block::ContainsID(const TransactionHash&) const noexcept -> bool
{
    return {};
}

auto Block::ExtractElements(const cfilter::Type, alloc::Strategy alloc)
    const noexcept -> Elements
{
    return Elements{alloc.result_};
}

auto Block::FindByHash(const TransactionHash&) const noexcept
    -> const block::Transaction&
{
    return block::Transaction::Blank();
}

auto Block::FindByID(const TransactionHash& id) const noexcept
    -> const block::Transaction&
{
    return FindByHash(id);
}

auto Block::FindMatches(
    const api::Session&,
    const cfilter::Type,
    const Patterns&,
    const Patterns&,
    const Log&,
    alloc::Strategy alloc) const noexcept -> Matches
{
    return std::make_pair(
        InputMatches{alloc.result_}, OutputMatches{alloc.result_});
}

auto Block::get() const noexcept -> std::span<const block::Transaction>
{
    return {};
}

auto Block::Header() const noexcept -> const block::Header&
{
    static const auto blank = block::Header{};

    return blank;
}

auto Block::ID() const noexcept -> const block::Hash&
{
    static const auto blank = block::Hash{};

    return blank;
}

auto Block::IsValid() const noexcept -> bool { return {}; }

auto Block::Print(const api::Crypto&) const noexcept -> UnallocatedCString
{
    return {};
}

auto Block::Print(const api::Crypto&, alloc::Strategy alloc) const noexcept
    -> CString
{
    return CString{alloc.result_};
}

auto Block::Serialize(Writer&&) const noexcept -> bool { return {}; }

auto Block::size() const noexcept -> std::size_t { return {}; }
}  // namespace opentxs::blockchain::block::internal
