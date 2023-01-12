// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>

#include "opentxs/util/Allocator.hpp"

namespace opentxs::alloc
{
// This class adapts a non-thread safe resource to a thread safe resource by
/// protecting it with a mutex
class ThreadSafe final : public Resource
{
public:
    auto do_allocate(std::size_t bytes, std::size_t alignment) -> void* final
    {
        auto handle = upstream_.lock();
        auto* resource = *handle;

        return resource->allocate(bytes, alignment);
    }

    auto do_deallocate(void* p, std::size_t size, std::size_t alignment)
        -> void final
    {
        auto handle = upstream_.lock();
        auto* resource = *handle;

        return resource->deallocate(p, size, alignment);
    }
    auto do_is_equal(const Resource& other) const noexcept -> bool final
    {
        auto handle = upstream_.lock();
        auto* resource = *handle;

        return std::addressof(other) == resource;
    }

    ThreadSafe(Resource* upstream)
        : upstream_((nullptr == upstream) ? System() : upstream)
    {
    }
    ThreadSafe(const ThreadSafe&) = delete;
    ThreadSafe(ThreadSafe&&) = delete;
    auto operator=(const ThreadSafe&) -> ThreadSafe& = delete;
    auto operator=(ThreadSafe&&) -> ThreadSafe& = delete;

    ~ThreadSafe() final = default;

private:
    mutable libguarded::plain_guarded<Resource*> upstream_;
};
}  // namespace opentxs::alloc
