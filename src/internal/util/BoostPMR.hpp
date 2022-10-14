// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/pmr/memory_resource.hpp>  // IWYU pragma: export
#include <boost/container/pmr/monotonic_buffer_resource.hpp>  // IWYU pragma: export
#include <boost/container/pmr/synchronized_pool_resource.hpp>  // IWYU pragma: export
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>  // IWYU pragma: export
#include <cs_plain_guarded.h>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"

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
        return std::addressof(other) == this;
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
        return std::addressof(other) == this;
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
        return std::addressof(other) == this;
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

/// This class adapts a non-thread safe resource to a thread safe resource by
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

class Logging final : public Resource
{
public:
    auto do_allocate(std::size_t bytes, std::size_t alignment) -> void* final
    {
        auto* out = upstream_->allocate(bytes, alignment);
        LogConsole()(OT_PRETTY_CLASS())("allocated ")(bytes)(" bytes at ")(
            reinterpret_cast<std::uintptr_t>(out))
            .Flush();

        return out;
    }

    auto do_deallocate(void* p, std::size_t size, std::size_t alignment)
        -> void final
    {
        LogConsole()(OT_PRETTY_CLASS())("deallocating ")(size)(" bytes at ")(
            reinterpret_cast<std::uintptr_t>(p))
            .Flush();

        return upstream_->deallocate(p, size, alignment);
    }
    auto do_is_equal(const Resource& other) const noexcept -> bool final
    {

        return std::addressof(other) == upstream_;
    }

    Logging(Resource* upstream = nullptr)
        : upstream_((nullptr == upstream) ? System() : upstream)
    {
    }
    Logging(const Logging&) = delete;
    Logging(Logging&&) = delete;
    auto operator=(const Logging&) -> Logging& = delete;
    auto operator=(Logging&&) -> Logging& = delete;

    ~Logging() final = default;

private:
    Resource* upstream_;
};

using BoostMonotonic = Boost<boost::container::pmr::monotonic_buffer_resource>;
using BoostPoolSync = Boost<boost::container::pmr::synchronized_pool_resource>;
}  // namespace opentxs::alloc
