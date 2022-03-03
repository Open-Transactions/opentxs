// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node::wallet
{
class BlockIndex
{
public:
    auto Query(const block::Hash& block) const noexcept -> bool;

    auto Add(const UnallocatedVector<block::Position>& blocks) noexcept -> void;
    auto Forget(const UnallocatedVector<block::pHash>& blocks) noexcept -> void;

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
