// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/P2PBlockchainSync.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/P2PBlockchainSync.pb.h>  // IWYU pragma: keep
#include <cstdint>
#include <limits>

#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const P2PBlockchainSync& input, const Log& log) -> bool
{
    CHECK_EXISTS(header);
    CHECK_EXISTS(filter);

    constexpr auto max =
        static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max());

    if (max < input.height()) { FAIL_2("height too large", input.height()); }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
