// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/identifier/Generic.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class IdentifierPrivate;
class Notary;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::identifier::Notary> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::identifier::Notary& data) const noexcept
        -> std::size_t;
};
}  // namespace std

namespace opentxs::identifier
{
class OPENTXS_EXPORT Notary : virtual public identifier::Generic
{
public:
    OPENTXS_NO_EXPORT Notary(IdentifierPrivate* imp) noexcept;
    Notary(allocator_type alloc = {}) noexcept;
    Notary(const Notary& rhs, allocator_type alloc = {}) noexcept;
    Notary(Notary&& rhs) noexcept;
    Notary(Notary&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Notary& rhs) noexcept -> Notary&;
    auto operator=(Notary&& rhs) noexcept -> Notary&;

    ~Notary() override;
};
}  // namespace opentxs::identifier
