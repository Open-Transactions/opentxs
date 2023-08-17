// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <cstddef>
#include <string_view>

#include "opentxs/Export.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/token/Types.hpp"
#include "opentxs/core/FixedByteArray.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace token
{
struct Descriptor;
}  // namespace token
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::blockchain::token::Descriptor> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::blockchain::token::Descriptor& data)
        const noexcept -> std::size_t;
};
}  // namespace std

namespace opentxs::blockchain::token
{
OPENTXS_EXPORT auto operator==(const Descriptor&, const Descriptor&) noexcept
    -> bool;
OPENTXS_EXPORT auto operator<=>(const Descriptor&, const Descriptor&) noexcept
    -> std::strong_ordering;
}  // namespace opentxs::blockchain::token

namespace opentxs::blockchain::token
{
struct OPENTXS_EXPORT Descriptor {
    blockchain::Type host_{};
    token::Type type_{};
    FixedByteArray<32> id_{};
};
}  // namespace opentxs::blockchain::token
