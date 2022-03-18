// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/pmr/memory_resource.hpp>  // IWYU pragma: export
#include <boost/container/pmr/monotonic_buffer_resource.hpp>  // IWYU pragma: export
#include <boost/container/pmr/synchronized_pool_resource.hpp>  // IWYU pragma: export
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>  // IWYU pragma: export
#include <utility>

#include "opentxs/util/Allocator.hpp"

namespace opentxs::alloc
{
class BoostWrap final : public Resource
{
public:
    auto do_allocate(std::size_t bytes, std::size_t alignment) -> void* final
    {
        return boost_->allocate(bytes, alignment);
    }

    auto do_deallocate(void* p, std::size_t size, std::size_t alignment)
        -> void final
    {
        return boost_->deallocate(p, size, alignment);
    }
    auto do_is_equal(const Resource& other) const noexcept -> bool final
    {
        return &other == this;
    }

    BoostWrap(boost::container::pmr::memory_resource* boost) noexcept
        : boost_(boost)
    {
    }
    BoostWrap(const BoostWrap&) = delete;
    BoostWrap(BoostWrap&&) = delete;
    auto operator=(const BoostWrap&) -> BoostWrap& = delete;
    auto operator=(BoostWrap&&) -> BoostWrap& = delete;

    ~BoostWrap() final = default;

private:
    boost::container::pmr::memory_resource* boost_;
};

template <typename T>
class Boost final : public Resource
{
public:
    T boost_;

    auto do_allocate(std::size_t bytes, std::size_t alignment) -> void* final
    {
        return boost_.allocate(bytes, alignment);
    }

    auto do_deallocate(void* p, std::size_t size, std::size_t alignment)
        -> void final
    {
        return boost_.deallocate(p, size, alignment);
    }
    auto do_is_equal(const Resource& other) const noexcept -> bool final
    {
        return &other == this;
    }

    template <typename... Args>
    Boost(Args&&... args)
        : boost_(std::forward<Args>(args)...)
    {
    }
    Boost(const Boost&) = delete;
    Boost(Boost&&) = delete;
    auto operator=(const Boost&) -> Boost& = delete;
    auto operator=(Boost&&) -> Boost& = delete;

    ~Boost() final = default;
};

using BoostMonotonic = Boost<boost::container::pmr::monotonic_buffer_resource>;
using BoostPool = Boost<boost::container::pmr::unsynchronized_pool_resource>;
using BoostPoolSync = Boost<boost::container::pmr::synchronized_pool_resource>;

auto standard_to_boost(Resource* standard) noexcept
    -> boost::container::pmr::memory_resource*;
}  // namespace opentxs::alloc
