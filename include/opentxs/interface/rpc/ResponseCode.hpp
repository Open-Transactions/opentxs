// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/interface/rpc/Types.hpp"  // IWYU pragma: associated

#include <limits>

#include "opentxs/Export.hpp"

namespace opentxs::rpc
{
enum class ResponseCode : TypeEnum {
    invalid = 0,
    success = 1,
    bad_session = 2,
    none = 3,
    queued = 4,
    unnecessary = 5,
    retry = 6,
    no_path_to_recipient = 7,
    bad_server_argument = 8,
    cheque_not_found = 9,
    payment_not_found = 10,
    start_task_failed = 11,
    nym_not_found = 12,
    add_claim_failed = 13,
    add_contact_failed = 14,
    register_account_failed = 15,
    bad_server_response = 16,
    workflow_not_found = 17,
    unit_definition_not_found = 18,
    session_not_found = 19,
    create_nym_failed = 20,
    create_unit_definition_failed = 21,
    delete_claim_failed = 22,
    account_not_found = 23,
    move_funds_failed = 24,
    register_nym_failed = 25,
    contact_not_found = 26,
    account_owner_not_found = 27,
    send_payment_failed = 28,
    transaction_failed = 29,
    txid = 30,
    unimplemented = std::numeric_limits<TypeEnum>::max() - 1u,
    error = std::numeric_limits<TypeEnum>::max(),
};
}  // namespace opentxs::rpc
