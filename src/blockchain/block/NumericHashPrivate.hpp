// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overflow"  // NOLINT
#include <boost/multiprecision/cpp_int.hpp>

#pragma GCC diagnostic pop
#include <compare>
#include <cstddef>
#include <cstdint>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
class NumericHashPrivate
{
public:
    using Type = boost::multiprecision::checked_cpp_int;

    auto asHex(const std::size_t minimumBytes = 32) const noexcept
        -> UnallocatedCString;
    auto Decimal() const noexcept -> UnallocatedCString;
    auto operator<=>(const NumericHashPrivate&) const noexcept
        -> std::strong_ordering;
    auto operator==(const NumericHashPrivate&) const noexcept -> bool;

    NumericHashPrivate() noexcept;
    NumericHashPrivate(std::uint32_t difficulty) noexcept;
    NumericHashPrivate(const Data& data) noexcept;
    NumericHashPrivate(const NumericHashPrivate& rhs) noexcept;
    NumericHashPrivate(NumericHashPrivate&&) = delete;
    auto operator=(const NumericHashPrivate&) noexcept
        -> NumericHashPrivate& = delete;
    auto operator=(NumericHashPrivate&&) noexcept
        -> NumericHashPrivate& = delete;

    ~NumericHashPrivate() = default;

private:
    Type data_;

    NumericHashPrivate(Type&& data) noexcept;
};
}  // namespace opentxs::blockchain::block
