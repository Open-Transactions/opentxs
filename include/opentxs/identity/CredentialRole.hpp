// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/identity/Types.hpp"  // IWYU pragma: keep

namespace opentxs::identity
{
enum class CredentialRole : std::underlying_type_t<CredentialRole> {
    Error = 0,
    MasterKey = 1,
    ChildKey = 2,
    Contact = 3,
    Verify = 4,
};
}  // namespace opentxs::identity
