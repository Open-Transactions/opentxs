// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/multiprecision/cpp_int.hpp>
#include <cstddef>
#include <iosfwd>

#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/util/Container.hpp"

namespace bmp = boost::multiprecision;

namespace opentxs::blockchain::implementation
{
class NumericHash final : public blockchain::NumericHash
{
public:
    using Type = bmp::checked_cpp_int;

    auto operator==(const blockchain::NumericHash& rhs) const noexcept
        -> bool final;
    auto operator!=(const blockchain::NumericHash& rhs) const noexcept
        -> bool final;
    auto operator<(const blockchain::NumericHash& rhs) const noexcept
        -> bool final;
    auto operator<=(const blockchain::NumericHash& rhs) const noexcept
        -> bool final;

    auto asHex(const std::size_t minimumBytes) const noexcept
        -> UnallocatedCString final;
    auto Decimal() const noexcept -> UnallocatedCString final
    {
        return data_.str();
    }

    NumericHash(const Type& data) noexcept;
    NumericHash() noexcept;

    ~NumericHash() final = default;

private:
    Type data_;

    auto clone() const noexcept -> NumericHash* final
    {
        return new NumericHash(*this);
    }

    NumericHash(const NumericHash& rhs) noexcept;
    NumericHash(NumericHash&& rhs) = delete;
    auto operator=(const NumericHash& rhs) -> NumericHash& = delete;
    auto operator=(NumericHash&& rhs) -> NumericHash& = delete;
};
}  // namespace opentxs::blockchain::implementation
