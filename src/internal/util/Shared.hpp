// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cassert>  // IWYU pragma: keep
#include <memory>
#include <shared_mutex>
#include <stdexcept>
#include <utility>

#include "internal/util/Mutex.hpp"

namespace opentxs
{
template <class C>
class Shared
{
public:
    operator bool() const { return nullptr != p_; }
    operator const C&() const { return get(); }

    auto get() const -> const C&
    {
        if (nullptr == p_) { throw std::runtime_error("Invalid pointer"); }

        return *p_;
    }

    auto Release() noexcept -> bool
    {
        if (nullptr == p_) { return false; }

        p_ = nullptr;
        lock_.reset(nullptr);

        return true;
    }

    Shared(const C* in, std::shared_mutex& lock) noexcept
        : p_(in)
        , lock_(new sLock(lock))
    {
        assert(lock_);
    }
    Shared() noexcept
        : p_(nullptr)
        , lock_(nullptr)
    {
    }

    Shared(const Shared& rhs) noexcept
        : p_(rhs.p_)
        , lock_(
              (nullptr != rhs.lock_->mutex()) ? new sLock(*rhs.lock_->mutex())
                                              : nullptr)
    {
    }
    Shared(Shared&& rhs) noexcept
        : p_(rhs.p_)
        , lock_(rhs.lock_.release())
    {
        rhs.p_ = nullptr;
    }
    auto operator=(const Shared& rhs) noexcept -> Shared&
    {
        p_ = rhs.p_;

        if (nullptr != rhs.lock_->mutex()) {
            lock_ = std::make_unique<sLock>(*rhs.lock_->mutex());
        } else {
            lock_.reset(nullptr);
        }

        return *this;
    }
    auto operator=(Shared&& rhs) noexcept -> Shared&
    {
        p_ = rhs.p_;
        rhs.p_ = nullptr;
        lock_ = std::move(rhs.lock_);

        return *this;
    }

    ~Shared() { Release(); }

private:
    const C* p_{nullptr};
    std::unique_ptr<sLock> lock_{nullptr};

};  // class Shared
}  // namespace opentxs
