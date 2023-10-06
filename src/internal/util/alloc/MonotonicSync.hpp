// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <cstddef>

#include "internal/util/Thread.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::alloc
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
class MonotonicSync final : public Resource
{
public:
    auto do_allocate(std::size_t bytes, std::size_t alignment) -> void* final
    {
        return alloc_.lock()->allocate(bytes, alignment);
    }

    auto do_deallocate(void* p, std::size_t size, std::size_t alignment)
        -> void final
    {
        return alloc_.lock()->deallocate(p, size, alignment);
    }
    auto do_is_equal(const Resource& other) const noexcept -> bool final
    {
        return other == *alloc_.lock();
    }

    MonotonicSync(Resource* upstream)
        : alloc_(
              buf_,
              sizeof(buf_),
              (nullptr == upstream) ? System() : upstream)
    {
    }
    MonotonicSync(const MonotonicSync&) = delete;
    MonotonicSync(MonotonicSync&&) = delete;
    auto operator=(const MonotonicSync&) -> MonotonicSync& = delete;
    auto operator=(MonotonicSync&&) -> MonotonicSync& = delete;

    ~MonotonicSync() final = default;

private:
    std::byte buf_[thread_pool_monotonic_];  // NOLINT(modernize-avoid-c-arrays)
    mutable libguarded::plain_guarded<MonotonicUnsync> alloc_;
};
#pragma GCC diagnostic pop
}  // namespace opentxs::alloc
