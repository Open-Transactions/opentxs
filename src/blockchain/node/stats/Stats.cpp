// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/node/Stats.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "blockchain/node/stats/Imp.hpp"
#include "blockchain/node/stats/Shared.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node
{
Stats::Stats(Imp* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp_);
}

Stats::Stats() noexcept
    : Stats(std::make_unique<Imp>().release())
{
}

Stats::Stats(const Stats& rhs) noexcept
    : Stats(rhs.imp_->clone().release())
{
}

Stats::Stats(Stats&& rhs) noexcept
    : imp_(nullptr)
{
    operator=(std::move(rhs));
}

auto Stats::operator=(const Stats& rhs) noexcept -> Stats&
{
    auto old = std::unique_ptr<Imp>{imp_};
    imp_ = rhs.imp_->clone().release();
    old.reset();

    assert_false(nullptr == imp_);

    return *this;
}

auto Stats::operator=(Stats&& rhs) noexcept -> Stats&
{
    using std::swap;
    swap(imp_, rhs.imp_);

    assert_false(nullptr == imp_);

    return *this;
}

auto Stats::BlockHeaderTip(Type chain) const noexcept -> block::Position
{
    return imp_->shared_->BlockHeaderTip(chain);
}

auto Stats::BlockTip(Type chain) const noexcept -> block::Position
{
    return imp_->shared_->BlockTip(chain);
}

auto Stats::CfilterTip(Type chain) const noexcept -> block::Position
{
    return imp_->shared_->CfilterTip(chain);
}

auto Stats::PeerCount(Type chain) const noexcept -> std::size_t
{
    return imp_->shared_->PeerCount(chain);
}

auto Stats::SyncTip(Type chain) const noexcept -> block::Position
{
    return imp_->shared_->SyncTip(chain);
}

Stats::~Stats()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::node
