// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <limits>

#include "opentxs/Export.hpp"
#include "opentxs/interface/rpc/Types.hpp"

namespace opentxs::rpc
{
enum class CommandType : TypeEnum {
    error = 0,
    add_client_session = 1,
    add_server_session = 2,
    list_client_sessions = 3,
    list_server_sessions = 4,
    import_hd_seed = 5,
    list_hd_seeds = 6,
    get_hd_seed = 7,
    create_nym = 8,
    list_nyms = 9,
    get_nym = 10,
    add_claim = 11,
    delete_claim = 12,
    import_server_contract = 13,
    list_server_contracts = 14,
    register_nym = 15,
    create_unit_definition = 16,
    list_unit_definitions = 17,
    issue_unit_definition = 18,
    create_account = 19,
    list_accounts = 20,
    get_account_balance = 21,
    get_account_activity = 22,
    send_payment = 23,
    move_funds = 24,
    add_contact = 25,
    list_contacts = 26,
    get_contact = 27,
    add_contact_claim = 28,
    delete_contact_claim = 29,
    verify_claim = 30,
    accept_verification = 31,
    send_contact_message = 32,
    get_contact_activity = 33,
    get_server_contract = 34,
    get_pending_payments = 35,
    accept_pending_payments = 36,
    get_compatible_accounts = 37,
    create_compatible_account = 38,
    get_workflow = 39,
    get_server_password = 40,
    get_admin_nym = 41,
    get_unit_definition = 42,
    get_transaction_data = 43,
    lookup_accountid = 44,
    rename_account = 45,
};
}  // namespace opentxs::rpc
