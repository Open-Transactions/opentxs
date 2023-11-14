// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/multiprecision/cpp_bin_float.hpp>  // IWYU pragma: keep
#include <boost/multiprecision/cpp_int.hpp>
#include <compare>

#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain
{
class WorkPrivate final : public opentxs::pmr::Allocated
{
public:
    using Type = boost::multiprecision::cpp_bin_float_double;

    auto IsNull() const noexcept -> bool;
    auto operator==(const WorkPrivate&) const noexcept -> bool;
    auto operator<=>(const WorkPrivate&) const noexcept -> std::strong_ordering;
    auto operator+(const WorkPrivate& rhs) const noexcept -> Type;

    auto asHex(allocator_type alloc) const noexcept -> CString;
    [[nodiscard]] auto clone(allocator_type alloc) const noexcept
        -> WorkPrivate*
    {
        return pmr::clone(this, {alloc});
    }
    auto Decimal(allocator_type alloc) const noexcept -> CString;

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    WorkPrivate(Type&& data, allocator_type alloc) noexcept;
    WorkPrivate(allocator_type alloc) noexcept;
    WorkPrivate() = delete;
    WorkPrivate(const WorkPrivate& rhs, allocator_type alloc) noexcept;
    WorkPrivate(WorkPrivate&& rhs) = delete;
    auto operator=(const WorkPrivate& rhs) -> WorkPrivate& = delete;
    auto operator=(WorkPrivate&& rhs) -> WorkPrivate& = delete;

    ~WorkPrivate() final = default;

private:
    Type data_;
};
}  // namespace opentxs::blockchain
