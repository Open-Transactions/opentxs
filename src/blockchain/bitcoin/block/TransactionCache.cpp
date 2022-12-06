// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "blockchain/bitcoin/block/Transaction.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/block/Block.hpp"  // IWYU pragma: keep
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Container.hpp"

namespace opentxs::blockchain::bitcoin::block::implementation
{
Transaction::Cache::Cache(
    const UnallocatedCString& memo,
    UnallocatedVector<blockchain::Type>&& chains,
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
    -> UnallocatedVector<blockchain::Type>
{
    auto lock = rLock{lock_};

    return chains_;
}

auto Transaction::Cache::height() const noexcept -> block::Height
{
    auto lock = rLock{lock_};

    return mined_position_.height_;
}

auto Transaction::Cache::memo() const noexcept -> UnallocatedCString
{
    auto lock = rLock{lock_};

    return memo_;
}

auto Transaction::Cache::merge(
    const api::crypto::Blockchain& crypto,
    const internal::Transaction& rhs,
    const Log& log) noexcept -> void
{
    auto lock = rLock{lock_};

    if (auto memo = rhs.Memo(crypto);
        memo_.empty() || (false == memo.empty())) {
        memo_.swap(memo);
        log(OT_PRETTY_CLASS())("memo set to: \"")(memo_)("\"").Flush();
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

auto Transaction::Cache::set_memo(const UnallocatedCString& memo) noexcept
    -> void
{
    set_memo(UnallocatedCString{memo});
}

auto Transaction::Cache::set_memo(UnallocatedCString&& memo) noexcept -> void
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
}  // namespace opentxs::blockchain::bitcoin::block::implementation
