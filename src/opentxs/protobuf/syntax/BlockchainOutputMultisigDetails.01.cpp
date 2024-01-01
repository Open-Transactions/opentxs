// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlockchainOutputMultisigDetails.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainOutputMultisigDetails.pb.h>
#include <cstddef>

#include "opentxs/protobuf/Types.internal.hpp"
#include "opentxs/protobuf/syntax/Macros.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BlockchainOutputMultisigDetails& input, const Log& log)
    -> bool
{
    if (input.m() > input.n()) {
        FAIL_4("invalid m value", input.m(), " expected <= ", input.n());
    }

    if (static_cast<std::size_t>(input.pubkey().size()) !=
        static_cast<std::size_t>(input.n())) {
        FAIL_4("wrong number of pubkeys", input.m(), " expected ", input.n());
    }

    return true;
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
