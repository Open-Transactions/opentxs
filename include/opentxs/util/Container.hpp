// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/Export.hpp"

#pragma once

#include <cstddef>
#include <deque>         // IWYU pragma: export
#include <forward_list>  // IWYU pragma: export
#include <functional>
#include <list>           // IWYU pragma: export
#include <map>            // IWYU pragma: export
#include <set>            // IWYU pragma: export
#include <string>         // IWYU pragma: export
#include <unordered_map>  // IWYU pragma: export
#include <unordered_set>  // IWYU pragma: export
#include <vector>         // IWYU pragma: export

// NOTE sometime in early 2023 LLVM version 16 will be released which will
// FINALLY implement std::pmr from c++17 in libc++. That support should
// eventually reach Apple platforms, possibly within the same year. Once that
// happens all this __has_include garbage can be removed.
#if __has_include(<memory_resource>)
#include <memory_resource>  // IWYU pragma: export
#elif __has_include(<experimental/memory_resource>)
#include <experimental/deque>            // IWYU pragma: export
#include <experimental/forward_list>     // IWYU pragma: export
#include <experimental/list>             // IWYU pragma: export
#include <experimental/map>              // IWYU pragma: export
#include <experimental/memory_resource>  // IWYU pragma: export
#include <experimental/set>              // IWYU pragma: export
#include <experimental/string>           // IWYU pragma: export
#include <experimental/unordered_map>    // IWYU pragma: export
#include <experimental/unordered_set>    // IWYU pragma: export
#include <experimental/vector>           // IWYU pragma: export

// clang-format off
namespace std::pmr { using namespace experimental::pmr; }  // NOLINT(cert-dcl58-cpp)
// clang-format on
#else
#error polymorphic allocator support is required
#endif

namespace opentxs
{
using CString = std::pmr::string;
template <typename T>
using Deque = std::pmr::deque<T>;
template <typename T>
using ForwardList = std::pmr::forward_list<T>;
template <typename T>
using List = std::pmr::list<T>;
template <typename K, typename V>
using Map = std::pmr::map<K, V, std::less<>>;
template <typename T>
using Multiset = std::pmr::multiset<T, std::less<>>;
template <typename K, typename V>
using Multimap = std::pmr::multimap<K, V, std::less<>>;
template <typename T>
using Set = std::pmr::set<T, std::less<>>;
template <typename K, typename V>
using UnorderedMap = std::pmr::unordered_map<K, V>;
template <typename K, typename V>
using UnorderedMultimap = std::pmr::unordered_multimap<K, V>;
template <typename T>
using UnorderedMultiset = std::pmr::unordered_multiset<T>;
template <typename T>
using UnorderedSet = std::pmr::unordered_set<T>;
template <typename T>
using Vector = std::pmr::vector<T>;

using UnallocatedCString = std::string;
template <typename T>
using UnallocatedDeque = std::deque<T>;
template <typename T>
using UnallocatedForwardList = std::forward_list<T>;
template <typename T>
using UnallocatedList = std::list<T>;
template <typename K, typename V>
using UnallocatedMap = std::map<K, V, std::less<>>;
template <typename T>
using UnallocatedMultiset = std::multiset<T, std::less<>>;
template <typename K, typename V>
using UnallocatedMultimap = std::multimap<K, V, std::less<>>;
template <typename T>
using UnallocatedSet = std::set<T, std::less<>>;
template <typename K, typename V>
using UnallocatedUnorderedMap = std::unordered_map<K, V>;
template <typename K, typename V>
using UnallocatedUnorderedMultimap = std::unordered_multimap<K, V>;
template <typename T>
using UnallocatedUnorderedMultiset = std::unordered_multiset<T>;
template <typename T>
using UnallocatedUnorderedSet = std::unordered_set<T>;
template <typename T>
using UnallocatedVector = std::vector<T>;
}  // namespace opentxs
