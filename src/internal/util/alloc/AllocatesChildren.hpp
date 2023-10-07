// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::pmr
{
class HasUpstreamAllocator
{
public:
    HasUpstreamAllocator(const HasUpstreamAllocator&) = delete;
    HasUpstreamAllocator(HasUpstreamAllocator&&) = delete;
    auto operator=(const HasUpstreamAllocator&)
        -> HasUpstreamAllocator& = delete;
    auto operator=(HasUpstreamAllocator&&) -> HasUpstreamAllocator& = delete;

    virtual ~HasUpstreamAllocator() = default;

protected:
    alloc::Resource* upstream_resource_;

    auto parent_resource() noexcept -> alloc::Resource*
    {
        return upstream_resource_;
    }

    HasUpstreamAllocator(alloc::Resource* mr) noexcept
        : upstream_resource_(mr)
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
