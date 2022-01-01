// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <vector>

#include "opentxs/blockchain/Blockchain.hpp"

namespace opentxs::blockchain::node::wallet
{
class BlockIndex
{
public:
    auto Query(const block::Hash& block) const noexcept -> bool;

    auto Add(const std::pmr::vector<block::Position>& blocks) noexcept -> void;
    auto Forget(const std::pmr::vector<block::pHash>& blocks) noexcept -> void;

    BlockIndex() noexcept;

    ~BlockIndex();

private:
    struct Imp;

    Imp* imp_;

    BlockIndex(const BlockIndex&) = delete;
    BlockIndex(BlockIndex&&) = delete;
    auto operator=(const BlockIndex&) -> BlockIndex& = delete;
    auto operator=(BlockIndex&&) -> BlockIndex& = delete;
};
}  // namespace opentxs::blockchain::node::wallet
