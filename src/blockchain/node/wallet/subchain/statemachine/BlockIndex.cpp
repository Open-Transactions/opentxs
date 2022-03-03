// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/BlockIndex.hpp"  // IWYU pragma: associated

#include <memory>
#include <mutex>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::blockchain::node::wallet
{
struct BlockIndex::Imp {
    auto Query(const block::Hash& block) const noexcept -> bool
    {
        auto lock = Lock{lock_};

        return 0 < set_.count(block.str());
    }

    auto Add(const UnallocatedVector<block::Position>& blocks) noexcept -> void
    {
        auto lock = Lock{lock_};

        for (const auto& [height, hash] : blocks) { set_.emplace(hash->str()); }
    }
    auto Forget(const UnallocatedVector<block::pHash>& blocks) noexcept -> void
    {
        auto lock = Lock{lock_};

        for (const auto& block : blocks) { set_.erase(block->str()); }
    }

    Imp() noexcept
        : lock_()
        , set_()
    {
    }

    ~Imp() = default;

private:
    mutable std::mutex lock_;
    UnallocatedUnorderedSet<UnallocatedCString> set_;  // TODO benchmark robin
                                                       // hood set

    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

BlockIndex::BlockIndex() noexcept
    : imp_(std::make_unique<Imp>().release())
{
}

auto BlockIndex::Add(const UnallocatedVector<block::Position>& blocks) noexcept
    -> void
{
    imp_->Add(blocks);
}

auto BlockIndex::Forget(const UnallocatedVector<block::pHash>& blocks) noexcept
    -> void
{
    imp_->Forget(blocks);
}

auto BlockIndex::Query(const block::Hash& block) const noexcept -> bool
{
    return imp_->Query(block);
}

BlockIndex::~BlockIndex()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::node::wallet
