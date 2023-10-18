// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>

#include "blockchain/node/wallet/subchain/statemachine/MatchIndex.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node::wallet
{
class MatchCache final : public Allocated
{
public:
    using Results = opentxs::Map<block::Position, MatchIndex>;

    auto GetMatches(const block::Position& block) const noexcept
        -> std::optional<MatchIndex>;
    auto get_allocator() const noexcept -> allocator_type final;

    auto Add(Results&& results) noexcept -> void;
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    auto Forget(const block::Position& last) noexcept -> void;
    auto Reset() noexcept -> void;

    MatchCache(allocator_type alloc) noexcept;

    ~MatchCache() final = default;

private:
    Results results_;
};
}  // namespace opentxs::blockchain::node::wallet
