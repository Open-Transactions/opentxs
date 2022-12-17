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
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace cfilter
{
class Header;
}  // namespace cfilter
}  // namespace blockchain

struct HexType;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::blockchain::cfilter::Header> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::blockchain::cfilter::Header& data)
        const noexcept -> std::size_t;
};
}  // namespace std

namespace opentxs::blockchain::cfilter
{
class OPENTXS_IMPORT Header : virtual public FixedByteArray<32>
{
public:
    Header() noexcept;
    Header(const ReadView bytes) noexcept(false);
    Header(const HexType&, const ReadView bytes) noexcept(false);
    Header(const Header& rhs) noexcept;
    auto operator=(const Header& rhs) noexcept -> Header&;

    ~Header() override;
};
}  // namespace opentxs::blockchain::cfilter
