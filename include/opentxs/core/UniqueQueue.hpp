// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cassert>
#include <deque>
#include <mutex>
#include <set>

#include "opentxs/Types.hpp"
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
        Lock lock(lock_);

        if (0 == set_.count(in)) { return; }

        for (auto i = queue_.cbegin(); i < queue_.cend(); ++i) {
            /* TODO: these lines will cause a segfault in the clang-5 ast
             *  parser.
            const auto & [ key, value ] = *i;
            [[maybe_unused]] const auto& notUsed = key;
            */
            const auto& value = std::get<1>(*i);

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
        Lock lock(lock_);

        for (auto i = queue_.cbegin(); i < queue_.cend(); ++i) {
            /* TODO: this line will cause a segfault in the clang-5 ast parser.
            const auto & [ key, value ] = *i;
            */
            const auto& key = std::get<0>(*i);
            const auto& value = std::get<1>(*i);

            if (key == in) {
                set_.erase(value);
                queue_.erase(i);
                break;
            }
        }

        assert(set_.size() == queue_.size());
    }

    auto Copy() const -> std::pmr::map<T, Key>
    {
        std::pmr::map<T, Key> output{};
        Lock lock(lock_);

        /* TODO: this line will cause a segfault in the clang-5 ast parser.
        for (const auto & [ key, value ] : queue_) {
        */
        for (const auto& it : queue_) {
            const auto& key = it.first;
            const auto& value = it.second;
            output.emplace(value, key);
        }

        return output;
    }

    auto empty() const -> bool
    {
        Lock lock(lock_);

        return queue_.empty();
    }

    auto Push(const Key key, const T& in) const -> bool
    {
        assert(0 < key);

        Lock lock(lock_);

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
        Lock lock(lock_);

        if (0 == queue_.size()) { return false; }

        /* TODO: this line will cause a segfault in the clang-5 ast parser.
        const auto & [ outKey, outValue ] = queue_.back();
        */
        const auto& outKey = queue_.back().first;
        const auto& outValue = queue_.back().second;
        set_.erase(outValue);
        out = outValue;
        key = outKey;
        queue_.pop_back();

        assert(set_.size() == queue_.size());

        return true;
    }

    auto size() const -> std::size_t
    {
        Lock lock(lock_);

        return queue_.size();
    }

    UniqueQueue() noexcept
        : lock_()
        , queue_()
        , set_()
    {
    }

private:
    mutable std::mutex lock_;
    mutable std::pmr::deque<std::pair<Key, T>> queue_;
    mutable std::pmr::set<T> set_;

    UniqueQueue(const UniqueQueue&) = delete;
    UniqueQueue(UniqueQueue&&) = delete;
    auto operator=(const UniqueQueue&) -> UniqueQueue& = delete;
    auto operator=(UniqueQueue&&) -> UniqueQueue& = delete;
};
}  // namespace opentxs
