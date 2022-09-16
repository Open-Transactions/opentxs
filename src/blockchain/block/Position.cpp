// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "opentxs/blockchain/block/Position.hpp"  // IWYU pragma: associated

#include <compare>
#include <sstream>

#include "opentxs/core/Data.hpp"

namespace opentxs::blockchain::block
{
auto swap(Position& lhs, Position& rhs) noexcept -> void { lhs.swap(rhs); }
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block
{
Position::Position() noexcept
    : Position(Height{-1}, Hash{})
{
}

Position::Position(const Height& height, const Hash& hash) noexcept
    : Position(Height{height}, Hash{hash})
{
}

Position::Position(const Height& height, Hash&& hash) noexcept
    : Position(std::move(Height{height}), std::move(hash))
{
}

Position::Position(const Height& height, ReadView hash) noexcept
    : Position(std::move(height), Hash{hash})
{
}

Position::Position(Height&& height, const Hash& hash) noexcept
    : Position(std::move(height), Hash{hash})
{
}

Position::Position(Height&& height, Hash&& hash) noexcept
    : height_(std::move(height))
    , hash_(std::move(hash))
{
}

Position::Position(Height&& height, ReadView hash) noexcept
    : Position(std::move(height), Hash{hash})
{
}

Position::Position(const std::pair<Height, Hash>& data) noexcept
    : Position(Height{data.first}, Hash{data.second})
{
}

Position::Position(std::pair<Height, Hash>&& data) noexcept
    : Position(std::move(data.first), std::move(data.second))
{
}

Position::Position(const Position& rhs) noexcept
    : Position(rhs.height_, rhs.hash_)
{
}

Position::Position(Position&& rhs) noexcept
    : Position(std::move(rhs.height_), std::move(rhs.hash_))
{
}

auto Position::operator=(const Position& rhs) noexcept -> Position&
{
    if (this != std::addressof(rhs)) {
        height_ = rhs.height_;
        hash_ = rhs.hash_;
    }

    return *this;
}

auto Position::operator=(Position&& rhs) noexcept -> Position&
{
    swap(rhs);

    return *this;
}

auto Position::operator==(const Position& rhs) const noexcept -> bool
{
    return (height_ == rhs.height_) && (hash_ == rhs.hash_);
}

auto Position::operator!=(const Position& rhs) const noexcept -> bool
{
    return (height_ != rhs.height_) || (hash_ != rhs.hash_);
}

auto Position::operator<(const Position& rhs) const noexcept -> bool
{
    if (height_ < rhs.height_) { return true; }

    if (height_ > rhs.height_) { return false; }

    return hash_ < rhs.hash_;
}

auto Position::operator<=(const Position& rhs) const noexcept -> bool
{
    if (height_ < rhs.height_) { return true; }

    if (height_ > rhs.height_) { return false; }

    return hash_ <= rhs.hash_;
}

auto Position::operator>(const Position& rhs) const noexcept -> bool
{
    if (height_ > rhs.height_) { return true; }

    if (height_ < rhs.height_) { return false; }

    return hash_ != rhs.hash_;
}

auto Position::operator>=(const Position& rhs) const noexcept -> bool
{
    if (height_ > rhs.height_) { return true; }

    if (height_ < rhs.height_) { return false; }

    return true;
}

auto Position::print() const noexcept -> UnallocatedCString
{
    return print({}).c_str();
}

auto Position::print(alloc::Default alloc) const noexcept -> CString
{
    // TODO c++20 use allocator
    auto out = std::stringstream{};
    out << hash_.asHex();
    out << " at height ";
    out << std::to_string(height_);

    return CString{alloc}.append(out.str());
}

auto Position::swap(Position& rhs) noexcept -> void
{
    using std::swap;
    swap(height_, rhs.height_);
    swap(hash_, rhs.hash_);
}
}  // namespace opentxs::blockchain::block
