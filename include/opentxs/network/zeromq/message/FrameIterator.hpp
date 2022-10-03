// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <string_view>

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace internal
{
class FrameIterator;
}  // namespace internal

class Frame;
class FrameIterator;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct hash<opentxs::network::zeromq::FrameIterator> {
    auto operator()(const opentxs::network::zeromq::FrameIterator& rhs)
        const noexcept -> std::size_t;
};
}  // namespace std

namespace opentxs::network::zeromq
{
OPENTXS_EXPORT auto operator<(
    const FrameIterator& lhs,
    const FrameIterator& rhs) noexcept(false) -> bool;
OPENTXS_EXPORT auto operator==(
    const FrameIterator& lhs,
    const FrameIterator& rhs) noexcept -> bool;
OPENTXS_EXPORT auto swap(FrameIterator& lhs, FrameIterator& rhs) noexcept
    -> void;

class OPENTXS_EXPORT FrameIterator
{
public:
    class Imp;

    using difference_type = std::size_t;
    using value_type = Frame;
    using pointer = Frame*;
    using reference = Frame&;
    using iterator_category = std::forward_iterator_tag;

    auto operator*() const noexcept(false) -> const Frame&;
    auto operator->() const noexcept(false) -> const Frame*;
    auto operator!=(const FrameIterator&) const noexcept -> bool;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::FrameIterator&;

    auto operator*() noexcept(false) -> Frame&;
    auto operator->() noexcept(false) -> Frame*;
    auto operator++() noexcept -> FrameIterator&;
    auto operator++(int) noexcept -> FrameIterator;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::FrameIterator&;
    virtual auto swap(FrameIterator& rhs) noexcept -> void;

    FrameIterator() noexcept;
    OPENTXS_NO_EXPORT FrameIterator(Imp* imp) noexcept;
    FrameIterator(const FrameIterator&) noexcept;
    FrameIterator(FrameIterator&&) noexcept;
    auto operator=(const FrameIterator&) noexcept -> FrameIterator&;
    auto operator=(FrameIterator&&) noexcept -> FrameIterator&;

    virtual ~FrameIterator();

private:
    friend auto zeromq::operator<(const FrameIterator&, const FrameIterator&)
        -> bool;
    friend auto zeromq::operator==(
        const FrameIterator&,
        const FrameIterator&) noexcept -> bool;

    Imp* imp_;
};
}  // namespace opentxs::network::zeromq
