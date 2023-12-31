// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Authority.hpp"  // IWYU pragma: associated

namespace opentxs::protobuf::inline syntax
{
auto version_6(
    const Authority& input,
    const Log& log,
    const Identifier& nymID,
    const KeyMode& key,
    bool& haveHD,
    const AuthorityMode& mode) -> bool
{
    return version_1(input, log, nymID, key, haveHD, mode);
}
}  // namespace opentxs::protobuf::inline syntax
