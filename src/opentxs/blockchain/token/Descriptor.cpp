// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/token/Descriptor.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstring>

#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs::blockchain::token
{
auto from_hex(
    blockchain::Type chain,
    token::Type type,
    std::string_view hex) noexcept -> Descriptor
{
    auto out = Descriptor{chain, type};
    auto bytes = ByteArray{};
    bytes.DecodeHex(hex);
    using namespace std;
    memcpy(out.id_.data(), bytes.data(), min(out.id_.size(), bytes.size()));

    return out;
}

auto operator==(const Descriptor& lhs, const Descriptor& rhs) noexcept -> bool
{
    if (lhs.host_ != rhs.host_) { return false; }
    if (lhs.type_ != rhs.type_) { return false; }

    return lhs.id_ == rhs.id_;
}

auto operator<=>(const Descriptor& lhs, const Descriptor& rhs) noexcept
    -> std::strong_ordering
{
    constexpr auto& equal = std::strong_ordering::equal;

    if (const auto host = lhs.host_ <=> rhs.host_; equal != host) {

        return host;
    } else if (const auto type = lhs.type_ <=> rhs.type_; equal != type) {

        return type;
    } else {

        return lhs.id_ <=> rhs.id_;
    }
}
}  // namespace opentxs::blockchain::token
