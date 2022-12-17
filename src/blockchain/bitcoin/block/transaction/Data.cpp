// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "blockchain/bitcoin/block/transaction/Data.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <type_traits>
#include <utility>

#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "internal/blockchain/block/Block.hpp"  // IWYU pragma: keep
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::bitcoin::block::transaction
{
Data::Data(
    std::string_view memo,
    Set<blockchain::Type> chains,
    block::Position minedPosition,
    allocator_type alloc) noexcept(false)
    : normalized_id_()
    , size_()
    , normalized_size_()
    , memo_(memo, alloc)
    , chains_(std::move(chains), alloc)
    , mined_position_(std::move(minedPosition))
{
    if (chains_.empty()) { throw std::runtime_error("missing chains"); }
}

Data::Data(const Data& rhs, allocator_type alloc) noexcept
    : normalized_id_(rhs.normalized_id_)
    , size_(rhs.size_)
    , normalized_size_(rhs.normalized_size_)
    , memo_(rhs.memo_, alloc)
    , chains_(rhs.chains_, alloc)
    , mined_position_(rhs.mined_position_)
{
}

auto Data::add(blockchain::Type chain) noexcept -> void
{
    chains_.emplace(chain);
}

auto Data::chains(allocator_type alloc) const noexcept -> Set<blockchain::Type>
{
    return {chains_, alloc};
}

auto Data::get_allocator() const noexcept -> allocator_type
{
    return memo_.get_allocator();
}

auto Data::height() const noexcept -> block::Height
{
    return mined_position_.height_;
}

auto Data::memo() const noexcept -> std::string_view { return memo_; }

auto Data::merge(
    const api::crypto::Blockchain& crypto,
    const internal::Transaction& rhs,
    const Log& log) noexcept -> void
{
    if (auto m = rhs.Memo(crypto); memo_.empty() || (false == m.empty())) {
        memo_ = m;
        log(OT_PRETTY_CLASS())("memo set to: \"")(memo_)("\"").Flush();
    }

    // TODO monotonic allocator
    for (auto chain : rhs.Chains(get_allocator())) { chains_.emplace(chain); }

    mined_position_ = rhs.MinedPosition();
}

auto Data::position() const noexcept -> const block::Position&
{
    return mined_position_;
}

auto Data::reset_size() noexcept -> void
{
    size_ = std::nullopt;
    normalized_size_ = std::nullopt;
}

auto Data::set_memo(const std::string_view memo) noexcept -> void
{
    set_memo(UnallocatedCString{memo});
}

auto Data::set_memo(UnallocatedCString&& memo) noexcept -> void
{
    memo_ = std::move(memo);
}

auto Data::set_position(const block::Position& pos) noexcept -> void
{
    mined_position_ = pos;
}

Data::~Data() = default;
}  // namespace opentxs::blockchain::bitcoin::block::transaction
