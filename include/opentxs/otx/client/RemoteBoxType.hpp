// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/otx/client/Types.hpp"  // IWYU pragma: keep

namespace opentxs::otx::client
{
enum class RemoteBoxType : std::underlying_type_t<RemoteBoxType> {
    Error = -1,
    Nymbox = 0,
    Inbox = 1,
    Outbox = 2,
};
}  // namespace opentxs::otx::client
