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

namespace opentxs
{
inline namespace pmr
{
template <typename Out, typename In>
auto clone_as(const In* me, alloc::PMR<In> alloc) noexcept -> Out*
{
    auto* out = alloc.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    alloc.construct(out, *me);

    return out;
}

template <typename Out, typename In>
auto clone_as_mutable(In* me, alloc::PMR<In> alloc) noexcept -> Out*
{
    auto* out = alloc.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    alloc.construct(out, *me);

    return out;
}

template <typename In>
auto clone(const In* me, alloc::PMR<In> alloc) noexcept -> In*
{
    return clone_as<In, In>(me, alloc);
}

template <typename In>
auto clone_mutable(In* me, alloc::PMR<In> alloc) noexcept -> In*
{
    return clone_as_mutable<In, In>(me, alloc);
}

template <typename T>
auto pmr_delete(T*& imp) noexcept -> void
{
    if (nullptr != imp) {
        // TODO switch to destroying delete after resolution of
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=107352
        auto deleter = imp->get_deleter();
        std::invoke(deleter);
        imp = nullptr;
    }
}

template <typename T, typename Imp>
auto copy_assign_base(T& lhs, const T& rhs, Imp*& limp, Imp* rimp) noexcept
    -> T&
{
    if (limp != rimp) {
        OT_ASSERT(nullptr != rimp);

        if (nullptr == limp) {
            // NOTE moved-from state
            limp = rimp->clone(rhs.get_allocator());
        } else {
            auto* old{limp};
            limp = rimp->clone(lhs.get_allocator());
            pmr_delete(old);
        }
    }

    return lhs;
}

template <typename Parent, typename T>
auto copy_assign_child(T& lhs, const T& rhs) noexcept -> T&
{
    lhs.Parent::operator=(rhs);

    return lhs;
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

template <typename T>
auto default_construct(alloc::PMR<T> alloc) noexcept -> T*
{
    return construct<T>(alloc);
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

template <typename T, typename Imp>
auto move_assign_base(T& lhs, T&& rhs, Imp*& limp, Imp* rimp) noexcept -> T&
{
    if (nullptr == limp) {
        // NOTE moved-from state
        lhs.swap(rhs);

        return lhs;
    } else if (lhs.get_allocator() == rhs.get_allocator()) {
        lhs.swap(rhs);

        return lhs;
    } else {

        return copy_assign_base<T, Imp>(lhs, rhs, limp, rimp);
    }
}

template <typename Parent, typename T>
auto move_assign_child(T& lhs, T&& rhs) noexcept -> T&
{
    lhs.Parent::operator=(std::move(rhs));

    return lhs;
}

template <typename T>
auto pmr_swap(T*& lhs, T*& rhs) noexcept -> void
{
    OT_ASSERT(nullptr != rhs);

    if (nullptr != lhs) {
        OT_ASSERT(lhs->get_allocator() == rhs->get_allocator());
    }

    using std::swap;
    swap(lhs, rhs);
}

template <typename T, typename Imp>
auto pmr_swap(T& lhs, T& rhs, Imp*& limp, Imp*& rimp) noexcept -> void
{
    OT_ASSERT(nullptr != rimp);

    if (nullptr != limp) {
        OT_ASSERT(lhs.get_allocator() == rhs.get_allocator());
    }

    using std::swap;
    swap(limp, rimp);
}

template <typename T, typename AllocatorType>
auto move_construct(T*& lhs, T*& rhs, AllocatorType alloc) noexcept -> void
{
    OT_ASSERT(nullptr != rhs);

    if (rhs->get_allocator() == alloc) {
        using std::swap;
        swap(lhs, rhs);
    } else {
        lhs = rhs->clone(alloc);
    }
}
}  // namespace pmr
}  // namespace opentxs
