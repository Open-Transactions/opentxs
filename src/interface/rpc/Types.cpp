// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/rpc/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <functional>  // IWYU pragma: keep
#include <string_view>
#include <utility>

#include "opentxs/interface/rpc/AccountEventType.hpp"  // IWYU pragma: keep
#include "opentxs/interface/rpc/AccountType.hpp"       // IWYU pragma: keep
#include "opentxs/interface/rpc/CommandType.hpp"       // IWYU pragma: keep
#include "opentxs/interface/rpc/ContactEventType.hpp"  // IWYU pragma: keep
#include "opentxs/interface/rpc/PaymentType.hpp"       // IWYU pragma: keep
#include "opentxs/interface/rpc/PushType.hpp"          // IWYU pragma: keep
#include "opentxs/interface/rpc/ResponseCode.hpp"      // IWYU pragma: keep

namespace opentxs::rpc
{
using namespace std::literals;

auto print(AccountEventType in) noexcept -> std::string_view
{
    using enum AccountEventType;
    static constexpr auto map =
        frozen::make_unordered_map<AccountEventType, std::string_view>({
            {incoming_cheque, "incoming cheque"sv},
            {outgoing_cheque, "outgoing cheque"sv},
            {incoming_transfer, "incoming transfer"sv},
            {outgoing_transfer, "outgoing transfer"sv},
            {incoming_invoice, "incoming invoice"sv},
            {outgoing_invoice, "outgoing invoice"sv},
            {incoming_voucher, "incoming voucher"sv},
            {outgoing_voucher, "outgoing voucher"sv},
            {incoming_blockchain, "incoming blockchain"sv},
            {outgoing_blockchain, "outgoing blockchain"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown rpc::AccountEventType"sv;
    }
}

auto print(AccountType in) noexcept -> std::string_view
{
    using enum AccountType;
    static constexpr auto map =
        frozen::make_unordered_map<AccountType, std::string_view>({
            {normal, "custodial"sv},
            {issuer, "custodial issuer"sv},
            {blockchain, "blockchain"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown rpc::AccountType"sv;
    }
}

auto print(CommandType in) noexcept -> std::string_view
{
    using enum CommandType;
    static constexpr auto map =
        frozen::make_unordered_map<CommandType, std::string_view>({
            {add_client_session, "add client session"sv},
            {add_server_session, "add server session"sv},
            {list_client_sessions, "list client sessions"sv},
            {list_server_sessions, "list server sessions"sv},
            {import_hd_seed, "import hd seed"sv},
            {list_hd_seeds, "list hd seeds"sv},
            {get_hd_seed, "get hd seed"sv},
            {create_nym, "create nym"sv},
            {list_nyms, "list nyms"sv},
            {get_nym, "get nym"sv},
            {add_claim, "add claim"sv},
            {delete_claim, "delete claim"sv},
            {import_server_contract, "import server contract"sv},
            {list_server_contracts, "list server contracts"sv},
            {register_nym, "register nym"sv},
            {create_unit_definition, "create unit definition"sv},
            {list_unit_definitions, "list unit definitions"sv},
            {issue_unit_definition, "issue unit definition"sv},
            {create_account, "create account"sv},
            {list_accounts, "list accounts"sv},
            {get_account_balance, "get account balance"sv},
            {get_account_activity, "get account activity"sv},
            {send_payment, "send payment"sv},
            {move_funds, "move funds"sv},
            {add_contact, "add contact"sv},
            {list_contacts, "list contacts"sv},
            {get_contact, "get contact"sv},
            {add_contact_claim, "add contact claim"sv},
            {delete_contact_claim, "delete contact claim"sv},
            {verify_claim, "verify claim"sv},
            {accept_verification, "accept verification"sv},
            {send_contact_message, "send contact message"sv},
            {get_contact_activity, "get contact activity"sv},
            {get_server_contract, "get server contract"sv},
            {get_pending_payments, "get pending payments"sv},
            {accept_pending_payments, "accept pending payments"sv},
            {get_compatible_accounts, "get compatible accounts"sv},
            {create_compatible_account, "create compatible account"sv},
            {get_workflow, "get workflow"sv},
            {get_server_password, "get server password"sv},
            {get_admin_nym, "get admin nym"sv},
            {get_unit_definition, "get unit definition"sv},
            {get_transaction_data, "get transaction data"sv},
            {lookup_accountid, "lookup accountid"sv},
            {rename_account, "rename account"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown rpc::CommandType"sv;
    }
}

auto print(ContactEventType in) noexcept -> std::string_view
{
    using enum ContactEventType;
    static constexpr auto map =
        frozen::make_unordered_map<ContactEventType, std::string_view>({
            {incoming_message, "incoming message"sv},
            {outgoing_message, "outgoing message"sv},
            {incoming_payment, "incoming payment"sv},
            {outgoing_payment, "outgoing payment"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown rpc::ContactEventType"sv;
    }
}

auto print(PaymentType in) noexcept -> std::string_view
{
    using enum PaymentType;
    static constexpr auto map =
        frozen::make_unordered_map<PaymentType, std::string_view>({
            {cheque, "cheque"sv},
            {transfer, "transfer"sv},
            {voucher, "voucher"sv},
            {invoice, "invoice"sv},
            {blinded, "blinded"sv},
            {blockchain, "blockchain"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown rpc::PaymentType"sv;
    }
}

auto print(PushType in) noexcept -> std::string_view
{
    using enum PushType;
    static constexpr auto map =
        frozen::make_unordered_map<PushType, std::string_view>({
            {account, "account"sv},
            {contact, "contact"sv},
            {task, "task"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown rpc::PushType"sv;
    }
}

auto print(ResponseCode in) noexcept -> std::string_view
{
    using enum ResponseCode;
    static constexpr auto map =
        frozen::make_unordered_map<ResponseCode, std::string_view>({
            {invalid, "invalid"sv},
            {success, "success"sv},
            {bad_session, "bad_session"sv},
            {none, "none"sv},
            {queued, "queued"sv},
            {unnecessary, "unnecessary"sv},
            {retry, "retry"sv},
            {no_path_to_recipient, "no path to recipient"sv},
            {bad_server_argument, "bad server argument"sv},
            {cheque_not_found, "cheque not found"sv},
            {payment_not_found, "payment not found"sv},
            {start_task_failed, "start task failed"sv},
            {nym_not_found, "nym not found"sv},
            {add_claim_failed, "add claim failed"sv},
            {add_contact_failed, "add contact failed"sv},
            {register_account_failed, "register account failed"sv},
            {bad_server_response, "bad server response"sv},
            {workflow_not_found, "workflow not found"sv},
            {unit_definition_not_found, "unit definition not found"sv},
            {session_not_found, "session not found"sv},
            {create_nym_failed, "create nym failed"sv},
            {create_unit_definition_failed, "create unit definition failed"sv},
            {delete_claim_failed, "delete claim failed"sv},
            {account_not_found, "account not found"sv},
            {move_funds_failed, "move funds failed"sv},
            {register_nym_failed, "register nym failed"sv},
            {contact_not_found, "contact not found"sv},
            {account_owner_not_found, "account owner not found"sv},
            {send_payment_failed, "send payment failed"sv},
            {transaction_failed, "transaction failed"sv},
            {txid, "txid"sv},
            {unimplemented, "unimplemented"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown rpc::ResponseCode"sv;
    }
}
}  // namespace opentxs::rpc
