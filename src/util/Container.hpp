// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <ranges>
#include <span>

#include "internal/util/P0330.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
template <typename T>
auto contains(const UnallocatedVector<T>& vector, const T& value) noexcept
    -> bool
{
    for (const auto& item : vector) {
        if (item == value) { return true; }
    }

    return false;
}

template <typename Value, typename Container = Vector<Value>, typename... Args>
auto copy_construct(std::span<const Value> in, Args... args) noexcept
    -> Container
{
    auto out = Container{args...};
    out.reserve(in.size());
    out.clear();
    std::ranges::copy(in, std::back_inserter(out));

    return out;
}

template <typename Container>
auto dedup(Container& vector) noexcept -> void
{
    std::ranges::sort(vector);
    vector.erase(std::unique(vector.begin(), vector.end()), vector.end());
}

template <typename Store, typename Interface>
auto insert_sorted(
    UnallocatedVector<Store>& vector,
    const Interface& key) noexcept -> void
{
    static const auto less = std::less<Interface>{};

    for (auto i{vector.begin()}; i != vector.end(); std::advance(i, 1)) {
        const auto& val = *i;

        if (less(val, key)) {

            continue;
        } else if (less(key, val)) {
            vector.insert(i, key);

            return;
        } else {

            return;
        }
    }

    vector.emplace_back(key);
}

template <typename Value, typename Container = Vector<Value>, typename... Args>
auto move_construct(std::span<Value> in, Args... args) noexcept -> Container
{
    auto out = Container{args...};
    out.reserve(in.size());
    out.clear();
    std::ranges::move(in, std::back_inserter(out));

    return out;
}

template <typename Store, typename Interface>
auto remove(UnallocatedVector<Store>& vector, const Interface& key) noexcept
    -> std::size_t
{
    const auto before{vector.size()};
    vector.erase(std::remove(vector.begin(), vector.end(), key), vector.end());

    return before - vector.size();
}

template <
    typename Key,
    typename Value,
    typename Out = UnallocatedMap<Value, Key>,
    typename In = UnallocatedMap<Key, Value>>
auto reverse_arbitrary_map(const In& map) noexcept -> Out
{
    auto output = Out{};

    for (const auto& [key, value] : map) { output.emplace(value, key); }

    return output;
}

template <typename Key, typename Value>
auto reverse_map(const UnallocatedMap<Key, Value>& map) noexcept
    -> UnallocatedMap<Value, Key>
{
    return reverse_arbitrary_map<Key, Value>(map);
}

template <typename T>
auto span_from_object(T& value) noexcept
{
    return std::span<T, 1_uz>{std::addressof(value), 1_uz};
}
}  // namespace opentxs
