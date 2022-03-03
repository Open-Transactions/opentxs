// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "opentxs/network/zeromq/message/FrameSection.hpp"  // IWYU pragma: associated

#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "network/zeromq/message/FrameIterator.hpp"
#include "network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"

namespace opentxs::network::zeromq::blank
{
class FrameSection final : public zeromq::FrameSection::Imp
{
public:
    auto at(const std::size_t index) const noexcept(false)
        -> const zeromq::Frame& final
    {
        throw std::out_of_range{"no frames"};
    }
    auto begin() const noexcept -> const zeromq::FrameIterator final
    {
        return {};
    }
    auto end() const noexcept -> const zeromq::FrameIterator final
    {
        return {};
    }
    auto size() const noexcept -> std::size_t final { return {}; }

    auto at(const std::size_t index) noexcept(false) -> zeromq::Frame& final
    {
        throw std::out_of_range{"no frames"};
    }
    auto begin() noexcept -> zeromq::FrameIterator final { return {}; }
    auto end() noexcept -> zeromq::FrameIterator final { return {}; }

    FrameSection() = default;

    ~FrameSection() final = default;

private:
    FrameSection(const FrameSection&) = delete;
    FrameSection(FrameSection&&) = delete;
    auto operator=(const FrameSection&) -> FrameSection& = delete;
    auto operator=(FrameSection&) -> FrameSection& = delete;
};
}  // namespace opentxs::network::zeromq::blank

namespace opentxs::network::zeromq
{
auto swap(FrameSection& lhs, FrameSection& rhs) noexcept -> void
{
    lhs.swap(rhs);
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
FrameSection::FrameSection(
    const Message* parent,
    std::size_t position,
    std::size_t size) noexcept
    : parent_(parent)
    , position_(position)
    , size_(size)
{
    OT_ASSERT(nullptr != parent_);
}

auto FrameSection::at(const std::size_t index) const -> const Frame&
{
    return const_cast<FrameSection*>(this)->at(index);
}

auto FrameSection::at(const std::size_t index) -> Frame&
{
    if (size_ <= index) { throw std::out_of_range{"Invalid index"}; }

    return const_cast<Message*>(parent_)->at(position_ + index);
}

auto FrameSection::begin() const noexcept -> const FrameIterator
{
    return FrameIterator(std::make_unique<FrameIterator::Imp>(
                             const_cast<Message*>(parent_), position_)
                             .release());
}

auto FrameSection::begin() noexcept -> FrameIterator
{
    return FrameIterator(std::make_unique<FrameIterator::Imp>(
                             const_cast<Message*>(parent_), position_)
                             .release());
}

auto FrameSection::end() const noexcept -> const FrameIterator
{
    return FrameIterator(std::make_unique<FrameIterator::Imp>(
                             const_cast<Message*>(parent_), position_ + size_)
                             .release());
}

auto FrameSection::end() noexcept -> FrameIterator
{
    return FrameIterator(std::make_unique<FrameIterator::Imp>(
                             const_cast<Message*>(parent_), position_ + size_)
                             .release());
}

auto FrameSection::size() const noexcept -> std::size_t { return size_; }
}  // namespace opentxs::network::zeromq::implementation

namespace opentxs::network::zeromq
{
FrameSection::FrameSection(Imp* imp) noexcept
    : imp_(imp)
{
    OT_ASSERT(nullptr != imp_);
}

FrameSection::FrameSection() noexcept
    : FrameSection(std::make_unique<blank::FrameSection>().release())
{
}

FrameSection::FrameSection(FrameSection&& rhs) noexcept
    : FrameSection()
{
    swap(rhs);
}

auto FrameSection::operator=(FrameSection&& rhs) noexcept -> FrameSection&
{
    swap(rhs);

    return *this;
}

auto FrameSection::at(const std::size_t index) const -> const Frame&
{
    return imp_->at(index);
}

auto FrameSection::at(const std::size_t index) -> Frame&
{
    return imp_->at(index);
}

auto FrameSection::begin() const noexcept -> const FrameIterator
{
    return imp_->begin();
}

auto FrameSection::begin() noexcept -> FrameIterator { return imp_->begin(); }

auto FrameSection::end() const noexcept -> const FrameIterator
{
    return imp_->end();
}

auto FrameSection::end() noexcept -> FrameIterator { return imp_->end(); }

auto FrameSection::size() const noexcept -> std::size_t { return imp_->size(); }

auto FrameSection::swap(FrameSection& rhs) noexcept -> void
{
    std::swap(imp_, rhs.imp_);
}

FrameSection::~FrameSection()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::network::zeromq
