// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <functional>

#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace identifier
{
class UnitDefinition;
}  // namespace identifier

using OTUnitID = Pimpl<identifier::UnitDefinition>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::OTUnitID> {
    auto operator()(const opentxs::identifier::UnitDefinition& data)
        const noexcept -> std::size_t;
};

template <>
struct OPENTXS_EXPORT less<opentxs::OTUnitID> {
    auto operator()(const opentxs::OTUnitID& lhs, const opentxs::OTUnitID& rhs)
        const -> bool;
};
}  // namespace std

namespace opentxs
{
OPENTXS_EXPORT auto operator==(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator!=(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<=(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>=(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
}  // namespace opentxs

namespace opentxs::identifier
{
class OPENTXS_EXPORT UnitDefinition : virtual public opentxs::Identifier
{
public:
    static auto Factory() -> OTUnitID;
    static auto Factory(const UnallocatedCString& rhs) -> OTUnitID;
    static auto Factory(const String& rhs) -> OTUnitID;

    ~UnitDefinition() override = default;

protected:
    UnitDefinition() = default;

private:
    friend OTUnitID;

#ifndef _WIN32
    auto clone() const -> UnitDefinition* override = 0;
#endif

    UnitDefinition(const UnitDefinition&) = delete;
    UnitDefinition(UnitDefinition&&) = delete;
    auto operator=(const UnitDefinition&) -> UnitDefinition& = delete;
    auto operator=(UnitDefinition&&) -> UnitDefinition& = delete;
};
}  // namespace opentxs::identifier
