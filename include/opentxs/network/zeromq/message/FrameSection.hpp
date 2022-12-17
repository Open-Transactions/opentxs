// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/message/Frame.hpp"
// IWYU pragma: no_include "opentxs/network/zeromq/message/FrameIterator.hpp"

#pragma once

#include <cstddef>
#include <iterator>

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Frame;
class FrameIterator;
class FrameSection;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
OPENTXS_EXPORT auto swap(FrameSection& lhs, FrameSection& rhs) noexcept -> void;

class OPENTXS_EXPORT FrameSection
{
public:
    class Imp;

    using difference_type = std::size_t;
    using value_type = Frame;
    using pointer = Frame*;
    using reference = Frame&;
    using iterator_category = std::forward_iterator_tag;

    auto at(const std::size_t index) const noexcept(false) -> const Frame&;
    auto begin() const noexcept -> const FrameIterator;
    auto end() const noexcept -> const FrameIterator;
    auto size() const noexcept -> std::size_t;

    auto at(const std::size_t index) noexcept(false) -> Frame&;
    auto begin() noexcept -> FrameIterator;
    auto end() noexcept -> FrameIterator;
    virtual auto swap(FrameSection& rhs) noexcept -> void;

    FrameSection(Imp* imp) noexcept;
    FrameSection() noexcept;
    FrameSection(const FrameSection&) = delete;
    FrameSection(FrameSection&& rhs) noexcept;
    auto operator=(const FrameSection&) -> FrameSection& = delete;
    auto operator=(FrameSection&&) noexcept -> FrameSection&;

    virtual ~FrameSection();

private:
    Imp* imp_;
};
}  // namespace opentxs::network::zeromq
