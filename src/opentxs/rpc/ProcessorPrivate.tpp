// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/protobuf/RPCEnums.pb.h>
#include <memory>

#include "internal/core/String.hpp"
#include "internal/otx/common/Message.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/rpc/ProcessorPrivate.hpp"
#include "opentxs/util/Log.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::rpc
{
template <typename T>
void ProcessorPrivate::evaluate_register_account(
    const api::session::OTX::Result& result,
    T& output) const
{
    const auto& [status, pReply] = result;

    if (otx::LastReplyStatus::NotSent == status) {
        add_output_status(output, protobuf::RPCRESPONSE_ERROR);
    } else if (otx::LastReplyStatus::Unknown == status) {
        add_output_status(output, protobuf::RPCRESPONSE_BAD_SERVER_RESPONSE);
    } else if (otx::LastReplyStatus::MessageFailed == status) {
        add_output_status(
            output, protobuf::RPCRESPONSE_REGISTER_ACCOUNT_FAILED);
    } else if (otx::LastReplyStatus::MessageSuccess == status) {
        assert_false(nullptr == pReply);

        const auto& reply = *pReply;
        add_output_identifier(reply.acct_id_->Get(), output);
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }
}

template <typename T>
void ProcessorPrivate::evaluate_register_nym(
    const api::session::OTX::Result& result,
    T& output) const
{
    const auto& [status, pReply] = result;

    if (otx::LastReplyStatus::NotSent == status) {
        add_output_status(output, protobuf::RPCRESPONSE_ERROR);
    } else if (otx::LastReplyStatus::Unknown == status) {
        add_output_status(output, protobuf::RPCRESPONSE_BAD_SERVER_RESPONSE);
    } else if (otx::LastReplyStatus::MessageFailed == status) {
        add_output_status(output, protobuf::RPCRESPONSE_REGISTER_NYM_FAILED);
    } else if (otx::LastReplyStatus::MessageSuccess == status) {
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    }
}

template <typename T>
void ProcessorPrivate::evaluate_transaction_reply(
    const api::session::Client& client,
    const Message& reply,
    T& output,
    const protobuf::RPCResponseCode code) const
{
    const auto success = evaluate_transaction_reply(client, reply);

    if (success) {
        add_output_status(output, protobuf::RPCRESPONSE_SUCCESS);
    } else {
        add_output_status(output, code);
    }
}
}  // namespace opentxs::rpc
