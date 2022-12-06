// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/interface/rpc/RPC.hpp"  // IWYU pragma: associated

#include <RPCEnums.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <functional>
#include <string_view>
#include <utility>

#include "opentxs/interface/rpc/AccountEventType.hpp"
#include "opentxs/interface/rpc/AccountType.hpp"
#include "opentxs/interface/rpc/CommandType.hpp"
#include "opentxs/interface/rpc/ContactEventType.hpp"
#include "opentxs/interface/rpc/PaymentType.hpp"
#include "opentxs/interface/rpc/PushType.hpp"
#include "opentxs/interface/rpc/ResponseCode.hpp"
#include "opentxs/interface/rpc/Types.hpp"

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

namespace opentxs::rpc
{
using AccountEventMap =
    frozen::unordered_map<AccountEventType, proto::AccountEventType, 11>;
using AccountEventReverseMap =
    frozen::unordered_map<proto::AccountEventType, AccountEventType, 11>;
using AccountMap = frozen::unordered_map<AccountType, proto::AccountType, 4>;
using AccountReverseMap =
    frozen::unordered_map<proto::AccountType, AccountType, 4>;
using CommandMap =
    frozen::unordered_map<CommandType, proto::RPCCommandType, 46>;
using CommandReverseMap =
    frozen::unordered_map<proto::RPCCommandType, CommandType, 46>;
using ContactEventMap =
    frozen::unordered_map<ContactEventType, proto::ContactEventType, 5>;
using ContactEventReverseMap =
    frozen::unordered_map<proto::ContactEventType, ContactEventType, 5>;
using PaymentMap = frozen::unordered_map<PaymentType, proto::RPCPaymentType, 7>;
using PaymentReverseMap =
    frozen::unordered_map<proto::RPCPaymentType, PaymentType, 7>;
using PushMap = frozen::unordered_map<PushType, proto::RPCPushType, 4>;
using PushReverseMap = frozen::unordered_map<proto::RPCPushType, PushType, 4>;
using ResponseCodeMap =
    frozen::unordered_map<ResponseCode, proto::RPCResponseCode, 33>;
using ResponseCodeReverseMap =
    frozen::unordered_map<proto::RPCResponseCode, ResponseCode, 33>;

auto account_event_map() noexcept -> AccountEventMap;
auto account_event_map() noexcept -> AccountEventMap
{
    using enum AccountEventType;
    using enum proto::AccountEventType;
    static constexpr auto map = AccountEventMap{
        {error, ACCOUNTEVENT_ERROR},
        {incoming_cheque, ACCOUNTEVENT_INCOMINGCHEQUE},
        {outgoing_cheque, ACCOUNTEVENT_OUTGOINGCHEQUE},
        {incoming_transfer, ACCOUNTEVENT_INCOMINGTRANSFER},
        {outgoing_transfer, ACCOUNTEVENT_OUTGOINGTRANSFER},
        {incoming_invoice, ACCOUNTEVENT_INCOMINGINVOICE},
        {outgoing_invoice, ACCOUNTEVENT_OUTGOINGINVOICE},
        {incoming_voucher, ACCOUNTEVENT_INCOMINGVOUCHER},
        {outgoing_voucher, ACCOUNTEVENT_OUTGOINGVOUCHER},
        {incoming_blockchain, ACCOUNTEVENT_INCOMINGBLOCKCHAIN},
        {outgoing_blockchain, ACCOUNTEVENT_OUTGOINGBLOCKCHAIN},
    };

    return map;
}
auto account_map() noexcept -> AccountMap;
auto account_map() noexcept -> AccountMap
{
    using enum AccountType;
    using enum proto::AccountType;
    static constexpr auto map = AccountMap{
        {error, ACCOUNTTYPE_ERROR},
        {normal, ACCOUNTTYPE_NORMAL},
        {issuer, ACCOUNTTYPE_ISSUER},
        {blockchain, ACCOUNTTYPE_BLOCKCHAIN},
    };

    return map;
}
auto command_map() noexcept -> CommandMap;
auto command_map() noexcept -> CommandMap
{
    using enum CommandType;
    using enum proto::RPCCommandType;
    static constexpr auto map = CommandMap{
        {error, RPCCOMMAND_ERROR},
        {add_client_session, RPCCOMMAND_ADDCLIENTSESSION},
        {add_server_session, RPCCOMMAND_ADDSERVERSESSION},
        {list_client_sessions, RPCCOMMAND_LISTCLIENTSESSIONS},
        {list_server_sessions, RPCCOMMAND_LISTSERVERSESSIONS},
        {import_hd_seed, RPCCOMMAND_IMPORTHDSEED},
        {list_hd_seeds, RPCCOMMAND_LISTHDSEEDS},
        {get_hd_seed, RPCCOMMAND_GETHDSEED},
        {create_nym, RPCCOMMAND_CREATENYM},
        {list_nyms, RPCCOMMAND_LISTNYMS},
        {get_nym, RPCCOMMAND_GETNYM},
        {add_claim, RPCCOMMAND_ADDCLAIM},
        {delete_claim, RPCCOMMAND_DELETECLAIM},
        {import_server_contract, RPCCOMMAND_IMPORTSERVERCONTRACT},
        {list_server_contracts, RPCCOMMAND_LISTSERVERCONTRACTS},
        {register_nym, RPCCOMMAND_REGISTERNYM},
        {create_unit_definition, RPCCOMMAND_CREATEUNITDEFINITION},
        {list_unit_definitions, RPCCOMMAND_LISTUNITDEFINITIONS},
        {issue_unit_definition, RPCCOMMAND_ISSUEUNITDEFINITION},
        {create_account, RPCCOMMAND_CREATEACCOUNT},
        {list_accounts, RPCCOMMAND_LISTACCOUNTS},
        {get_account_balance, RPCCOMMAND_GETACCOUNTBALANCE},
        {get_account_activity, RPCCOMMAND_GETACCOUNTACTIVITY},
        {send_payment, RPCCOMMAND_SENDPAYMENT},
        {move_funds, RPCCOMMAND_MOVEFUNDS},
        {add_contact, RPCCOMMAND_ADDCONTACT},
        {list_contacts, RPCCOMMAND_LISTCONTACTS},
        {get_contact, RPCCOMMAND_GETCONTACT},
        {add_contact_claim, RPCCOMMAND_ADDCONTACTCLAIM},
        {delete_contact_claim, RPCCOMMAND_DELETECONTACTCLAIM},
        {verify_claim, RPCCOMMAND_VERIFYCLAIM},
        {accept_verification, RPCCOMMAND_ACCEPTVERIFICATION},
        {send_contact_message, RPCCOMMAND_SENDCONTACTMESSAGE},
        {get_contact_activity, RPCCOMMAND_GETCONTACTACTIVITY},
        {get_server_contract, RPCCOMMAND_GETSERVERCONTRACT},
        {get_pending_payments, RPCCOMMAND_GETPENDINGPAYMENTS},
        {accept_pending_payments, RPCCOMMAND_ACCEPTPENDINGPAYMENTS},
        {get_compatible_accounts, RPCCOMMAND_GETCOMPATIBLEACCOUNTS},
        {create_compatible_account, RPCCOMMAND_CREATECOMPATIBLEACCOUNT},
        {get_workflow, RPCCOMMAND_GETWORKFLOW},
        {get_server_password, RPCCOMMAND_GETSERVERPASSWORD},
        {get_admin_nym, RPCCOMMAND_GETADMINNYM},
        {get_unit_definition, RPCCOMMAND_GETUNITDEFINITION},
        {get_transaction_data, RPCCOMMAND_GETTRANSACTIONDATA},
        {lookup_accountid, RPCCOMMAND_LOOKUPACCOUNTID},
        {rename_account, RPCCOMMAND_RENAMEACCOUNT},
    };

    return map;
}
auto contact_event_map() noexcept -> ContactEventMap;
auto contact_event_map() noexcept -> ContactEventMap
{
    using enum ContactEventType;
    using enum proto::ContactEventType;
    static constexpr auto map = ContactEventMap{
        {error, CONTACTEVENT_ERROR},
        {incoming_message, CONTACTEVENT_INCOMINGMESSAGE},
        {outgoing_message, CONTACTEVENT_OUTGOINGMESSAGE},
        {incoming_payment, CONTACTEVENT_INCOMONGPAYMENT},
        {outgoing_payment, CONTACTEVENT_OUTGOINGPAYMENT},
    };

    return map;
}
auto payment_map() noexcept -> PaymentMap;
auto payment_map() noexcept -> PaymentMap
{
    using enum PaymentType;
    using enum proto::RPCPaymentType;
    static constexpr auto map = PaymentMap{
        {error, RPCPAYMENTTYPE_ERROR},
        {cheque, RPCPAYMENTTYPE_CHEQUE},
        {transfer, RPCPAYMENTTYPE_TRANSFER},
        {voucher, RPCPAYMENTTYPE_VOUCHER},
        {invoice, RPCPAYMENTTYPE_INVOICE},
        {blinded, RPCPAYMENTTYPE_BLINDED},
        {blockchain, RPCPAYMENTTYPE_BLOCKCHAIN},
    };

    return map;
}
auto push_map() noexcept -> PushMap;
auto push_map() noexcept -> PushMap
{
    using enum PushType;
    using enum proto::RPCPushType;
    static constexpr auto map = PushMap{
        {error, RPCPUSH_ERROR},
        {account, RPCPUSH_ACCOUNT},
        {contact, RPCPUSH_CONTACT},
        {task, RPCPUSH_TASK},
    };

    return map;
}
auto response_code_map() noexcept -> ResponseCodeMap;
auto response_code_map() noexcept -> ResponseCodeMap
{
    using enum ResponseCode;
    using enum proto::RPCResponseCode;
    static constexpr auto map = ResponseCodeMap{
        {invalid, RPCRESPONSE_INVALID},
        {success, RPCRESPONSE_SUCCESS},
        {bad_session, RPCRESPONSE_BAD_SESSION},
        {none, RPCRESPONSE_NONE},
        {queued, RPCRESPONSE_QUEUED},
        {unnecessary, RPCRESPONSE_UNNECESSARY},
        {retry, RPCRESPONSE_RETRY},
        {no_path_to_recipient, RPCRESPONSE_NO_PATH_TO_RECIPIENT},
        {bad_server_argument, RPCRESPONSE_BAD_SERVER_ARGUMENT},
        {cheque_not_found, RPCRESPONSE_CHEQUE_NOT_FOUND},
        {payment_not_found, RPCRESPONSE_PAYMENT_NOT_FOUND},
        {start_task_failed, RPCRESPONSE_START_TASK_FAILED},
        {nym_not_found, RPCRESPONSE_NYM_NOT_FOUND},
        {add_claim_failed, RPCRESPONSE_ADD_CLAIM_FAILED},
        {add_contact_failed, RPCRESPONSE_ADD_CONTACT_FAILED},
        {register_account_failed, RPCRESPONSE_REGISTER_ACCOUNT_FAILED},
        {bad_server_response, RPCRESPONSE_BAD_SERVER_RESPONSE},
        {workflow_not_found, RPCRESPONSE_WORKFLOW_NOT_FOUND},
        {unit_definition_not_found, RPCRESPONSE_UNITDEFINITION_NOT_FOUND},
        {session_not_found, RPCRESPONSE_SESSION_NOT_FOUND},
        {create_nym_failed, RPCRESPONSE_CREATE_NYM_FAILED},
        {create_unit_definition_failed,
         RPCRESPONSE_CREATE_UNITDEFINITION_FAILED},
        {delete_claim_failed, RPCRESPONSE_DELETE_CLAIM_FAILED},
        {account_not_found, RPCRESPONSE_ACCOUNT_NOT_FOUND},
        {move_funds_failed, RPCRESPONSE_MOVE_FUNDS_FAILED},
        {register_nym_failed, RPCRESPONSE_REGISTER_NYM_FAILED},
        {contact_not_found, RPCRESPONSE_CONTACT_NOT_FOUND},
        {account_owner_not_found, RPCRESPONSE_ACCOUNT_OWNER_NOT_FOUND},
        {send_payment_failed, RPCRESPONSE_SEND_PAYMENT_FAILED},
        {transaction_failed, RPCRESPONSE_TRANSACTION_FAILED},
        {txid, RPCRESPONSE_TXID},
        {unimplemented, RPCRESPONSE_UNIMPLEMENTED},
        {error, RPCRESPONSE_ERROR},
    };

    return map;
}
}  // namespace opentxs::rpc

