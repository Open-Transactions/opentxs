// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "blockchain/block/bitcoin/Transaction.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <memory_resource>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/block/Block.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "util/Container.hpp"

namespace opentxs::blockchain::block::bitcoin::implementation
{
Transaction::Cache::Cache(
    const std::string& memo,
    std::pmr::vector<blockchain::Type>&& chains,
    block::Position&& minedPosition) noexcept(false)
    : lock_()
    , normalized_id_()
    , size_()
    , normalized_size_()
    , memo_(memo)
    , chains_(std::move(chains))
    , mined_position_(std::move(minedPosition))
{
    if (0 == chains_.size()) { throw std::runtime_error("missing chains"); }

    dedup(chains_);
}

Transaction::Cache::Cache(const Cache& rhs) noexcept
    : lock_()
    , normalized_id_()
    , size_()
    , normalized_size_()
    , memo_()
    , chains_()
    , mined_position_(rhs.mined_position_)
{
    auto lock = rLock{rhs.lock_};
    normalized_id_ = rhs.normalized_id_;
    size_ = rhs.size_;
    normalized_size_ = rhs.normalized_size_;
    memo_ = rhs.memo_;
    chains_ = rhs.chains_;
}

auto Transaction::Cache::add(blockchain::Type chain) noexcept -> void
{
    auto lock = rLock{lock_};
    chains_.emplace_back(chain);
    dedup(chains_);
}

auto Transaction::Cache::chains() const noexcept
    -> std::pmr::vector<blockchain::Type>
{
    auto lock = rLock{lock_};

    return chains_;
}

auto Transaction::Cache::height() const noexcept -> block::Height
{
    auto lock = rLock{lock_};

    return mined_position_.first;
}

auto Transaction::Cache::memo() const noexcept -> std::string
{
    auto lock = rLock{lock_};

    return memo_;
}

auto Transaction::Cache::merge(const internal::Transaction& rhs) noexcept
    -> void
{
    auto lock = rLock{lock_};

    if (auto memo = rhs.Memo(); memo_.empty() || (false == memo.empty())) {
        memo_.swap(memo);
    }

    for (auto chain : rhs.Chains()) {
        chains_.emplace_back(chain);
        dedup(chains_);
    }

    mined_position_ = rhs.MinedPosition();
}

auto Transaction::Cache::position() const noexcept -> const block::Position&
{
    auto lock = rLock{lock_};

    return mined_position_;
}

auto Transaction::Cache::reset_size() noexcept -> void
{
    auto lock = rLock{lock_};
    size_ = std::nullopt;
    normalized_size_ = std::nullopt;
}

auto Transaction::Cache::set_memo(const std::string& memo) noexcept -> void
{
    set_memo(std::string{memo});
}

auto Transaction::Cache::set_memo(std::string&& memo) noexcept -> void
{
    auto lock = rLock{lock_};
    memo_ = std::move(memo);
}

auto Transaction::Cache::set_position(const block::Position& pos) noexcept
    -> void
{
    auto lock = rLock{lock_};
    mined_position_ = pos;
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
