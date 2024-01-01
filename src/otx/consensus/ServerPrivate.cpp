// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "otx/consensus/ServerPrivate.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ConsensusEnums.pb.h>
#include <opentxs/protobuf/Context.pb.h>
#include <opentxs/protobuf/PendingCommand.pb.h>
#include <opentxs/protobuf/ServerContext.pb.h>
#include <atomic>

#include "internal/network/zeromq/Context.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/util/Flag.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/Types.internal.hpp"
#include "otx/consensus/Server.hpp"

namespace opentxs::otx::context
{
ServerPrivate::ServerPrivate(
    const api::session::Client& api,
    const VersionNumber targetVersion,
    const network::zeromq::socket::Publish& requestSent,
    const network::zeromq::socket::Publish& replyReceived,
    network::ServerConnection& connection) noexcept
    : ConsensusPrivate(targetVersion)
    , request_sent_(requestSent)
    , reply_received_(replyReceived)
    , client_(nullptr)
    , connection_(connection)
    , message_lock_()
    , admin_password_("")
    , admin_attempted_(Flag::Factory(false))
    , admin_success_(Flag::Factory(false))
    , revision_(0)
    , highest_transaction_number_(0)
    , tentative_transaction_numbers_()
    , state_(protobuf::DELIVERTYSTATE_IDLE)
    , last_status_(otx::LastReplyStatus::None)
    , pending_message_()
    , pending_args_("", false)
    , pending_result_()
    , pending_result_set_(false)
    , process_nymbox_(false)
    , enable_otx_push_(true)
    , failure_counter_(0)
    , inbox_()
    , outbox_()
    , numbers_(nullptr)
    , find_nym_(api.Network().ZeroMQ().Context().Internal().PushSocket(
          network::zeromq::socket::Direction::Connect))
    , find_server_(api.Network().ZeroMQ().Context().Internal().PushSocket(
          network::zeromq::socket::Direction::Connect))
    , find_unit_definition_(
          api.Network().ZeroMQ().Context().Internal().PushSocket(
              network::zeromq::socket::Direction::Connect))

{
}

ServerPrivate::ServerPrivate(
    const api::session::Client& api,
    const VersionNumber targetVersion,
    const network::zeromq::socket::Publish& requestSent,
    const network::zeromq::socket::Publish& replyReceived,
    const protobuf::Context& serialized,
    network::ServerConnection& connection) noexcept
    : ConsensusPrivate(api, targetVersion, serialized)
    , request_sent_(requestSent)
    , reply_received_(replyReceived)
    , client_(nullptr)
    , connection_(connection)
    , message_lock_()
    , admin_password_(serialized.servercontext().adminpassword())
    , admin_attempted_(
          Flag::Factory(serialized.servercontext().adminattempted()))
    , admin_success_(Flag::Factory(serialized.servercontext().adminsuccess()))
    , revision_(serialized.servercontext().revision())
    , highest_transaction_number_(
          serialized.servercontext().highesttransactionnumber())
    , tentative_transaction_numbers_()
    , state_(serialized.servercontext().state())
    , last_status_(translate(serialized.servercontext().laststatus()))
    , pending_message_(implementation::Server::instantiate_message(
          api,
          serialized.servercontext().pending().serialized()))
    , pending_args_(
          serialized.servercontext().pending().accountlabel(),
          serialized.servercontext().pending().resync())
    , pending_result_()
    , pending_result_set_(false)
    , process_nymbox_(false)
    , enable_otx_push_(true)
    , failure_counter_(0)
    , inbox_()
    , outbox_()
    , numbers_(nullptr)
    , find_nym_(api.Network().ZeroMQ().Context().Internal().PushSocket(
          network::zeromq::socket::Direction::Connect))
    , find_server_(api.Network().ZeroMQ().Context().Internal().PushSocket(
          network::zeromq::socket::Direction::Connect))
    , find_unit_definition_(
          api.Network().ZeroMQ().Context().Internal().PushSocket(
              network::zeromq::socket::Direction::Connect))
{
    for (const auto& it : serialized.servercontext().tentativerequestnumber()) {
        tentative_transaction_numbers_.insert(it);
    }

    if (3 > serialized.version()) {
        state_.store(protobuf::DELIVERTYSTATE_IDLE);
        last_status_.store(otx::LastReplyStatus::None);
    }
}
}  // namespace opentxs::otx::context
