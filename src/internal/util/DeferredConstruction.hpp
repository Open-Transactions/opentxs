// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>

#include "internal/util/Future.hpp"

namespace opentxs
{
template <typename T>
class DeferredConstruction
{
public:
    using value_type = T;

    operator const T&() const noexcept { return get(); }

    auto get() const -> const T& { return future_.get(); }
    auto ready() const noexcept { return IsReady(future_); }

    template <typename... Args>
    auto set_value(Args&&... args) -> const T&
    {
        promise_.set_value(std::forward<Args>(args)...);

        return get();
    }

    DeferredConstruction() noexcept
        : promise_()
        , future_(promise_.get_future())
    {
    }
    DeferredConstruction(const DeferredConstruction& rhs) noexcept
        : DeferredConstruction()
    {
        if (rhs.ready()) { set_value(rhs.get()); }
    }
    DeferredConstruction(DeferredConstruction&&) = delete;
    auto operator=(const DeferredConstruction&)
        -> DeferredConstruction& = delete;
    auto operator=(DeferredConstruction&&) -> DeferredConstruction& = delete;

    ~DeferredConstruction() = default;

private:
    std::promise<T> promise_;
    std::shared_future<T> future_;
};
}  // namespace opentxs
