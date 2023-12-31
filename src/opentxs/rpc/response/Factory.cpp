// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/response/Factory.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/RPCResponse.pb.h>
#include <stdexcept>

#include "opentxs/protobuf/Types.internal.tpp"
#include "opentxs/protobuf/syntax/RPCResponse.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/rpc/CommandType.hpp"  // IWYU pragma: keep
#include "opentxs/rpc/Types.hpp"
#include "opentxs/rpc/Types.internal.hpp"
#include "opentxs/rpc/response/GetAccountActivity.hpp"
#include "opentxs/rpc/response/GetAccountBalance.hpp"
#include "opentxs/rpc/response/ListAccounts.hpp"
#include "opentxs/rpc/response/ListNyms.hpp"
#include "opentxs/rpc/response/Message.hpp"
#include "opentxs/rpc/response/SendPayment.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::rpc::response
{
auto Factory(ReadView protobuf) noexcept -> std::unique_ptr<Message>
{
    try {
        const auto proto = protobuf::Factory<protobuf::RPCResponse>(protobuf);

        if (false == protobuf::syntax::check(LogError(), proto)) {
            throw std::runtime_error{"invalid serialized rpc response"};
        }

        switch (translate(proto.type())) {
            case CommandType::get_account_activity: {

                return std::make_unique<GetAccountActivity>(proto);
            }
            case CommandType::get_account_balance: {

                return std::make_unique<GetAccountBalance>(proto);
            }
            case CommandType::list_accounts: {

                return std::make_unique<ListAccounts>(proto);
            }
            case CommandType::list_nyms: {

                return std::make_unique<ListNyms>(proto);
            }
            case CommandType::send_payment: {

                return std::make_unique<SendPayment>(proto);
            }
            case CommandType::error:
            case CommandType::add_client_session:
            case CommandType::add_server_session:
            case CommandType::list_client_sessions:
            case CommandType::list_server_sessions:
            case CommandType::import_hd_seed:
            case CommandType::list_hd_seeds:
            case CommandType::get_hd_seed:
            case CommandType::create_nym:
            case CommandType::get_nym:
            case CommandType::add_claim:
            case CommandType::delete_claim:
            case CommandType::import_server_contract:
            case CommandType::list_server_contracts:
            case CommandType::register_nym:
            case CommandType::create_unit_definition:
            case CommandType::list_unit_definitions:
            case CommandType::issue_unit_definition:
            case CommandType::create_account:
            case CommandType::move_funds:
            case CommandType::add_contact:
            case CommandType::list_contacts:
            case CommandType::get_contact:
            case CommandType::add_contact_claim:
            case CommandType::delete_contact_claim:
            case CommandType::verify_claim:
            case CommandType::accept_verification:
            case CommandType::send_contact_message:
            case CommandType::get_contact_activity:
            case CommandType::get_server_contract:
            case CommandType::get_pending_payments:
            case CommandType::accept_pending_payments:
            case CommandType::get_compatible_accounts:
            case CommandType::create_compatible_account:
            case CommandType::get_workflow:
            case CommandType::get_server_password:
            case CommandType::get_admin_nym:
            case CommandType::get_unit_definition:
            case CommandType::get_transaction_data:
            case CommandType::lookup_accountid:
            case CommandType::rename_account:
            default: {

                throw std::runtime_error{"unsupported type"};
            }
        }
    } catch (...) {

        return std::make_unique<Message>();
    }
}
}  // namespace opentxs::rpc::response
