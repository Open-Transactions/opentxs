// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <initializer_list>
#include <memory>
#include <span>

#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
template <typename T>
struct Multiple {
    using allocator_type = alloc::Default;
    using initializer_list = std::initializer_list<T>;
    using span = std::span<const T>;

    operator span() const noexcept { return get(); }

    auto get() const noexcept -> std::span<const T> { return value_; }
    auto get_allocator() const noexcept -> allocator_type
    {
        return value_.get_allocator();
    }

    Multiple(initializer_list in, allocator_type alloc = {}) noexcept
        : value_(in.begin(), std::next(in.begin(), in.size()), alloc)
    {
    }
    Multiple(span in, allocator_type alloc = {}) noexcept
        : value_(in.begin(), in.end(), alloc)
    {
    }
    Multiple(const T& in, allocator_type alloc = {}) noexcept
        : value_(1u, in, alloc)
    {
    }
    Multiple(T& in, allocator_type alloc = {}) noexcept
        : value_(1u, std::move(in), alloc)
    {
    }
    Multiple(allocator_type alloc = {}) noexcept
        : value_(alloc)
    {
    }
    Multiple(const Multiple& rhs, allocator_type alloc = {}) noexcept
        : value_(rhs.value_, alloc)
    {
    }
    Multiple(Multiple&& rhs) noexcept
        : value_(rhs.value_, rhs.get_allocator())
    {
    }
    Multiple(Multiple&& rhs, allocator_type alloc) noexcept
        : value_(rhs.value_, alloc)
    {
    }
    auto operator=(const Multiple& rhs) noexcept -> Multiple&
    {
        value_ = rhs.value_;

        return *this;
    }
    auto operator=(Multiple&& rhs) noexcept -> Multiple&
    {
        value_ = std::move(rhs.value_);

        return *this;
    }

private:
    using vector = Vector<T>;

    vector value_;
};
}  // namespace opentxs
