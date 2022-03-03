// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/network/p2p/Types.hpp"

namespace opentxs::network::p2p
{
enum class MessageType : TypeEnum {
    error = 0,
    sync_request = 1,
    sync_ack = 2,
    sync_reply = 3,
    new_block_header = 4,
    query = 5,
    publish_contract = 6,
    publish_ack = 7,
    contract_query = 8,
    contract = 9,
    pushtx = 10,
    pushtx_reply = 11,
};
}  // namespace opentxs::network::p2p
