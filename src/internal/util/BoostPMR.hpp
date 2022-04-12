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
/// This class adapts a boost::container::pmr::memory_resource* to a
/// std::pmr::memory_resource
///
/// Its purpose is to allow resources returned by functions such as
/// boost::container::pmr::new_delete_resource() to be used with std::pmr
/// containers.
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

/// This class adapts any boost::container::pmr::memory_resource to a
/// std::pmr::memory_resource
///
/// It allows you to construct the boost memory_resource of your choice and use
/// it for std::pmr containers
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

/// This class adapts a std::pmr::memory_resource* to a
/// boost::container::pmr::memory_resource
///
/// Its purpose is to allow std::pmr::memory_resource objects to act as upstream
/// allocators for boost memory_resource objects
class StandardToBoost final : public boost::container::pmr::memory_resource
{
public:
    auto do_allocate(std::size_t bytes, std::size_t alignment) -> void* final
    {
        return std_->allocate(bytes, alignment);
    }

    auto do_deallocate(void* p, std::size_t size, std::size_t alignment)
        -> void final
    {
        return std_->deallocate(p, size, alignment);
    }
    auto do_is_equal(const boost::container::pmr::memory_resource& other)
        const noexcept -> bool final
    {
        return &other == this;
    }

    StandardToBoost(Resource* std) noexcept
        : std_(std)
    {
    }
    StandardToBoost(const StandardToBoost&) = delete;
    StandardToBoost(StandardToBoost&&) = delete;
    auto operator=(const StandardToBoost&) -> StandardToBoost& = delete;
    auto operator=(StandardToBoost&&) -> StandardToBoost& = delete;

    ~StandardToBoost() final = default;

private:
    Resource* std_;
};

using BoostMonotonic = Boost<boost::container::pmr::monotonic_buffer_resource>;
using BoostPool = Boost<boost::container::pmr::unsynchronized_pool_resource>;
using BoostPoolSync = Boost<boost::container::pmr::synchronized_pool_resource>;
}  // namespace opentxs::alloc
