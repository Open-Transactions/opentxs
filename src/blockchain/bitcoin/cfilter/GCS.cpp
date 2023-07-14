// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <utility>

#include "blockchain/bitcoin/cfilter/GCSPrivate.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain
{
GCS::GCS(GCSPrivate* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

GCS::GCS(allocator_type alloc) noexcept
    : GCS(std::make_unique<GCSPrivate>(alloc).release())
{
}

GCS::GCS(const GCS& rhs, allocator_type alloc) noexcept
    : GCS(rhs.imp_->clone(alloc).release())
{
}

GCS::GCS(GCS&& rhs) noexcept
    : GCS(std::move(rhs), rhs.get_allocator())
{
}

GCS::GCS(GCS&& rhs, allocator_type alloc) noexcept
    : GCS(alloc)
{
    swap(rhs);
}

auto GCS::operator=(const GCS& rhs) noexcept -> GCS&
{
    auto old = std::unique_ptr<GCSPrivate>{imp_};
    imp_ = rhs.imp_->clone(get_allocator()).release();

    return *this;
}

auto GCS::operator=(GCS&& rhs) noexcept -> GCS&
{
    swap(rhs);

    return *this;
}

auto GCS::Compressed(Writer&& out) const noexcept -> bool
{
    return imp_->Compressed(std::move(out));
}

auto GCS::ElementCount() const noexcept -> std::uint32_t
{
    return imp_->ElementCount();
}

auto GCS::Encode(Writer&& out) const noexcept -> bool
{
    return imp_->Encode(std::move(out));
}

auto GCS::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto GCS::get_deleter() noexcept -> delete_function
{
    return make_deleter(this);
}

auto GCS::Hash() const noexcept -> cfilter::Hash { return imp_->Hash(); }

auto GCS::Header(const cfilter::Header& previous) const noexcept
    -> cfilter::Header
{
    return imp_->Header(previous);
}

auto GCS::Internal() const noexcept -> const internal::GCS& { return *imp_; }

auto GCS::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto GCS::Match(
    const Targets& in,
    allocator_type alloc,
    allocator_type monotonic) const noexcept -> Matches
{
    return imp_->Match(in, alloc, monotonic);
}

auto GCS::size() const noexcept -> std::size_t { return imp_->size(); }

auto GCS::Serialize(Writer&& out) const noexcept -> bool
{
    return imp_->Serialize(std::move(out));
}

auto GCS::swap(GCS& rhs) noexcept -> void
{
    if (imp_->get_allocator() == rhs.imp_->get_allocator()) {
        std::swap(imp_, rhs.imp_);
    } else {
        auto oldLhs = std::unique_ptr<GCSPrivate>{imp_};
        auto oldRhs = std::unique_ptr<GCSPrivate>{rhs.imp_};
        imp_ = oldRhs->clone(get_allocator()).release();
        rhs.imp_ = oldLhs->clone(rhs.get_allocator()).release();
    }
}

auto GCS::Test(const Data& target, allocator_type monotonic) const noexcept
    -> bool
{
    return imp_->Test(target, monotonic);
}

auto GCS::Test(const ReadView target, allocator_type monotonic) const noexcept
    -> bool
{
    return imp_->Test(target, monotonic);
}

auto GCS::Test(const Vector<ByteArray>& targets, allocator_type monotonic)
    const noexcept -> bool
{
    return imp_->Test(targets, monotonic);
}

auto GCS::Test(const Vector<Space>& targets, allocator_type monotonic)
    const noexcept -> bool
{
    return imp_->Test(targets, monotonic);
}

GCS::~GCS()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain
