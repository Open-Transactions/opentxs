// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/UnitDefinition.hpp"  // IWYU pragma: associated

namespace opentxs::protobuf::inline syntax
{
auto version_2(const UnitDefinition& input, const Log& log, const bool checkSig)
    -> bool
{
    return version_1(input, log, checkSig);
}
}  // namespace opentxs::protobuf::inline syntax
