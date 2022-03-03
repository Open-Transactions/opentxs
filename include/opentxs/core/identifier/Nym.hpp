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
class Nym;
}  // namespace identifier

using OTNymID = Pimpl<identifier::Nym>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::OTNymID> {
    auto operator()(const opentxs::identifier::Nym& data) const noexcept
        -> std::size_t;
};

template <>
struct OPENTXS_EXPORT less<opentxs::OTNymID> {
    auto operator()(const opentxs::OTNymID& lhs, const opentxs::OTNymID& rhs)
        const -> bool;
};
}  // namespace std

namespace opentxs
{
OPENTXS_EXPORT auto operator==(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator!=(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<=(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>=(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
}  // namespace opentxs

namespace opentxs::identifier
{
class OPENTXS_EXPORT Nym : virtual public opentxs::Identifier
{
public:
    static auto Factory() -> OTNymID;
    static auto Factory(const UnallocatedCString& rhs) -> OTNymID;
    static auto Factory(const String& rhs) -> OTNymID;
    static auto Factory(const identity::Nym& nym) -> OTNymID;

    ~Nym() override = default;

protected:
    Nym() = default;

private:
    friend OTNymID;

#ifndef _WIN32
    auto clone() const -> Nym* override = 0;
#endif
    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    auto operator=(const Nym&) -> Nym& = delete;
    auto operator=(Nym&&) -> Nym& = delete;
};
}  // namespace opentxs::identifier
