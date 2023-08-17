// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/blockchain/protocol/bitcoin/bitcoincash/token/Types.hpp"

namespace opentxs::blockchain::protocol::bitcoin::bitcoincash::token::cashtoken
{
enum class Capability : std::underlying_type<Capability>::type {
    none = 0,
    mutable_token = 1,
    minting = 2,
};
}  // namespace
   // opentxs::blockchain::protocol::bitcoin::bitcoincash::token::cashtoken
