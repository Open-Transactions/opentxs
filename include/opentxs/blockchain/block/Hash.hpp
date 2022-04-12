// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <functional>
#include <string_view>

#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
class Hash;
}  // namespace block
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::blockchain::block::Hash> {
    auto operator()(const opentxs::blockchain::block::Hash& data) const noexcept
        -> std::size_t;
};

template <>
struct OPENTXS_EXPORT less<opentxs::blockchain::block::Hash> {
    auto operator()(
        const opentxs::blockchain::block::Hash& lhs,
        const opentxs::blockchain::block::Hash& rhs) const noexcept -> bool;
};
}  // namespace std

namespace opentxs::blockchain::block
{
// NOTE sorry Windows users, MSVC throws an ICE if we export this symbol
class OPENTXS_EXPORT_TEMPLATE Hash : virtual public FixedByteArray<32>
{
public:
    Hash() noexcept;
    Hash(const ReadView bytes) noexcept(false);
    Hash(const Hash& rhs) noexcept;
    auto operator=(const Hash& rhs) noexcept -> Hash&;

    ~Hash() override;
};
}  // namespace opentxs::blockchain::block
