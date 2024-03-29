// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/blockoracle/BlockBatch.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <utility>

#include "internal/blockchain/node/Job.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/block/Hash.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::internal
{
BlockBatch::Imp::Imp(
    download::JobID id,
    Vector<block::Hash>&& hashes,
    DownloadCallback download,
    SimpleCallback&& finish,
    allocator_type alloc) noexcept
    : id_(id)
    , hashes_(std::move(hashes), alloc)
    , start_(sClock::now())
    , log_(LogTrace())
    , callback_(std::move(download))
    , finish_(std::move(finish))
    , last_(start_)
    , submitted_(0)
{
    if (hashes_.empty()) {
        assert_true(-1 == id_);
        assert_false(finish_.operator bool());
        assert_false(callback_.operator bool());
    }

    if (-1 != id_) {
        assert_false(nullptr == finish_);
        assert_false(nullptr == callback_);
    }
}

BlockBatch::Imp::Imp(Imp& rhs, allocator_type alloc) noexcept
    : id_(rhs.id_)
    , hashes_(rhs.hashes_, alloc)
    , start_(rhs.start_)
    , log_(rhs.log_)
    , callback_(rhs.callback_)
    , finish_(rhs.finish_)
    , last_(rhs.last_)
    , submitted_(rhs.submitted_)
{
    rhs.callback_ = {};  // NOLINT(cert-oop58-cpp)
    rhs.finish_ = {};    // NOLINT(cert-oop58-cpp)
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

auto BlockBatch::Imp::Submit(const std::string_view block) noexcept -> bool
{
    if (callback_) { callback_(block); }

    ++submitted_;
    last_ = sClock::now();
    log_()(submitted_)(" of ")(hashes_.size())(" hashes submitted for job ")(
        id_)
        .Flush();

    return 0_uz == Remaining();
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
    assert_false(nullptr == imp_);
}

BlockBatch::BlockBatch(allocator_type alloc) noexcept
    : BlockBatch(pmr::default_construct<Imp>(alloc))
{
}

BlockBatch::BlockBatch(BlockBatch&& rhs) noexcept
    : BlockBatch(std::exchange(rhs.imp_, nullptr))
{
}

BlockBatch::BlockBatch(BlockBatch&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    pmr::move_construct(imp_, rhs.imp_, alloc);
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

auto BlockBatch::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto BlockBatch::get_deleter() noexcept -> delete_function
{
    return pmr::make_deleter(imp_);
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

auto BlockBatch::Submit(const std::string_view block) noexcept -> bool
{
    return imp_->Submit(block);
}

auto BlockBatch::swap(BlockBatch& rhs) noexcept -> void
{
    pmr::swap(imp_, rhs.imp_);
}

BlockBatch::~BlockBatch() { pmr::destroy(imp_); }
}  // namespace opentxs::blockchain::node::internal
