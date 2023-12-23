// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/crypto/Types.hpp"  // IWYU pragma: keep

namespace opentxs::crypto
{
enum class SignatureRole : std::underlying_type_t<SignatureRole> {
    PublicCredential = 0,
    PrivateCredential = 1,
    NymIDSource = 2,
    Claim = 3,
    ServerContract = 4,
    UnitDefinition = 5,
    PeerRequest = 6,
    PeerReply = 7,
    Context = 8,
    Account = 9,
    ServerRequest = 10,
    ServerReply = 11
};
}  // namespace opentxs::crypto
