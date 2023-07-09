// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class NumericHash;
}  // namespace block

class Work;
class WorkPrivate;
}  // namespace blockchain

struct HexType;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain
{
OPENTXS_EXPORT auto swap(Work&, Work&) noexcept -> void;
}  // namespace opentxs::blockchain

namespace opentxs::blockchain
{
class OPENTXS_EXPORT Work final : public Allocated
{
public:
    auto asHex(alloc::Strategy alloc) const noexcept -> CString;
    auto asHex() const noexcept -> UnallocatedCString;
    auto Decimal(alloc::Strategy alloc) const noexcept -> CString;
    auto Decimal() const noexcept -> UnallocatedCString;
    auto get_allocator() const noexcept -> allocator_type final;
    auto IsNull() const noexcept -> bool;
    auto operator<=>(const Work&) const noexcept -> std::strong_ordering;
    auto operator==(const Work&) const noexcept -> bool;
    auto operator+(const Work&) const noexcept -> Work;

    auto swap(Work& rhs) noexcept -> void;

    Work(allocator_type alloc = {}) noexcept;
    Work(
        const block::NumericHash& target,
        blockchain::Type chain,
        allocator_type alloc = {}) noexcept;
    Work(
        const HexType&,
        std::string_view hex,
        allocator_type alloc = {}) noexcept;
    OPENTXS_NO_EXPORT Work(WorkPrivate* imp) noexcept;
    Work(const Work& rhs, allocator_type alloc = {}) noexcept;
    Work(Work&& rhs) noexcept;
    Work(Work&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Work& rhs) noexcept -> Work&;
    auto operator=(Work&& rhs) noexcept -> Work&;

    ~Work() final;

private:
    WorkPrivate* imp_;

    auto cleanup(WorkPrivate*) noexcept -> void;
};
}  // namespace opentxs::blockchain
