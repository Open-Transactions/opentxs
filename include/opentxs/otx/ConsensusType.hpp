// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/otx/Types.hpp"  // IWYU pragma: keep

namespace opentxs::otx
{
enum class ConsensusType : std::underlying_type_t<ConsensusType> {
    Error = 0,
    Server = 1,
    Client = 2,
    Peer = 3,
};
}  // namespace opentxs::otx
