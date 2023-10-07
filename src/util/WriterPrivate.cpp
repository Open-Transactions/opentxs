// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/WriterPrivate.hpp"  // IWYU pragma: associated

#include <span>
#include <utility>

#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"

namespace opentxs
{
WriterPrivate::WriterPrivate(
    std::function<WriteBuffer(std::size_t)> reserve,
    std::function<bool(std::size_t)> truncate,
    allocator_type alloc) noexcept
    : Allocated(alloc)
    , reserve_(std::move(reserve))
    , truncate_(std::move(truncate))
    , size_(std::nullopt)
{
}

WriterPrivate::WriterPrivate(
    const WriterPrivate& rhs,
    allocator_type alloc) noexcept
    : Allocated(alloc)
    , reserve_(rhs.reserve_)
    , truncate_(rhs.truncate_)
    , size_(rhs.size_)
{
}

auto WriterPrivate::CanTruncate() const noexcept -> bool
{
    return truncate_.operator bool();
}

auto WriterPrivate::Reserve(std::size_t val) noexcept -> WriteBuffer
{
    if (size_.has_value()) {
        LogError()()("already reserved ") (*size_)(" bytes").Flush();

        return std::span<std::byte>{};
    } else {
        size_.emplace(val);
    }

    if (reserve_.operator bool()) {
        LogInsane()()("reserving ")(val)(" bytes").Flush();

        return std::invoke(reserve_, val);
    } else {
        LogError()()("empty writer").Flush();

        return std::span<std::byte>{};
    }
}

auto WriterPrivate::Truncate(std::size_t val) noexcept -> bool
{
    if (false == size_.has_value() || (val > *size_)) {
        LogError()()("requesting truncation to ")(val)(" bytes but only ")(
            size_.value_or(0))(" bytes are reserved")
            .Flush();

        return false;
    }

    if (truncate_.operator bool()) {

        return std::invoke(truncate_, val);
    } else {
        LogError()()("no truncation method available").Flush();

        return false;
    }
}

WriterPrivate::~WriterPrivate() = default;
}  // namespace opentxs
