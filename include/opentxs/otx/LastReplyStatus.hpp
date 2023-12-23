// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/otx/Types.hpp"  // IWYU pragma: keep

namespace opentxs::otx
{
enum class LastReplyStatus : std::underlying_type_t<LastReplyStatus> {
    Invalid = 0,
    None = 1,
    MessageSuccess = 2,
    MessageFailed = 3,
    Unknown = 4,
    NotSent = 5,
};
}  // namespace opentxs::otx
