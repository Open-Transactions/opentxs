// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>

namespace opentxs
{
template <typename T>
class AsyncConst
{
public:
    using value_type = T;

    operator const T&() const noexcept { return get(); }

    auto get() const -> const T& { return future_.get(); }

    template <typename... Args>
    auto set_value(Args&&... args) -> const T&
    {
        promise_.set_value(std::forward<Args>(args)...);

        return get();
    }

    AsyncConst() noexcept
        : promise_()
        , future_(promise_.get_future())
    {
    }
    AsyncConst(const AsyncConst&) = delete;
    AsyncConst(AsyncConst&&) = delete;
    auto operator=(const AsyncConst&) -> AsyncConst& = delete;
    auto operator=(AsyncConst&&) -> AsyncConst& = delete;

    ~AsyncConst() = default;

private:
    std::promise<T> promise_;
    std::shared_future<T> future_;
};
}  // namespace opentxs
