// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::pmr
{
template <typename In, typename... Args>
auto clone(const In* me, alloc::PMR<In> alloc, Args&&... args) noexcept -> In*;

template <typename Out, typename In, typename... Args>
auto clone_as(const In* me, alloc::PMR<In> alloc, Args&&... args) noexcept
    -> Out*;

template <typename Out, typename In, typename... Args>
auto clone_as_mutable(In* me, alloc::PMR<In> alloc, Args&&... args) noexcept
    -> Out*;

template <typename In, typename... Args>
auto clone_mutable(In* me, alloc::PMR<In> alloc, Args&&... args) noexcept
    -> In*;

template <typename T, typename... Args>
auto construct(alloc::PMR<T> alloc, Args&&... args) noexcept(false) -> T*;

template <typename T, typename Imp, typename... Args>
auto copy_assign_base(
    T* parent,
    Imp*& lhs,
    const Imp* rhs,
    Args&&... args) noexcept -> T&;

template <typename Parent, typename T>
auto copy_assign_child(T& lhs, const T& rhs) noexcept -> T&;

template <typename T>
auto default_construct(alloc::PMR<T> alloc) noexcept -> T*;

template <typename T>
auto destroy(T*& imp) noexcept -> void;

template <typename T>
auto make_deleter(T* me) noexcept -> std::function<void()>;

template <typename T, typename Imp, typename... Args>
auto move_assign_base(
    T& lhs,
    T& rhs,
    Imp*& limp,
    Imp* rimp,
    Args&&... args) noexcept -> T&;

template <typename Parent, typename T>
auto move_assign_child(T& lhs, T&& rhs) noexcept -> T&;

template <typename T, typename AllocatorType, typename... Args>
auto move_construct(
    T*& lhs,
    T*& rhs,
    AllocatorType alloc,
    Args&&... args) noexcept -> void;

template <typename T>
auto swap(T*& lhs, T*& rhs) noexcept -> void;

template <typename T, typename Swapper>
auto swap(T*& lhs, T*& rhs, Swapper swapper) noexcept -> void;
}  // namespace opentxs::pmr

namespace opentxs::pmr
{
template <typename In, typename... Args>
auto clone(const In* me, alloc::PMR<In> alloc, Args&&... args) noexcept -> In*
{
    return clone_as<In, In>(me, alloc, std::forward<Args&&>(args)...);
}

template <typename Out, typename In, typename... Args>
auto clone_as(const In* me, alloc::PMR<In> alloc, Args&&... args) noexcept
    -> Out*
{
    auto* out = alloc.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    alloc.construct(out, *me, std::forward<Args&&>(args)...);

    return out;
}

template <typename Out, typename In, typename... Args>
auto clone_as_mutable(In* me, alloc::PMR<In> alloc, Args&&... args) noexcept
    -> Out*
{
    auto* out = alloc.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    alloc.construct(out, *me, std::forward<Args&&>(args)...);

    return out;
}

template <typename In, typename... Args>
auto clone_mutable(In* me, alloc::PMR<In> alloc, Args&&... args) noexcept -> In*
{
    return clone_as_mutable<In, In>(me, alloc, std::forward<Args&&>(args)...);
}

template <typename T, typename... Args>
auto construct(alloc::PMR<T> alloc, Args&&... args) noexcept(false) -> T*
{
    auto* out = alloc.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    try {
        alloc.construct(out, std::forward<Args&&>(args)...);
    } catch (...) {
        alloc.deallocate(out, 1_uz);
        std::rethrow_exception(std::current_exception());
    }

    return out;
}

template <typename T, typename Imp, typename... Args>
auto copy_assign_base(
    T* parent,
    Imp*& lhs,
    const Imp* rhs,
    Args&&... args) noexcept -> T&
{
    if (lhs != rhs) {
        OT_ASSERT(nullptr != rhs);

        if (nullptr == lhs) {
            // NOTE moved-from state
            lhs =
                rhs->clone(rhs->get_allocator(), std::forward<Args&&>(args)...);
        } else {
            auto* old{lhs};
            lhs =
                rhs->clone(lhs->get_allocator(), std::forward<Args&&>(args)...);
            destroy(old);
        }
    }

    return *parent;
}

template <typename Parent, typename T>
auto copy_assign_child(T& lhs, const T& rhs) noexcept -> T&
{
    lhs.Parent::operator=(rhs);

    return lhs;
}

template <typename T>
auto default_construct(alloc::PMR<T> alloc) noexcept -> T*
{
    return construct<T>(alloc);
}

template <typename T>
auto destroy(T*& imp) noexcept -> void
{
    if (nullptr != imp) {
        // TODO switch to destroying delete after resolution of
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107352
        auto deleter = imp->get_deleter();
        std::invoke(deleter);
        imp = nullptr;
    }
}

template <typename T>
auto make_deleter(T* me) noexcept -> std::function<void()>
{
    return [me] {
        auto pmr = alloc::PMR<T>{me->get_allocator()};
        pmr.destroy(me);
        pmr.deallocate(me, 1_uz);
    };
}

template <typename T, typename Imp, typename... Args>
auto move_assign_base(
    T& lhs,
    T& rhs,
    Imp*& limp,
    Imp* rimp,
    Args&&... args) noexcept -> T&
{
    if (nullptr == limp) {
        // NOTE moved-from state
        lhs.swap(rhs);

        return lhs;
    } else if (lhs.get_allocator() == rhs.get_allocator()) {
        lhs.swap(rhs);

        return lhs;
    } else {

        return copy_assign_base<T, Imp, Args...>(
            &lhs, limp, rimp, std::forward<Args&&>(args)...);
    }
}

template <typename Parent, typename T>
auto move_assign_child(T& lhs, T&& rhs) noexcept -> T&
{
    lhs.Parent::operator=(std::move(rhs));

    return lhs;
}

template <typename T, typename AllocatorType, typename... Args>
auto move_construct(
    T*& lhs,
    T*& rhs,
    AllocatorType alloc,
    Args&&... args) noexcept -> void
{
    OT_ASSERT(nullptr != rhs);

    if (rhs->get_allocator() == alloc) {
        using std::swap;
        swap(lhs, rhs);
    } else {
        lhs = rhs->clone(alloc, std::forward<Args&&>(args)...);
    }
}

template <typename T>
auto swap(T*& lhs, T*& rhs) noexcept -> void
{
    OT_ASSERT(nullptr != rhs);

    if (nullptr != lhs) {
        OT_ASSERT(lhs->get_allocator() == rhs->get_allocator());
    }

    using std::swap;
    swap(lhs, rhs);
}

template <typename T, typename Swapper>
auto swap(T*& lhs, T*& rhs, Swapper swapper) noexcept -> void
{
    OT_ASSERT(nullptr != rhs);

    if (nullptr != lhs) {
        OT_ASSERT(lhs->get_allocator() == rhs->get_allocator());
    }

    std::invoke(swapper, lhs, rhs);
}
}  // namespace opentxs::pmr
