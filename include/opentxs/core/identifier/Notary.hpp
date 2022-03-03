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
class Notary;
}  // namespace identifier

using OTNotaryID = Pimpl<identifier::Notary>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::OTNotaryID> {
    auto operator()(const opentxs::identifier::Notary& data) const noexcept
        -> std::size_t;
};

template <>
struct OPENTXS_EXPORT less<opentxs::OTNotaryID> {
    auto operator()(
        const opentxs::OTNotaryID& lhs,
        const opentxs::OTNotaryID& rhs) const -> bool;
};
}  // namespace std

namespace opentxs
{
OPENTXS_EXPORT auto operator==(
    const OTNotaryID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator!=(
    const OTNotaryID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<(
    const OTNotaryID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>(
    const OTNotaryID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<=(
    const OTNotaryID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>=(
    const OTNotaryID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
}  // namespace opentxs

namespace opentxs::identifier
{
class OPENTXS_EXPORT Notary : virtual public opentxs::Identifier
{
public:
    static auto Factory() -> OTNotaryID;
    static auto Factory(const UnallocatedCString& rhs) -> OTNotaryID;
    static auto Factory(const String& rhs) -> OTNotaryID;

    ~Notary() override = default;

protected:
    Notary() = default;

private:
    friend OTNotaryID;

#ifndef _WIN32
    auto clone() const -> Notary* override = 0;
#endif
    Notary(const Notary&) = delete;
    Notary(Notary&&) = delete;
    auto operator=(const Notary&) -> Notary& = delete;
    auto operator=(Notary&&) -> Notary& = delete;
};
}  // namespace opentxs::identifier
