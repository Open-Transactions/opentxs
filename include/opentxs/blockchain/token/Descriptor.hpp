// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <string_view>
// IWYU pragma: no_include <typeindex>

#pragma once

#include <compare>
#include <cstddef>

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

    // NOLINTBEGIN(modernize-use-equals-default)
    constexpr Descriptor(
        blockchain::Type chain = {},
        token::Type type = {},
        FixedByteArray<32> id = {}) noexcept
        : host_(chain)
        , type_(type)
        , id_(id)
    {
    }
    constexpr Descriptor(const Descriptor& rhs) noexcept
        : host_(rhs.host_)
        , type_(rhs.type_)
        , id_(rhs.id_)
    {
    }
    constexpr auto operator=(const Descriptor& rhs) noexcept -> Descriptor&
    {
        host_ = rhs.host_;
        type_ = rhs.type_;
        id_ = rhs.id_;

        return *this;
    }
    // NOLINTEND(modernize-use-equals-default)
};
}  // namespace opentxs::blockchain::token
