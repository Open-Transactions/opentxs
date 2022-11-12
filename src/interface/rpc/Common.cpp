// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "internal/interface/rpc/RPC.hpp"  // IWYU pragma: associated

#include <RPCEnums.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <robin_hood.h>
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
#include "util/Container.hpp"

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
    robin_hood::unordered_flat_map<AccountEventType, proto::AccountEventType>;
using AccountEventReverseMap =
    robin_hood::unordered_flat_map<proto::AccountEventType, AccountEventType>;
using AccountMap =
    robin_hood::unordered_flat_map<AccountType, proto::AccountType>;
using AccountReverseMap =
    robin_hood::unordered_flat_map<proto::AccountType, AccountType>;
using CommandMap =
    robin_hood::unordered_flat_map<CommandType, proto::RPCCommandType>;
using CommandReverseMap =
    robin_hood::unordered_flat_map<proto::RPCCommandType, CommandType>;
using ContactEventMap =
    robin_hood::unordered_flat_map<ContactEventType, proto::ContactEventType>;
using ContactEventReverseMap =
    robin_hood::unordered_flat_map<proto::ContactEventType, ContactEventType>;
using PaymentMap =
    robin_hood::unordered_flat_map<PaymentType, proto::RPCPaymentType>;
using PaymentReverseMap =
    robin_hood::unordered_flat_map<proto::RPCPaymentType, PaymentType>;
using PushMap = robin_hood::unordered_flat_map<PushType, proto::RPCPushType>;
using PushReverseMap =
    robin_hood::unordered_flat_map<proto::RPCPushType, PushType>;
using ResponseCodeMap =
    robin_hood::unordered_flat_map<ResponseCode, proto::RPCResponseCode>;
using ResponseCodeReverseMap =
    robin_hood::unordered_flat_map<proto::RPCResponseCode, ResponseCode>;