namespace opentxs
{
auto translate(const rpc::AccountEventType type) noexcept
    -> proto::AccountEventType
{
    try {

        return rpc::account_event_map().at(type);
    } catch (...) {

        return proto::ACCOUNTEVENT_ERROR;
    }
}
auto translate(const rpc::AccountType type) noexcept -> proto::AccountType
{
    try {

        return rpc::account_map().at(type);
    } catch (...) {

        return proto::ACCOUNTTYPE_ERROR;
    }
}
auto translate(const rpc::CommandType type) noexcept -> proto::RPCCommandType
{
    try {

        return rpc::command_map().at(type);
    } catch (...) {

        return proto::RPCCOMMAND_ERROR;
    }
}
auto translate(const rpc::ContactEventType type) noexcept
    -> proto::ContactEventType
{
    try {

        return rpc::contact_event_map().at(type);
    } catch (...) {

        return proto::CONTACTEVENT_ERROR;
    }
}
auto translate(const rpc::PaymentType type) noexcept -> proto::RPCPaymentType
{
    try {

        return rpc::payment_map().at(type);
    } catch (...) {

        return proto::RPCPAYMENTTYPE_ERROR;
    }
}
auto translate(const rpc::PushType type) noexcept -> proto::RPCPushType
{
    try {

        return rpc::push_map().at(type);
    } catch (...) {

        return proto::RPCPUSH_ERROR;
    }
}
auto translate(const rpc::ResponseCode type) noexcept -> proto::RPCResponseCode
{
    try {

        return rpc::response_code_map().at(type);
    } catch (...) {

        return proto::RPCRESPONSE_INVALID;
    }
}
auto translate(const proto::AccountEventType type) noexcept
    -> rpc::AccountEventType
{
    static const auto map =
        frozen::invert_unordered_map(rpc::account_event_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::AccountEventType::error;
    }
}
auto translate(const proto::AccountType type) noexcept -> rpc::AccountType
{
    static const auto map = frozen::invert_unordered_map(rpc::account_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::AccountType::error;
    }
}
auto translate(const proto::ContactEventType type) noexcept
    -> rpc::ContactEventType
{
    static const auto map =
        frozen::invert_unordered_map(rpc::contact_event_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::ContactEventType::error;
    }
}
auto translate(const proto::RPCCommandType type) noexcept -> rpc::CommandType
{
    static const auto map = frozen::invert_unordered_map(rpc::command_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::CommandType::error;
    }
}
auto translate(const proto::RPCPaymentType type) noexcept -> rpc::PaymentType
{
    static const auto map = frozen::invert_unordered_map(rpc::payment_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::PaymentType::error;
    }
}
auto translate(const proto::RPCPushType type) noexcept -> rpc::PushType
{
    static const auto map = frozen::invert_unordered_map(rpc::push_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::PushType::error;
    }
}
auto translate(const proto::RPCResponseCode type) noexcept -> rpc::ResponseCode
{
    static const auto map =
        frozen::invert_unordered_map(rpc::response_code_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::ResponseCode::invalid;
    }
}
}  // namespace opentxs
