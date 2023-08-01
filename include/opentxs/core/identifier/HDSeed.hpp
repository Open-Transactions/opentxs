// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/core/identifier/Generic.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class IdentifierPrivate;
class HDSeed;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::identifier::HDSeed> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::identifier::HDSeed& data) const noexcept
        -> std::size_t;
};
}  // namespace std

namespace opentxs::identifier
{
class OPENTXS_EXPORT HDSeed : virtual public identifier::Generic
{
public:
    OPENTXS_NO_EXPORT HDSeed(IdentifierPrivate* imp) noexcept;
    HDSeed(allocator_type alloc = {}) noexcept;
    HDSeed(const HDSeed& rhs, allocator_type alloc = {}) noexcept;
    HDSeed(HDSeed&& rhs) noexcept;
    HDSeed(HDSeed&& rhs, allocator_type alloc) noexcept;
    auto operator=(const HDSeed& rhs) noexcept -> HDSeed&;
    auto operator=(HDSeed&& rhs) noexcept -> HDSeed&;

    ~HDSeed() override;
};
}  // namespace opentxs::identifier
