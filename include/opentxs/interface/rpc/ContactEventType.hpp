// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"              // IWYU pragma: associated
#include "opentxs/interface/rpc/Types.hpp"  // IWYU pragma: associated

#include <limits>

namespace opentxs::rpc
{
enum class ContactEventType : TypeEnum {
    error = 0,
    incoming_message = 1,
    outgoing_message = 2,
    incoming_payment = 3,
    outgoing_payment = 4,
};
}  // namespace opentxs::rpc
