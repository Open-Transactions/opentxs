// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/statemachine/Matches.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <utility>

namespace opentxs::blockchain::node::wallet
{
Matches::Matches(allocator_type alloc) noexcept
    : match_20_(alloc)
    , match_32_(alloc)
    , match_33_(alloc)
    , match_64_(alloc)
    , match_65_(alloc)
    , match_txo_(alloc)
{
}

Matches::Matches(const Matches& rhs, allocator_type alloc) noexcept
    : match_20_(rhs.match_20_, alloc)
    , match_32_(rhs.match_32_, alloc)
    , match_33_(rhs.match_33_, alloc)
    , match_64_(rhs.match_64_, alloc)
    , match_65_(rhs.match_65_, alloc)
    , match_txo_(rhs.match_txo_, alloc)
{
}

Matches::Matches(Matches&& rhs, allocator_type alloc) noexcept
    : match_20_(std::move(rhs.match_20_), alloc)
    , match_32_(std::move(rhs.match_32_), alloc)
    , match_33_(std::move(rhs.match_33_), alloc)
    , match_64_(std::move(rhs.match_64_), alloc)
    , match_65_(std::move(rhs.match_65_), alloc)
    , match_txo_(std::move(rhs.match_txo_), alloc)
{
}

Matches::Matches(Matches&& rhs) noexcept
    : Matches(std::move(rhs), rhs.get_allocator())
{
}

auto Matches::get_allocator() const noexcept -> allocator_type
{
    return match_20_.get_allocator();
}

auto Matches::Merge(Matches&& rhs) noexcept -> void
{
    std::ranges::move(rhs.match_20_, std::inserter(match_20_, match_20_.end()));
    std::ranges::move(rhs.match_32_, std::inserter(match_32_, match_32_.end()));
    std::ranges::move(rhs.match_33_, std::inserter(match_33_, match_33_.end()));
    std::ranges::move(rhs.match_64_, std::inserter(match_64_, match_64_.end()));
    std::ranges::move(rhs.match_65_, std::inserter(match_65_, match_65_.end()));
    std::ranges::move(
        rhs.match_txo_, std::inserter(match_txo_, match_txo_.end()));
}

auto Matches::operator=(const Matches& rhs) noexcept -> Matches&
{
    match_20_ = rhs.match_20_;
    match_32_ = rhs.match_32_;
    match_33_ = rhs.match_33_;
    match_64_ = rhs.match_64_;
    match_65_ = rhs.match_65_;
    match_txo_ = rhs.match_txo_;

    return *this;
}

auto Matches::operator=(Matches&& rhs) noexcept -> Matches&
{
    match_20_ = std::move(rhs.match_20_);
    match_32_ = std::move(rhs.match_32_);
    match_33_ = std::move(rhs.match_33_);
    match_64_ = std::move(rhs.match_64_);
    match_65_ = std::move(rhs.match_65_);
    match_txo_ = std::move(rhs.match_txo_);

    return *this;
}
}  // namespace opentxs::blockchain::node::wallet
