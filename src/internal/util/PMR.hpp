// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

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

template <typename In>
auto clone(const In* me, alloc::PMR<In> alloc) noexcept -> In*
{
    return clone_as<In, In>(me, alloc);
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
auto construct(alloc::PMR<T> alloc, Args... args) noexcept -> T*
{
    auto* out = alloc.allocate(1_uz);

    OT_ASSERT(nullptr != out);

    alloc.construct(out, args...);

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
}  // namespace pmr
}  // namespace opentxs
