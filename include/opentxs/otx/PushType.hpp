// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/otx/Types.hpp"  // IWYU pragma: keep

namespace opentxs::otx
{
enum class PushType : std::underlying_type_t<PushType> {
    Error = 0,
    Nymbox = 1,
    Inbox = 2,
    Outbox = 3,
};
}  // namespace opentxs::otx
