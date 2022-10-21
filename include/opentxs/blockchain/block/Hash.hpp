// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Platform.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Hash;
}  // namespace block
}  // namespace blockchain

class Amount;
struct HexType;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::blockchain::block::Hash> {
    auto operator()(const opentxs::blockchain::block::Hash& data) const noexcept
        -> std::size_t;
};
}  // namespace std

namespace opentxs::blockchain::block
{
// NOTE sorry Windows users, MSVC throws an ICE if we export this symbol
class OPENTXS_EXPORT_TEMPLATE Hash : virtual public FixedByteArray<32>
{
public:
    auto asAmount() const noexcept -> Amount;

    Hash() noexcept;
    Hash(const ReadView bytes) noexcept(false);
    Hash(const HexType&, const ReadView bytes) noexcept(false);
    Hash(const Hash& rhs) noexcept;
    auto operator=(const Hash& rhs) noexcept -> Hash&;

    ~Hash() override;
};
}  // namespace opentxs::blockchain::block
