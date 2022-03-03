// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <functional>
#include <string_view>
#include <type_traits>

#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace internal
{
class Amount;
}  // namespace internal

class Amount;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct hash<opentxs::Amount> {
    auto operator()(const opentxs::Amount& data) const noexcept -> std::size_t;
};
}  // namespace std

namespace opentxs
{
template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
OPENTXS_EXPORT auto operator==(T lhs, const Amount& rhs) noexcept -> bool;

template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
OPENTXS_EXPORT auto operator!=(T lhs, const Amount& rhs) noexcept -> bool;

template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
OPENTXS_EXPORT auto operator<(T lhs, const Amount& rhs) noexcept -> bool;

template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
OPENTXS_EXPORT auto operator<=(T lhs, const Amount& rhs) noexcept -> bool;

template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
OPENTXS_EXPORT auto operator>(T lhs, const Amount& rhs) noexcept -> bool;

template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
OPENTXS_EXPORT auto operator>=(T lhs, const Amount& rhs) noexcept -> bool;

template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
OPENTXS_EXPORT auto operator*(T lhs, const Amount& rhs) noexcept(false)
    -> Amount;

OPENTXS_EXPORT auto signed_amount(
    long long ip,
    unsigned long long fp = 0,
    unsigned long long div = 0) -> Amount;
OPENTXS_EXPORT auto unsigned_amount(
    unsigned long long ip,
    unsigned long long fp = 0,
    unsigned long long div = 0) -> Amount;
}  // namespace opentxs

namespace opentxs
{
OPENTXS_EXPORT auto swap(Amount& lhs, Amount& rhs) noexcept -> void;

class OPENTXS_EXPORT Amount
{
public:
    class Imp;

    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Amount&;

    auto operator==(const Amount& rhs) const noexcept -> bool;
    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    auto operator==(const T) const noexcept -> bool;

    auto operator!=(const Amount& rhs) const noexcept -> bool;
    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    auto operator!=(const T) const noexcept -> bool;

    auto operator<(const Amount& rhs) const noexcept -> bool;
    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    auto operator<(const T) const noexcept -> bool;

    auto operator<=(const Amount& rhs) const noexcept -> bool;
    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    auto operator<=(const T) const noexcept -> bool;

    auto operator>(const Amount& rhs) const noexcept -> bool;
    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    auto operator>(const T) const noexcept -> bool;

    auto operator>=(const Amount& rhs) const noexcept -> bool;
    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    auto operator>=(const T) const noexcept -> bool;

    auto operator+(const Amount& rhs) const noexcept(false) -> Amount;
    auto operator-(const Amount& rhs) const noexcept(false) -> Amount;

    auto operator*(const Amount& rhs) const noexcept(false) -> Amount;
    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    auto operator*(const T) const noexcept(false) -> Amount;

    auto operator/(const Amount& rhs) const noexcept(false) -> Amount;
    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    auto operator/(const T) const noexcept(false) -> Amount;

    auto operator%(const Amount& rhs) const noexcept(false) -> Amount;
    template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
    auto operator%(const T) const noexcept(false) -> Amount;

    auto operator*=(const Amount& amount) noexcept(false) -> Amount&;
    auto operator+=(const Amount& amount) noexcept(false) -> Amount&;
    auto operator-=(const Amount& amount) noexcept(false) -> Amount&;
    auto operator-() -> Amount;

    auto Serialize(const AllocateOutput dest) const noexcept -> bool;

    auto swap(Amount& rhs) noexcept -> void;

    Amount(int);
    Amount(long);
    Amount(long long);
    Amount(unsigned);
    Amount(unsigned long);
    Amount(unsigned long long);
    OPENTXS_NO_EXPORT Amount(const Imp&) noexcept;
    OPENTXS_NO_EXPORT Amount(Imp*) noexcept;
    Amount() noexcept;
    Amount(const Amount& rhs) noexcept;
    Amount(Amount&& rhs) noexcept;
    auto operator=(const Amount&) noexcept -> Amount&;
    auto operator=(Amount&&) noexcept -> Amount&;

    ~Amount();

private:
    Imp* imp_;
};
}  // namespace opentxs
