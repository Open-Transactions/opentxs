// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/cfilter/GCS.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <utility>

#include "blockchain/cfilter/GCSPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/cfilter/Hash.hpp"
#include "opentxs/blockchain/cfilter/Header.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::cfilter
{
GCS::GCS(GCSPrivate* imp) noexcept
    : imp_(imp)
{
    assert_false(nullptr == imp_);
}

GCS::GCS(allocator_type alloc) noexcept
    : GCS(pmr::default_construct<GCSPrivate>(alloc))
{
}

GCS::GCS(const GCS& rhs, allocator_type alloc) noexcept
    : GCS(rhs.imp_->clone(alloc))
{
}

GCS::GCS(GCS&& rhs) noexcept
    : GCS(std::exchange(rhs.imp_, nullptr))
{
}

GCS::GCS(GCS&& rhs, allocator_type alloc) noexcept
    : imp_(nullptr)
{
    pmr::move_construct(imp_, rhs.imp_, alloc);
}

auto GCS::operator=(const GCS& rhs) noexcept -> GCS&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_base(this, imp_, rhs.imp_);
}

auto GCS::operator=(GCS&& rhs) noexcept -> GCS&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_base(*this, rhs, imp_, rhs.imp_);
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
    return pmr::make_deleter(this);
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

auto GCS::swap(GCS& rhs) noexcept -> void { pmr::swap(imp_, rhs.imp_); }

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

GCS::~GCS() { pmr::destroy(imp_); }
}  // namespace opentxs::blockchain::cfilter
