// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>

namespace opentxs
{
template <typename ValueType>
class AsyncValue
{
    bool set_;
    std::promise<ValueType> promise_;

public:
    std::shared_future<ValueType> value_;

    void set(const ValueType& value)
    {
        if (false == set_) {
            promise_.set_value(value);
            set_ = true;
        }
    }
    void set(ValueType&& value)
    {
        if (false == set_) {
            promise_.set_value(std::move(value));
            set_ = true;
        }
    }

    AsyncValue() noexcept
        : set_(false)
        , promise_()
        , value_(promise_.get_future())
    {
    }
};
}  // namespace opentxs
