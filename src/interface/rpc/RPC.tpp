// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "RPC.hpp"  // IWYU pragma: associated

#include <memory>
#include <type_traits>
#include <utility>

#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/ConsensusEnums.pb.h"
#include "serialization/protobuf/RPCEnums.pb.h"

namespace opentxs::rpc::implementation
{
template <typename T>
void RPC::evaluate_register_account(
    const api::session::OTX::Result& result,
    T& output) const
{
    const auto& [status, pReply] = result;

    if (otx::LastReplyStatus::NotSent == status) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
    } else if (otx::LastReplyStatus::Unknown == status) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SERVER_RESPONSE);
    } else if (otx::LastReplyStatus::MessageFailed == status) {
        add_output_status(output, proto::RPCRESPONSE_REGISTER_ACCOUNT_FAILED);
    } else if (otx::LastReplyStatus::MessageSuccess == status) {
        OT_ASSERT(pReply);

        const auto& reply = *pReply;
        add_output_identifier(reply.m_strAcctID->Get(), output);
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }
}

template <typename T>
void RPC::evaluate_register_nym(
    const api::session::OTX::Result& result,
    T& output) const
{
    const auto& [status, pReply] = result;

    if (otx::LastReplyStatus::NotSent == status) {
        add_output_status(output, proto::RPCRESPONSE_ERROR);
    } else if (otx::LastReplyStatus::Unknown == status) {
        add_output_status(output, proto::RPCRESPONSE_BAD_SERVER_RESPONSE);
    } else if (otx::LastReplyStatus::MessageFailed == status) {
        add_output_status(output, proto::RPCRESPONSE_REGISTER_NYM_FAILED);
    } else if (otx::LastReplyStatus::MessageSuccess == status) {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    }
}

template <typename T>
void RPC::evaluate_transaction_reply(
    const api::session::Client& client,
    const Message& reply,
    T& output,
    const proto::RPCResponseCode code) const
{
    const auto success = evaluate_transaction_reply(client, reply);

    if (success) {
        add_output_status(output, proto::RPCRESPONSE_SUCCESS);
    } else {
        add_output_status(output, code);
    }
}
}  // namespace opentxs::rpc::implementation
