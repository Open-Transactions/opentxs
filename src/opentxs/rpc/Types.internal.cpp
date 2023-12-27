// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/Types.internal.hpp"  // IWYU pragma: associated

#include <RPCEnums.pb.h>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <functional>

#include "opentxs/rpc/AccountEventType.hpp"  // IWYU pragma: keep
#include "opentxs/rpc/AccountType.hpp"       // IWYU pragma: keep
#include "opentxs/rpc/CommandType.hpp"       // IWYU pragma: keep
#include "opentxs/rpc/ContactEventType.hpp"  // IWYU pragma: keep
#include "opentxs/rpc/PaymentType.hpp"       // IWYU pragma: keep
#include "opentxs/rpc/PushType.hpp"          // IWYU pragma: keep
#include "opentxs/rpc/ResponseCode.hpp"      // IWYU pragma: keep
#include "opentxs/rpc/Types.hpp"

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

static auto account_event_map() noexcept -> AccountEventMap
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
static auto account_map() noexcept -> AccountMap
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
static auto command_map() noexcept -> CommandMap
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
static auto contact_event_map() noexcept -> ContactEventMap
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
static auto payment_map() noexcept -> PaymentMap
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
static auto push_map() noexcept -> PushMap
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
static auto response_code_map() noexcept -> ResponseCodeMap
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

namespace opentxs::rpc
{
auto translate(AccountEventType type) noexcept -> proto::AccountEventType
{
    try {

        return rpc::account_event_map().at(type);
    } catch (...) {

        return proto::ACCOUNTEVENT_ERROR;
    }
}
auto translate(AccountType type) noexcept -> proto::AccountType
{
    try {

        return rpc::account_map().at(type);
    } catch (...) {

        return proto::ACCOUNTTYPE_ERROR;
    }
}
auto translate(CommandType type) noexcept -> proto::RPCCommandType
{
    try {

        return rpc::command_map().at(type);
    } catch (...) {

        return proto::RPCCOMMAND_ERROR;
    }
}
auto translate(ContactEventType type) noexcept -> proto::ContactEventType
{
    try {

        return rpc::contact_event_map().at(type);
    } catch (...) {

        return proto::CONTACTEVENT_ERROR;
    }
}
auto translate(PaymentType type) noexcept -> proto::RPCPaymentType
{
    try {

        return rpc::payment_map().at(type);
    } catch (...) {

        return proto::RPCPAYMENTTYPE_ERROR;
    }
}
auto translate(PushType type) noexcept -> proto::RPCPushType
{
    try {

        return rpc::push_map().at(type);
    } catch (...) {

        return proto::RPCPUSH_ERROR;
    }
}
auto translate(ResponseCode type) noexcept -> proto::RPCResponseCode
{
    try {

        return rpc::response_code_map().at(type);
    } catch (...) {

        return proto::RPCRESPONSE_INVALID;
    }
}
}  // namespace opentxs::rpc

namespace opentxs::proto
{
auto translate(AccountEventType type) noexcept -> rpc::AccountEventType
{
    static const auto map =
        frozen::invert_unordered_map(rpc::account_event_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::AccountEventType::error;
    }
}
auto translate(AccountType type) noexcept -> rpc::AccountType
{
    static const auto map = frozen::invert_unordered_map(rpc::account_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::AccountType::error;
    }
}
auto translate(ContactEventType type) noexcept -> rpc::ContactEventType
{
    static const auto map =
        frozen::invert_unordered_map(rpc::contact_event_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::ContactEventType::error;
    }
}
auto translate(RPCCommandType type) noexcept -> rpc::CommandType
{
    static const auto map = frozen::invert_unordered_map(rpc::command_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::CommandType::error;
    }
}
auto translate(RPCPaymentType type) noexcept -> rpc::PaymentType
{
    static const auto map = frozen::invert_unordered_map(rpc::payment_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::PaymentType::error;
    }
}
auto translate(RPCPushType type) noexcept -> rpc::PushType
{
    static const auto map = frozen::invert_unordered_map(rpc::push_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::PushType::error;
    }
}
auto translate(RPCResponseCode type) noexcept -> rpc::ResponseCode
{
    static const auto map =
        frozen::invert_unordered_map(rpc::response_code_map());

    try {

        return map.at(type);
    } catch (...) {

        return rpc::ResponseCode::invalid;
    }
}
}  // namespace opentxs::proto
