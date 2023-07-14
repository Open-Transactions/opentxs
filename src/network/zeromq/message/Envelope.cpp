// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/zeromq/message/Envelope.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/network/zeromq/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
#include "network/zeromq/message/EnvelopePrivate.hpp"

namespace opentxs::network::zeromq
{
auto swap(Envelope& lhs, Envelope& rhs) noexcept -> void { lhs.swap(rhs); }

auto operator==(const Envelope& lhs, const Envelope& rhs) noexcept -> bool
{
    return lhs.get() == rhs.get();
}

auto operator<=>(const Envelope& lhs, const Envelope& rhs) noexcept
    -> std::strong_ordering
{
    return lhs.get() <=> rhs.get();
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq
{
Envelope::Envelope(EnvelopePrivate* imp) noexcept
    : imp_(std::move(imp))
{
    OT_ASSERT(nullptr != imp_);
}

Envelope::Envelope(std::span<Frame> frames, allocator_type alloc) noexcept
    : Envelope(construct<EnvelopePrivate>(alloc, frames))
{
}

Envelope::Envelope(allocator_type alloc) noexcept
    : Envelope(default_construct<EnvelopePrivate>(alloc))
{
}

Envelope::Envelope(const Envelope& rhs, allocator_type alloc) noexcept
    : Envelope(pmr::clone(rhs.imp_, {alloc}))
{
}

Envelope::Envelope(Envelope&& rhs) noexcept
    : Envelope(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

Envelope::Envelope(Envelope&& rhs, allocator_type alloc) noexcept
    : Envelope(alloc)
{
    operator=(std::move(rhs));
}

auto Envelope::get() const noexcept -> std::span<const Frame>
{
    return imp_->get();
}

auto Envelope::get() noexcept -> std::span<Frame> { return imp_->get(); }

auto Envelope::get_allocator() const noexcept -> allocator_type
{
    return imp_->get_allocator();
}

auto Envelope::get_deleter() noexcept -> delete_function
{
    return make_deleter(this);
}

auto Envelope::IsValid() const noexcept -> bool { return imp_->IsValid(); }

auto Envelope::operator=(const Envelope& rhs) noexcept -> Envelope&
{
    return copy_assign_base(*this, rhs, imp_, rhs.imp_);
}

auto Envelope::operator=(Envelope&& rhs) noexcept -> Envelope&
{
    return move_assign_base(*this, std::move(rhs), imp_, rhs.imp_);
}

auto Envelope::swap(Envelope& rhs) noexcept -> void
{
    using std::swap;
    swap(imp_, rhs.imp_);
}

Envelope::~Envelope() { pmr_delete(imp_); }
}  // namespace opentxs::network::zeromq
