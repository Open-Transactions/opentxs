// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "blockchain/node/wallet/subchain/statemachine/Matches.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/util/Allocated.hpp"

namespace opentxs::blockchain::node::wallet
{
struct MatchIndex final : public Allocated {
    Matches confirmed_no_match_;
    Matches confirmed_match_;

    auto get_allocator() const noexcept -> allocator_type final;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    auto Merge(MatchIndex&& rhs) noexcept -> void;

    MatchIndex(allocator_type alloc = {}) noexcept;
    MatchIndex(const MatchIndex& rhs, allocator_type alloc = {}) noexcept;
    MatchIndex(MatchIndex&& rhs) noexcept;
    MatchIndex(MatchIndex&& rhs, allocator_type alloc) noexcept;
    auto operator=(const MatchIndex& rhs) noexcept -> MatchIndex&;
    auto operator=(MatchIndex&& rhs) noexcept -> MatchIndex&;

    ~MatchIndex() final = default;
};
}  // namespace opentxs::blockchain::node::wallet
