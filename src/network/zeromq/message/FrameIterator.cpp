// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/zeromq/message/FrameIterator.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"

namespace opentxs::network::zeromq
{
auto swap(FrameIterator& lhs, FrameIterator& rhs) noexcept -> void
{
    lhs.swap(rhs);
}

auto operator<(const FrameIterator& lhs, const FrameIterator& rhs) noexcept(
    false) -> bool
{
    return lhs.imp_->operator<(rhs);
}

auto operator==(const FrameIterator& lhs, const FrameIterator& rhs) noexcept
    -> bool
{
    return lhs.imp_->operator==(rhs);
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq
{
FrameIterator::Imp::Imp(Message* parent, std::size_t position) noexcept
    : parent_(parent)
    , position_(position)
{
}

FrameIterator::Imp::Imp() noexcept
    : Imp(nullptr)
{
}

FrameIterator::Imp::Imp(const Imp& rhs) noexcept
    : Imp(rhs.parent_, rhs.position_)
{
}

auto FrameIterator::Imp::operator<(const zeromq::FrameIterator& rhs) const
    noexcept(false) -> bool
{
    if (parent_ != rhs.imp_->parent_) {
        throw std::runtime_error{
            "unable to compare frame iterators from different messages"};
    }

    return position_ < rhs.imp_->position_;
}

auto FrameIterator::Imp::operator==(
    const zeromq::FrameIterator& rhs) const noexcept -> bool
{
    return (parent_ == rhs.imp_->parent_) && (position_ == rhs.imp_->position_);
}

auto FrameIterator::Imp::hash() const noexcept -> std::size_t
{
    auto out = std::size_t{reinterpret_cast<std::uintptr_t>(parent_)};
    out ^= position_;

    return out;
}

FrameIterator::Imp::~Imp() = default;
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq
{
FrameIterator::FrameIterator(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

FrameIterator::FrameIterator() noexcept
    : FrameIterator(std::make_unique<Imp>().release())
{
}

FrameIterator::FrameIterator(const FrameIterator& rhs) noexcept
    : FrameIterator(new Imp(*rhs.imp_))
{
}

FrameIterator::FrameIterator(FrameIterator&& rhs) noexcept
    : FrameIterator()
{
    swap(rhs);
}

auto FrameIterator::operator=(const FrameIterator& rhs) noexcept
    -> FrameIterator&
{
    auto old = std::unique_ptr<Imp>{imp_};
    imp_ = new Imp(*rhs.imp_);

    return *this;
}

auto FrameIterator::operator=(FrameIterator&& rhs) noexcept -> FrameIterator&
{
    swap(rhs);

    return *this;
}

auto FrameIterator::operator*() const noexcept(false) -> const Frame&
{
    return const_cast<FrameIterator*>(this)->operator*();
}

auto FrameIterator::operator*() noexcept(false) -> Frame&
{
    return *(operator->());
}

auto FrameIterator::operator->() const noexcept(false) -> const Frame*
{
    return const_cast<FrameIterator*>(this)->operator->();
}

auto FrameIterator::operator->() noexcept(false) -> Frame*
{
    auto* parent = imp_->parent_;

    if (nullptr == parent) {
        throw std::out_of_range{"invalid frame iterator"};
    }

    return &(parent->at(imp_->position_));
}

auto FrameIterator::operator!=(const FrameIterator& rhs) const noexcept -> bool
{
    return !(*this == rhs);
}

auto FrameIterator::operator++() noexcept -> FrameIterator&
{
    imp_->position_++;

    return *this;
}

auto FrameIterator::operator++(int) noexcept -> FrameIterator
{
    auto output{*this};
    imp_->position_++;

    return output;
}

auto FrameIterator::Internal() const noexcept -> const internal::FrameIterator&
{
    return *imp_;
}

auto FrameIterator::Internal() noexcept -> internal::FrameIterator&
{
    return *imp_;
}

auto FrameIterator::swap(FrameIterator& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
}

FrameIterator::~FrameIterator()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::network::zeromq