auto account_event_map() noexcept -> AccountEventMap;
auto account_event_map() noexcept -> AccountEventMap
{
    static const auto map = AccountEventMap{
        {AccountEventType::error, proto::ACCOUNTEVENT_ERROR},
        {AccountEventType::incoming_cheque, proto::ACCOUNTEVENT_INCOMINGCHEQUE},
        {AccountEventType::outgoing_cheque, proto::ACCOUNTEVENT_OUTGOINGCHEQUE},
        {AccountEventType::incoming_transfer,
         proto::ACCOUNTEVENT_INCOMINGTRANSFER},
        {AccountEventType::outgoing_transfer,
         proto::ACCOUNTEVENT_OUTGOINGTRANSFER},
        {AccountEventType::incoming_invoice,
         proto::ACCOUNTEVENT_INCOMINGINVOICE},
        {AccountEventType::outgoing_invoice,
         proto::ACCOUNTEVENT_OUTGOINGINVOICE},
        {AccountEventType::incoming_voucher,
         proto::ACCOUNTEVENT_INCOMINGVOUCHER},
        {AccountEventType::outgoing_voucher,
         proto::ACCOUNTEVENT_OUTGOINGVOUCHER},
        {AccountEventType::incoming_blockchain,
         proto::ACCOUNTEVENT_INCOMINGBLOCKCHAIN},
        {AccountEventType::outgoing_blockchain,
         proto::ACCOUNTEVENT_OUTGOINGBLOCKCHAIN},
    };

    return map;
}
auto account_map() noexcept -> AccountMap;
auto account_map() noexcept -> AccountMap
{
    static const auto map = AccountMap{
        {AccountType::error, proto::ACCOUNTTYPE_ERROR},
        {AccountType::normal, proto::ACCOUNTTYPE_NORMAL},
        {AccountType::issuer, proto::ACCOUNTTYPE_ISSUER},
        {AccountType::blockchain, proto::ACCOUNTTYPE_BLOCKCHAIN},
    };

    return map;
}
auto command_map() noexcept -> CommandMap;
auto command_map() noexcept -> CommandMap
{
    static const auto map = CommandMap{
        {CommandType::error, proto::RPCCOMMAND_ERROR},
        {CommandType::add_client_session, proto::RPCCOMMAND_ADDCLIENTSESSION},
        {CommandType::add_server_session, proto::RPCCOMMAND_ADDSERVERSESSION},
        {CommandType::list_client_sessions,
         proto::RPCCOMMAND_LISTCLIENTSESSIONS},
        {CommandType::list_server_sessions,
         proto::RPCCOMMAND_LISTSERVERSESSIONS},
        {CommandType::import_hd_seed, proto::RPCCOMMAND_IMPORTHDSEED},
        {CommandType::list_hd_seeds, proto::RPCCOMMAND_LISTHDSEEDS},
        {CommandType::get_hd_seed, proto::RPCCOMMAND_GETHDSEED},
        {CommandType::create_nym, proto::RPCCOMMAND_CREATENYM},
        {CommandType::list_nyms, proto::RPCCOMMAND_LISTNYMS},
        {CommandType::get_nym, proto::RPCCOMMAND_GETNYM},
        {CommandType::add_claim, proto::RPCCOMMAND_ADDCLAIM},
        {CommandType::delete_claim, proto::RPCCOMMAND_DELETECLAIM},
        {CommandType::import_server_contract,
         proto::RPCCOMMAND_IMPORTSERVERCONTRACT},
        {CommandType::list_server_contracts,
         proto::RPCCOMMAND_LISTSERVERCONTRACTS},
        {CommandType::register_nym, proto::RPCCOMMAND_REGISTERNYM},
        {CommandType::create_unit_definition,
         proto::RPCCOMMAND_CREATEUNITDEFINITION},
        {CommandType::list_unit_definitions,
         proto::RPCCOMMAND_LISTUNITDEFINITIONS},
        {CommandType::issue_unit_definition,
         proto::RPCCOMMAND_ISSUEUNITDEFINITION},
        {CommandType::create_account, proto::RPCCOMMAND_CREATEACCOUNT},
        {CommandType::list_accounts, proto::RPCCOMMAND_LISTACCOUNTS},
        {CommandType::get_account_balance, proto::RPCCOMMAND_GETACCOUNTBALANCE},
        {CommandType::get_account_activity,
         proto::RPCCOMMAND_GETACCOUNTACTIVITY},
        {CommandType::send_payment, proto::RPCCOMMAND_SENDPAYMENT},
        {CommandType::move_funds, proto::RPCCOMMAND_MOVEFUNDS},
        {CommandType::add_contact, proto::RPCCOMMAND_ADDCONTACT},
        {CommandType::list_contacts, proto::RPCCOMMAND_LISTCONTACTS},
        {CommandType::get_contact, proto::RPCCOMMAND_GETCONTACT},
        {CommandType::add_contact_claim, proto::RPCCOMMAND_ADDCONTACTCLAIM},
        {CommandType::delete_contact_claim,
         proto::RPCCOMMAND_DELETECONTACTCLAIM},
        {CommandType::verify_claim, proto::RPCCOMMAND_VERIFYCLAIM},
        {CommandType::accept_verification,
         proto::RPCCOMMAND_ACCEPTVERIFICATION},
        {CommandType::send_contact_message,
         proto::RPCCOMMAND_SENDCONTACTMESSAGE},
        {CommandType::get_contact_activity,
         proto::RPCCOMMAND_GETCONTACTACTIVITY},
        {CommandType::get_server_contract, proto::RPCCOMMAND_GETSERVERCONTRACT},
        {CommandType::get_pending_payments,
         proto::RPCCOMMAND_GETPENDINGPAYMENTS},
        {CommandType::accept_pending_payments,
         proto::RPCCOMMAND_ACCEPTPENDINGPAYMENTS},
        {CommandType::get_compatible_accounts,
         proto::RPCCOMMAND_GETCOMPATIBLEACCOUNTS},
        {CommandType::create_compatible_account,
         proto::RPCCOMMAND_CREATECOMPATIBLEACCOUNT},
        {CommandType::get_workflow, proto::RPCCOMMAND_GETWORKFLOW},
        {CommandType::get_server_password, proto::RPCCOMMAND_GETSERVERPASSWORD},
        {CommandType::get_admin_nym, proto::RPCCOMMAND_GETADMINNYM},
        {CommandType::get_unit_definition, proto::RPCCOMMAND_GETUNITDEFINITION},
        {CommandType::get_transaction_data,
         proto::RPCCOMMAND_GETTRANSACTIONDATA},
        {CommandType::lookup_accountid, proto::RPCCOMMAND_LOOKUPACCOUNTID},
        {CommandType::rename_account, proto::RPCCOMMAND_RENAMEACCOUNT},
    };

    return map;
}
auto contact_event_map() noexcept -> ContactEventMap;
auto contact_event_map() noexcept -> ContactEventMap
{
    static const auto map = ContactEventMap{
        {ContactEventType::error, proto::CONTACTEVENT_ERROR},
        {ContactEventType::incoming_message,
         proto::CONTACTEVENT_INCOMINGMESSAGE},
        {ContactEventType::outgoing_message,
         proto::CONTACTEVENT_OUTGOINGMESSAGE},
        {ContactEventType::incoming_payment,
         proto::CONTACTEVENT_INCOMONGPAYMENT},
        {ContactEventType::outgoing_payment,
         proto::CONTACTEVENT_OUTGOINGPAYMENT},
    };

    return map;
}
auto payment_map() noexcept -> PaymentMap;
auto payment_map() noexcept -> PaymentMap
{
    static const auto map = PaymentMap{
        {PaymentType::error, proto::RPCPAYMENTTYPE_ERROR},
        {PaymentType::cheque, proto::RPCPAYMENTTYPE_CHEQUE},
        {PaymentType::transfer, proto::RPCPAYMENTTYPE_TRANSFER},
        {PaymentType::voucher, proto::RPCPAYMENTTYPE_VOUCHER},
        {PaymentType::invoice, proto::RPCPAYMENTTYPE_INVOICE},
        {PaymentType::blinded, proto::RPCPAYMENTTYPE_BLINDED},
        {PaymentType::blockchain, proto::RPCPAYMENTTYPE_BLOCKCHAIN},
    };

    return map;
}
auto push_map() noexcept -> PushMap;
auto push_map() noexcept -> PushMap
{
    static const auto map = PushMap{
        {PushType::error, proto::RPCPUSH_ERROR},
        {PushType::account, proto::RPCPUSH_ACCOUNT},
        {PushType::contact, proto::RPCPUSH_CONTACT},
        {PushType::task, proto::RPCPUSH_TASK},
    };

    return map;
}
auto response_code_map() noexcept -> ResponseCodeMap;
auto response_code_map() noexcept -> ResponseCodeMap
{
    static const auto map = ResponseCodeMap{
        {ResponseCode::invalid, proto::RPCRESPONSE_INVALID},
        {ResponseCode::success, proto::RPCRESPONSE_SUCCESS},
        {ResponseCode::bad_session, proto::RPCRESPONSE_BAD_SESSION},
        {ResponseCode::none, proto::RPCRESPONSE_NONE},
        {ResponseCode::queued, proto::RPCRESPONSE_QUEUED},
        {ResponseCode::unnecessary, proto::RPCRESPONSE_UNNECESSARY},
        {ResponseCode::retry, proto::RPCRESPONSE_RETRY},
        {ResponseCode::no_path_to_recipient,
         proto::RPCRESPONSE_NO_PATH_TO_RECIPIENT},
        {ResponseCode::bad_server_argument,
         proto::RPCRESPONSE_BAD_SERVER_ARGUMENT},
        {ResponseCode::cheque_not_found, proto::RPCRESPONSE_CHEQUE_NOT_FOUND},
        {ResponseCode::payment_not_found, proto::RPCRESPONSE_PAYMENT_NOT_FOUND},
        {ResponseCode::start_task_failed, proto::RPCRESPONSE_START_TASK_FAILED},
        {ResponseCode::nym_not_found, proto::RPCRESPONSE_NYM_NOT_FOUND},
        {ResponseCode::add_claim_failed, proto::RPCRESPONSE_ADD_CLAIM_FAILED},
        {ResponseCode::add_contact_failed,
         proto::RPCRESPONSE_ADD_CONTACT_FAILED},
        {ResponseCode::register_account_failed,
         proto::RPCRESPONSE_REGISTER_ACCOUNT_FAILED},
        {ResponseCode::bad_server_response,
         proto::RPCRESPONSE_BAD_SERVER_RESPONSE},
        {ResponseCode::workflow_not_found,
         proto::RPCRESPONSE_WORKFLOW_NOT_FOUND},
        {ResponseCode::unit_definition_not_found,
         proto::RPCRESPONSE_UNITDEFINITION_NOT_FOUND},
        {ResponseCode::session_not_found, proto::RPCRESPONSE_SESSION_NOT_FOUND},
        {ResponseCode::create_nym_failed, proto::RPCRESPONSE_CREATE_NYM_FAILED},
        {ResponseCode::create_unit_definition_failed,
         proto::RPCRESPONSE_CREATE_UNITDEFINITION_FAILED},
        {ResponseCode::delete_claim_failed,
         proto::RPCRESPONSE_DELETE_CLAIM_FAILED},
        {ResponseCode::account_not_found, proto::RPCRESPONSE_ACCOUNT_NOT_FOUND},
        {ResponseCode::move_funds_failed, proto::RPCRESPONSE_MOVE_FUNDS_FAILED},
        {ResponseCode::register_nym_failed,
         proto::RPCRESPONSE_REGISTER_NYM_FAILED},
        {ResponseCode::contact_not_found, proto::RPCRESPONSE_CONTACT_NOT_FOUND},
        {ResponseCode::account_owner_not_found,
         proto::RPCRESPONSE_ACCOUNT_OWNER_NOT_FOUND},
        {ResponseCode::send_payment_failed,
         proto::RPCRESPONSE_SEND_PAYMENT_FAILED},
        {ResponseCode::transaction_failed,
         proto::RPCRESPONSE_TRANSACTION_FAILED},
        {ResponseCode::txid, proto::RPCRESPONSE_TXID},
        {ResponseCode::unimplemented, proto::RPCRESPONSE_UNIMPLEMENTED},
        {ResponseCode::error, proto::RPCRESPONSE_ERROR},
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
    static const auto map = reverse_arbitrary_map<
        rpc::AccountEventType,
        proto::AccountEventType,
        rpc::AccountEventReverseMap>(rpc::account_event_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::AccountEventType::error;
    }
}
auto translate(const proto::AccountType type) noexcept -> rpc::AccountType
{
    static const auto map = reverse_arbitrary_map<
        rpc::AccountType,
        proto::AccountType,
        rpc::AccountReverseMap>(rpc::account_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::AccountType::error;
    }
}
auto translate(const proto::ContactEventType type) noexcept
    -> rpc::ContactEventType
{
    static const auto map = reverse_arbitrary_map<
        rpc::ContactEventType,
        proto::ContactEventType,
        rpc::ContactEventReverseMap>(rpc::contact_event_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::ContactEventType::error;
    }
}
auto translate(const proto::RPCCommandType type) noexcept -> rpc::CommandType
{
    static const auto map = reverse_arbitrary_map<
        rpc::CommandType,
        proto::RPCCommandType,
        rpc::CommandReverseMap>(rpc::command_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::CommandType::error;
    }
}
auto translate(const proto::RPCPaymentType type) noexcept -> rpc::PaymentType
{
    static const auto map = reverse_arbitrary_map<
        rpc::PaymentType,
        proto::RPCPaymentType,
        rpc::PaymentReverseMap>(rpc::payment_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::PaymentType::error;
    }
}
auto translate(const proto::RPCPushType type) noexcept -> rpc::PushType
{
    static const auto map = reverse_arbitrary_map<
        rpc::PushType,
        proto::RPCPushType,
        rpc::PushReverseMap>(rpc::push_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::PushType::error;
    }
}
auto translate(const proto::RPCResponseCode type) noexcept -> rpc::ResponseCode
{
    static const auto map = reverse_arbitrary_map<
        rpc::ResponseCode,
        proto::RPCResponseCode,
        rpc::ResponseCodeReverseMap>(rpc::response_code_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::ResponseCode::invalid;
    }
}
}  // namespace opentxs
