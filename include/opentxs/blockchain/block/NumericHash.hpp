// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Hash;
class NumericHash;
class NumericHashPrivate;
}  // namespace block
}  // namespace blockchain

class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block
{
class OPENTXS_EXPORT NumericHash
{
public:
    auto asHex(const std::size_t minimumBytes = 32) const noexcept
        -> UnallocatedCString;
    auto Decimal() const noexcept -> UnallocatedCString;
    auto operator<=>(const NumericHash&) const noexcept -> std::strong_ordering;
    auto operator==(const NumericHash&) const noexcept -> bool;

    NumericHash() noexcept;
    NumericHash(std::uint32_t difficulty) noexcept;
    NumericHash(const Hash& block) noexcept;
    NumericHash(const Data& data) noexcept;
    OPENTXS_NO_EXPORT NumericHash(NumericHashPrivate* imp) noexcept;
    NumericHash(const NumericHash& rhs) noexcept;
    NumericHash(NumericHash&& rhs) noexcept;
    auto operator=(const NumericHash& rhs) noexcept -> NumericHash&;
    auto operator=(NumericHash&& rhs) noexcept -> NumericHash&;

    ~NumericHash();

private:
    NumericHashPrivate* imp_;
};
}  // namespace opentxs::blockchain::block
