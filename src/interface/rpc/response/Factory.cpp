// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "opentxs/interface/rpc/response/Base.hpp"  // IWYU pragma: associated

#include <stdexcept>

#include "Proto.tpp"
#include "internal/interface/rpc/RPC.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/RPCResponse.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/interface/rpc/CommandType.hpp"
#include "opentxs/interface/rpc/response/GetAccountActivity.hpp"
#include "opentxs/interface/rpc/response/GetAccountBalance.hpp"
#include "opentxs/interface/rpc/response/ListAccounts.hpp"
#include "opentxs/interface/rpc/response/ListNyms.hpp"
#include "opentxs/interface/rpc/response/SendPayment.hpp"
#include "serialization/protobuf/RPCResponse.pb.h"

namespace opentxs::rpc::response
{
auto Factory(ReadView serialized) noexcept -> std::unique_ptr<Base>
{
    try {
        const auto proto = proto::Factory<proto::RPCResponse>(serialized);

        if (false == proto::Validate(proto, VERBOSE)) {
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
            default: {

                throw std::runtime_error{"unsupported type"};
            }
        }
    } catch (...) {

        return std::make_unique<Base>();
    }
}
}  // namespace opentxs::rpc::response
