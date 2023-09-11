// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/util/alloc/Boost.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::pmr
{
class HasUpstreamAllocator
{
public:
    virtual ~HasUpstreamAllocator() = default;

protected:
    alloc::StandardToBoost upstream_resource_as_boost_;

    auto parent_resource() noexcept -> boost::container::pmr::memory_resource*
    {
        return std::addressof(upstream_resource_as_boost_);
    }

    HasUpstreamAllocator(alloc::Resource* mr) noexcept
        : upstream_resource_as_boost_(mr)
    {
    }
};

template <typename Resource>
class AllocatesChildren
{
public:
    AllocatesChildren(const AllocatesChildren&) = delete;
    AllocatesChildren(AllocatesChildren&&) = delete;

    virtual ~AllocatesChildren() = default;

private:
    Resource resource_;

protected:
    alloc::Default child_alloc_;

    template <typename... Args>
    AllocatesChildren(Args&&... args) noexcept
        : resource_(std::forward<Args>(args)...)
        , child_alloc_(std::addressof(resource_))
    {
    }
};
}  // namespace opentxs::pmr
