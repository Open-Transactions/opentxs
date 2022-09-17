// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/blockoracle/BlockBatch.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <utility>

#include "internal/blockchain/node/Job.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/block/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/util/Allocator.hpp"

namespace opentxs::blockchain::node::internal
{
BlockBatch::Imp::Imp(
    download::JobID id,
    Vector<block::Hash>&& hashes,
    DownloadCallback download,
    SimpleCallback&& finish,
    allocator_type alloc) noexcept
    : id_(id)
    , hashes_(std::move(hashes))
    , start_(Clock::now())
    , callback_(std::move(download))
    , finish_(std::move(finish))
    , last_(start_)
    , submitted_(0)
{
    if (hashes_.empty()) {
        OT_ASSERT(-1 == id_);
        OT_ASSERT(false == finish_.operator bool());
        OT_ASSERT(false == callback_.operator bool());
    }

    if (-1 != id_) {
        OT_ASSERT(finish_);
        OT_ASSERT(callback_);
    }
}

BlockBatch::Imp::Imp(allocator_type alloc) noexcept
    : Imp(-1, Vector<block::Hash>{alloc}, {}, nullptr, alloc)
{
}

auto BlockBatch::Imp::LastActivity() const noexcept -> std::chrono::seconds
{
    return std::chrono::duration_cast<std::chrono::seconds>(last_ - start_);
}

auto BlockBatch::Imp::Remaining() const noexcept -> std::size_t
{
    const auto target = hashes_.size();

    return target - std::min(submitted_, target);
}

auto BlockBatch::Imp::Submit(const std::string_view block) noexcept -> void
{
    if (callback_) { callback_(block); }

    ++submitted_;
    last_ = Clock::now();
}

BlockBatch::Imp::~Imp()
{
    if (finish_) { std::invoke(finish_); }
}
}  // namespace opentxs::blockchain::node::internal

namespace opentxs::blockchain::node::internal
{
BlockBatch::BlockBatch(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

BlockBatch::BlockBatch() noexcept
    : BlockBatch([] {
        auto alloc = alloc::PMR<Imp>{};
        auto* imp = alloc.allocate(1);
        alloc.construct(imp);

        return imp;
    }())
{
}

BlockBatch::BlockBatch(BlockBatch&& rhs) noexcept
    : imp_(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

BlockBatch::operator bool() const noexcept
{
    if (nullptr == imp_) {

        return false;
    } else {

        return -1 != imp_->id_;
    }
}

auto BlockBatch::operator=(BlockBatch&& rhs) noexcept -> BlockBatch&
{
    swap(rhs);

    return *this;
}

auto BlockBatch::Get() const noexcept -> const Vector<block::Hash>&
{
    return imp_->hashes_;
}

auto BlockBatch::ID() const noexcept -> std::size_t
{
    return static_cast<std::size_t>(imp_->id_);
}

auto BlockBatch::LastActivity() const noexcept -> std::chrono::seconds
{
    return imp_->LastActivity();
}

auto BlockBatch::Remaining() const noexcept -> std::size_t
{
    return imp_->Remaining();
}

auto BlockBatch::Submit(const std::string_view block) noexcept -> void
{
    imp_->Submit(block);
}

auto BlockBatch::swap(BlockBatch& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
}

BlockBatch::~BlockBatch()
{
    if (nullptr != imp_) {
        auto alloc = alloc::PMR<Imp>{imp_->get_allocator()};
        alloc.destroy(imp_);
        alloc.deallocate(imp_, 1);
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::node::internal
