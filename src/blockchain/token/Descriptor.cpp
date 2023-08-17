// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/token/Descriptor.hpp"  // IWYU pragma: associated

#include "opentxs/core/Data.hpp"

namespace opentxs::blockchain::token
{
auto operator==(const Descriptor& lhs, const Descriptor& rhs) noexcept -> bool
{
    if (lhs.host_ != rhs.host_) { return false; }
    if (lhs.type_ != rhs.type_) { return false; }

    return lhs.id_ == rhs.id_;
}

auto operator<=>(const Descriptor& lhs, const Descriptor& rhs) noexcept
    -> std::strong_ordering
{
    using namespace std;

    if (const auto v = lhs.host_ <=> rhs.host_; strong_ordering::equal != v) {

        return v;
    }

    if (const auto v = lhs.type_ <=> rhs.type_; strong_ordering::equal != v) {

        return v;
    }

    return lhs.id_ <=> rhs.id_;
}
}  // namespace opentxs::blockchain::token
