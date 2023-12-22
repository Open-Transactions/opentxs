// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cassert>
#include <mutex>

#include "internal/util/Mutex.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
template <class T>
class UniqueQueue
{
public:
    using Key = int;

    void CancelByValue(const T& in) const
    {
        auto lock = Lock{lock_};

        if (0 == set_.count(in)) { return; }

        for (auto i = queue_.cbegin(); i < queue_.cend(); ++i) {
            const auto& [key, value] = *i;

            if (value == in) {
                set_.erase(value);
                queue_.erase(i);
                break;
            }
        }

        assert(set_.size() == queue_.size());
    }

    void CancelByKey(const Key& in) const
    {
        auto lock = Lock{lock_};

        for (auto i = queue_.cbegin(); i < queue_.cend(); ++i) {
            const auto& [key, value] = *i;

            if (key == in) {
                set_.erase(value);
                queue_.erase(i);
                break;
            }
        }

        assert(set_.size() == queue_.size());
    }

    auto Copy() const -> UnallocatedMap<T, Key>
    {
        UnallocatedMap<T, Key> output{};
        auto lock = Lock{lock_};

        for (const auto& [key, value] : queue_) { output.emplace(value, key); }

        return output;
    }

    auto empty() const -> bool
    {
        auto lock = Lock{lock_};

        return queue_.empty();
    }

    auto Push(const Key key, const T& in) const -> bool
    {
        assert(0 < key);

        auto lock = Lock{lock_};

        if (0 == set_.count(in)) {
            queue_.push_front({key, in});
            set_.emplace(in);

            assert(set_.size() == queue_.size());

            return true;
        }

        return false;
    }

    auto Pop(Key& key, T& out) const -> bool
    {
        auto lock = Lock{lock_};

        if (0 == queue_.size()) { return false; }

        const auto& [outKey, outValue] = queue_.back();
        set_.erase(outValue);
        out = outValue;
        key = outKey;
        queue_.pop_back();

        assert(set_.size() == queue_.size());

        return true;
    }

    auto size() const -> std::size_t
    {
        auto lock = Lock{lock_};

        return queue_.size();
    }

    UniqueQueue() noexcept
        : lock_()
        , queue_()
        , set_()
    {
    }
    UniqueQueue(const UniqueQueue&) = delete;
    UniqueQueue(UniqueQueue&&) = delete;
    auto operator=(const UniqueQueue&) -> UniqueQueue& = delete;
    auto operator=(UniqueQueue&&) -> UniqueQueue& = delete;

private:
    mutable std::mutex lock_;
    mutable UnallocatedDeque<std::pair<Key, T>> queue_;
    mutable UnallocatedSet<T> set_;
};
}  // namespace opentxs
