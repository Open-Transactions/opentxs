// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node::wallet
{
struct Matches final : public Allocated {
    Set<crypto::Bip32Index> match_20_;
    Set<crypto::Bip32Index> match_32_;
    Set<crypto::Bip32Index> match_33_;
    Set<crypto::Bip32Index> match_64_;
    Set<crypto::Bip32Index> match_65_;
    Set<block::Outpoint> match_txo_;

    auto get_allocator() const noexcept -> allocator_type final;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    auto Merge(Matches&& rhs) noexcept -> void;

    Matches(allocator_type alloc = {}) noexcept;
    Matches(const Matches& rhs, allocator_type alloc = {}) noexcept;
    Matches(Matches&& rhs) noexcept;
    Matches(Matches&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Matches& rhs) noexcept -> Matches&;
    auto operator=(Matches&& rhs) noexcept -> Matches&;

    ~Matches() final = default;
};
}  // namespace opentxs::blockchain::node::wallet
