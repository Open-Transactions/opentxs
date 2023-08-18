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

    // TODO this can be improved after XCode 15 and Android NDK 26

    if (lhs.host_ < rhs.host_) {

        return strong_ordering::less;
    } else if (lhs.host_ > rhs.host_) {

        return strong_ordering::greater;
    } else if (lhs.type_ < rhs.type_) {

        return strong_ordering::less;
    } else if (lhs.type_ > rhs.type_) {

        return strong_ordering::greater;
    } else {

        return lhs.id_ <=> rhs.id_;
    }
}
}  // namespace opentxs::blockchain::token
