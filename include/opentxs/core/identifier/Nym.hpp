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
class Nym;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::identifier::Nym> {
    auto operator()(const opentxs::identifier::Nym& data) const noexcept
        -> std::size_t;
};
}  // namespace std

namespace opentxs::identifier
{
class OPENTXS_EXPORT Nym : virtual public identifier::Generic
{
public:
    OPENTXS_NO_EXPORT Nym(Imp* imp) noexcept;
    Nym(allocator_type alloc = {}) noexcept;
    Nym(const Nym& rhs, allocator_type alloc = {}) noexcept;
    Nym(Nym&& rhs) noexcept;
    Nym(Nym&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Nym& rhs) noexcept -> Nym&;
    auto operator=(Nym&& rhs) noexcept -> Nym&;

    ~Nym() override;
};
}  // namespace opentxs::identifier
