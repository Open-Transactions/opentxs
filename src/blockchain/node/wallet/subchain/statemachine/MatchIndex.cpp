// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/statemachine/MatchIndex.hpp"  // IWYU pragma: associated

#include <utility>

namespace opentxs::blockchain::node::wallet
{
MatchIndex::MatchIndex(allocator_type alloc) noexcept
    : confirmed_no_match_(alloc)
    , confirmed_match_(alloc)
{
}

MatchIndex::MatchIndex(const MatchIndex& rhs, allocator_type alloc) noexcept
    : confirmed_no_match_(rhs.confirmed_no_match_, alloc)
    , confirmed_match_(rhs.confirmed_match_, alloc)
{
}

MatchIndex::MatchIndex(MatchIndex&& rhs, allocator_type alloc) noexcept
    : confirmed_no_match_(std::move(rhs.confirmed_no_match_), alloc)
    , confirmed_match_(std::move(rhs.confirmed_match_), alloc)
{
}

MatchIndex::MatchIndex(MatchIndex&& rhs) noexcept
    : MatchIndex(std::move(rhs), rhs.get_allocator())
{
}

auto MatchIndex::get_allocator() const noexcept -> allocator_type
{
    return confirmed_no_match_.get_allocator();
}

auto MatchIndex::Merge(MatchIndex&& rhs) noexcept -> void
{
    confirmed_no_match_.Merge(std::move(rhs.confirmed_no_match_));
    confirmed_match_.Merge(std::move(rhs.confirmed_match_));
}

auto MatchIndex::operator=(const MatchIndex& rhs) noexcept -> MatchIndex&
{
    confirmed_no_match_ = rhs.confirmed_no_match_;
    confirmed_match_ = rhs.confirmed_match_;

    return *this;
}

auto MatchIndex::operator=(MatchIndex&& rhs) noexcept -> MatchIndex&
{
    confirmed_no_match_ = std::move(rhs.confirmed_no_match_);
    confirmed_match_ = std::move(rhs.confirmed_match_);

    return *this;
}
}  // namespace opentxs::blockchain::node::wallet
