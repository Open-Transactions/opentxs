// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Allocated.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class IdentifierPrivate;
class UnitDefinition;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::identifier::UnitDefinition> {
    auto operator()(const opentxs::identifier::UnitDefinition& data)
        const noexcept -> std::size_t;
};
}  // namespace std

namespace opentxs::identifier
{
class OPENTXS_EXPORT UnitDefinition : virtual public identifier::Generic
{
public:
    OPENTXS_NO_EXPORT UnitDefinition(IdentifierPrivate* imp) noexcept;
    UnitDefinition(allocator_type alloc = {}) noexcept;
    UnitDefinition(
        const UnitDefinition& rhs,
        allocator_type alloc = {}) noexcept;
    UnitDefinition(UnitDefinition&& rhs) noexcept;
    UnitDefinition(UnitDefinition&& rhs, allocator_type alloc) noexcept;
    auto operator=(const UnitDefinition& rhs) noexcept -> UnitDefinition&;
    auto operator=(UnitDefinition&& rhs) noexcept -> UnitDefinition&;

    ~UnitDefinition() override;
};
}  // namespace opentxs::identifier
