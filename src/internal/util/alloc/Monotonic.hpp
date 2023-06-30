// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <cstddef>

#include "internal/util/Thread.hpp"
#include "internal/util/alloc/Boost.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::alloc
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
class Monotonic final : public Resource
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

    Monotonic(Resource* upstream)
        : upstream_((nullptr == upstream) ? System() : upstream)
        , alloc_(buf_, sizeof(buf_), std::addressof(upstream_))
    {
    }
    Monotonic(const Monotonic&) = delete;
    Monotonic(Monotonic&&) = delete;
    auto operator=(const Monotonic&) -> Monotonic& = delete;
    auto operator=(Monotonic&&) -> Monotonic& = delete;

    ~Monotonic() final = default;

private:
    std::byte buf_[thread_pool_monotonic_];  // NOLINT(modernize-avoid-c-arrays)
    StandardToBoost upstream_;
    mutable libguarded::plain_guarded<BoostMonotonic> alloc_;
};
#pragma GCC diagnostic pop
}  // namespace opentxs::alloc
