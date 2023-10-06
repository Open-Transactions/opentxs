// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/util/Exclusive.hpp"  // IWYU pragma: associated

#include "opentxs/util/Log.hpp"

namespace opentxs
{
template <class C>
Exclusive<C>::Exclusive() noexcept
    : p_{nullptr}
    , lock_{nullptr}
    , save_{[](Container&, eLock&, bool) -> void {}}
    , success_{true}
    , callback_{nullptr}
{
}

template <class C>
Exclusive<C>::Exclusive(
    Container* in,
    std::shared_mutex& lock,
    Save save,
    const Callback callback) noexcept
    : p_{in}
    , lock_{new eLock(lock)}
    , save_{save}
    , success_{true}
    , callback_{callback}
{
    assert_false(nullptr == lock_);
}

template <class C>
Exclusive<C>::Exclusive(Exclusive&& rhs) noexcept
    // NOLINTBEGIN(cert-oop11-cpp)
    : p_{rhs.p_}
    , lock_{rhs.lock_.release()}
    , save_{rhs.save_}
    , success_{rhs.success_.load()}
    , callback_{rhs.callback_}
// NOLINTEND(cert-oop11-cpp)
{
    rhs.p_ = nullptr;
    rhs.save_ = Save{nullptr};
    rhs.success_.store(false);
    rhs.callback_ = Callback{nullptr};
}

template <class C>
auto Exclusive<C>::operator=(Exclusive&& rhs) noexcept -> Exclusive<C>&
{
    p_ = rhs.p_;
    rhs.p_ = nullptr;
    lock_.reset(rhs.lock_.release());
    save_ = rhs.save_;
    rhs.save_ = Save{nullptr};
    success_.store(rhs.success_.load());
    rhs.success_.store(false);
    callback_ = std::move(rhs.callback_);
    rhs.callback_ = Callback{nullptr};

    return *this;
}

template <class C>
Exclusive<C>::operator bool() const
{
    return (nullptr != p_) && (p_);
}

template <class C>
Exclusive<C>::operator const C&() const
{
    return get();
}

template <class C>
Exclusive<C>::operator C&()
{
    return get();
}

template <class C>
auto Exclusive<C>::Abort() -> bool
{
    if (false == bool(*this)) { return false; }

    success_.store(false);

    return Release();
}

template <class C>
auto Exclusive<C>::get() const -> const C&
{
    assert_true(*this);

    return **p_;
}

template <class C>
auto Exclusive<C>::get() -> C&
{
    assert_true(*this);

    return **p_;
}

template <class C>
auto Exclusive<C>::Release() -> bool
{
    if (false == bool(*this)) { return false; }

    assert_false(nullptr == lock_);
    assert_false(nullptr == p_);
    assert_false(nullptr == *p_);

    save_(*p_, *lock_, success_);

    if (callback_) { callback_(**p_); }

    // NOTE: p_ points to an object owned by another class.
    p_ = nullptr;
    save_ = Save{nullptr};
    success_.store(false);

    return true;
}

template <class C>
Exclusive<C>::~Exclusive()
{
    Release();
}
}  // namespace opentxs
