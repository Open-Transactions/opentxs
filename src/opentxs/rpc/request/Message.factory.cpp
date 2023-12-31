// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/rpc/request/internal.factory.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/RPCCommand.pb.h>
#include <stdexcept>

#include "opentxs/protobuf/Types.internal.tpp"
#include "opentxs/protobuf/syntax/RPCCommand.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Types.internal.tpp"
#include "opentxs/rpc/CommandType.hpp"  // IWYU pragma: keep
#include "opentxs/rpc/Types.hpp"
#include "opentxs/rpc/Types.internal.hpp"
#include "opentxs/rpc/request/GetAccountActivity.hpp"
#include "opentxs/rpc/request/GetAccountBalance.hpp"
#include "opentxs/rpc/request/ListAccounts.hpp"
#include "opentxs/rpc/request/ListNyms.hpp"
#include "opentxs/rpc/request/Message.hpp"
#include "opentxs/rpc/request/SendPayment.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto RPCRequest(const protobuf::RPCCommand& proto) noexcept
    -> std::unique_ptr<rpc::request::Message>
{
    using enum rpc::CommandType;

    try {
        if (false == protobuf::syntax::check(LogError(), proto)) {
            throw std::runtime_error{"invalid serialized rpc request"};
        }

        switch (translate(proto.type())) {
            case get_account_activity: {

                return std::make_unique<rpc::request::GetAccountActivity>(
                    proto);
            }
            case get_account_balance: {

                return std::make_unique<rpc::request::GetAccountBalance>(proto);
            }
            case list_accounts: {

                return std::make_unique<rpc::request::ListAccounts>(proto);
            }
            case list_nyms: {

                return std::make_unique<rpc::request::ListNyms>(proto);
            }
            case send_payment: {

                return std::make_unique<rpc::request::SendPayment>(proto);
            }
            case error:
            case add_client_session:
            case add_server_session:
            case list_client_sessions:
            case list_server_sessions:
            case import_hd_seed:
            case list_hd_seeds:
            case get_hd_seed:
            case create_nym:
            case get_nym:
            case add_claim:
            case delete_claim:
            case import_server_contract:
            case list_server_contracts:
            case register_nym:
            case create_unit_definition:
            case list_unit_definitions:
            case issue_unit_definition:
            case create_account:
            case move_funds:
            case add_contact:
            case list_contacts:
            case get_contact:
            case add_contact_claim:
            case delete_contact_claim:
            case verify_claim:
            case accept_verification:
            case send_contact_message:
            case get_contact_activity:
            case get_server_contract:
            case get_pending_payments:
            case accept_pending_payments:
            case get_compatible_accounts:
            case create_compatible_account:
            case get_workflow:
            case get_server_password:
            case get_admin_nym:
            case get_unit_definition:
            case get_transaction_data:
            case lookup_accountid:
            case rename_account:
            default: {
                throw std::runtime_error{"unsupported type"};
            }
        }
    } catch (...) {

        return std::make_unique<rpc::request::Message>();
    }
}

auto RPCRequest(ReadView protobuf) noexcept
    -> std::unique_ptr<rpc::request::Message>
{
    return RPCRequest(protobuf::Factory<protobuf::RPCCommand>(protobuf));
}
}  // namespace opentxs::factory
