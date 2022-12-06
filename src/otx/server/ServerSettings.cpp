// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/server/ServerSettings.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/util/Container.hpp"

namespace opentxs::server
{
// These are default values. There are configurable in ~/.ot/server.cfg
// (static)

std::int64_t ServerSettings::_min_market_scale = 1;
// The number of client requests that will be processed per heartbeat.
std::int32_t ServerSettings::_heartbeat_no_requests = 10;
// number of ms between each heartbeat.
std::int32_t ServerSettings::_heartbeat_ms_between_beats = 100;
// The Nym who's allowed to do certain
// commands even if they are turned off.
UnallocatedCString ServerSettings::_override_nym_id;

// NOTE: These are all static variables, and these are all just default values.
//       (The ACTUAL values are configured in ~/.ot/server.cfg)
//
bool ServerSettings::_admin_usage_credits =
    false;  // Are usage credits REQUIRED in order to use this server?
bool ServerSettings::_admin_server_locked =
    false;  // Is server currently locked to non-override Nyms?
bool ServerSettings::_cmd_usage_credits =
    true;  // Command for setting / viewing usage credits. (Keep this true even
           // if usage credits are turned off. Otherwise the users won't get a
           // server response when they ask it for the policy.)
bool ServerSettings::_cmd_issue_asset = true;
bool ServerSettings::_cmd_get_contract = true;
bool ServerSettings::_cmd_check_notary_id = true;
bool ServerSettings::_cmd_create_user_acct = true;
bool ServerSettings::_cmd_del_user_acct = true;
bool ServerSettings::_cmd_check_nym = true;
bool ServerSettings::_cmd_get_requestnumber = true;
bool ServerSettings::_cmd_get_trans_nums = true;
bool ServerSettings::_cmd_send_message = true;
bool ServerSettings::_cmd_get_nymbox = true;
bool ServerSettings::_cmd_process_nymbox = true;
bool ServerSettings::_cmd_create_asset_acct = true;
bool ServerSettings::_cmd_del_asset_acct = true;
bool ServerSettings::_cmd_get_acct = true;
bool ServerSettings::_cmd_get_inbox = true;
bool ServerSettings::_cmd_get_outbox = true;
bool ServerSettings::_cmd_process_inbox = true;
bool ServerSettings::_cmd_issue_basket = false;
bool ServerSettings::_transact_exchange_basket = true;
bool ServerSettings::_cmd_notarize_transaction = true;
bool ServerSettings::_transact_process_inbox = true;
bool ServerSettings::_transact_transfer = true;
bool ServerSettings::_transact_withdrawal = true;
bool ServerSettings::_transact_deposit = true;
bool ServerSettings::_transact_withdraw_voucher = true;
bool ServerSettings::_transact_deposit_cheque = true;
bool ServerSettings::_transact_pay_dividend = true;
bool ServerSettings::_cmd_get_mint = true;
bool ServerSettings::_transact_withdraw_cash = true;
bool ServerSettings::_transact_deposit_cash = true;
bool ServerSettings::_cmd_get_market_list = true;
bool ServerSettings::_cmd_get_market_offers = true;
bool ServerSettings::_cmd_get_market_recent_trades = true;
bool ServerSettings::_cmd_get_nym_market_offers = true;
bool ServerSettings::_transact_market_offer = true;
bool ServerSettings::_transact_payment_plan = true;
bool ServerSettings::_transact_cancel_cron_item = true;
bool ServerSettings::_transact_smart_contract = true;
bool ServerSettings::_cmd_trigger_clause = true;
bool ServerSettings::_cmd_register_contract = true;
bool ServerSettings::_cmd_request_admin = true;

// Todo: Might set ALL of these to false (so you're FORCED to set them true
// in the server.cfg file.) This way you're also assured that the right data
// folder was found, before you start unlocking the server messages!
//

}  // namespace opentxs::server
