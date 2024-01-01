// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/BlockchainInputWitness.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/BlockchainInputWitness.pb.h>  // IWYU pragma: keep

namespace opentxs::protobuf::inline syntax
{
auto version_1(const BlockchainInputWitness& input, const Log& log) -> bool
{
    return true;
}
}  // namespace opentxs::protobuf::inline syntax
