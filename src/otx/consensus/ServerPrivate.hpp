// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::protobuf::DeliveryState

#pragma once

#include <opentxs/protobuf/ConsensusEnums.pb.h>  // IWYU pragma: keep
#include <atomic>
#include <cstdint>
#include <future>
#include <memory>
#include <mutex>

#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/otx/consensus/Base.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/util/Flag.hpp"
#include "opentxs/Export.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "otx/consensus/ConsensusPrivate.hpp"

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

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq

class ServerConnection;
}  // namespace network

namespace otx
{
namespace context
{
class ManagedNumber;
}  // namespace context
}  // namespace otx

namespace protobuf
{
class Context;
}  // namespace protobuf

class Ledger;
class Message;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::context
{
class OPENTXS_NO_EXPORT ServerPrivate final : public ConsensusPrivate
{
public:
    using TransactionNumbers = context::Base::TransactionNumbers;
    using ExtraArgs = context::Server::ExtraArgs;
    using DeliveryResult = context::Server::DeliveryResult;

    const network::zeromq::socket::Publish& request_sent_;
    const network::zeromq::socket::Publish& reply_received_;
    // WARNING the lifetime of the object pointed to by this member variable
    // has a shorter lifetime than this ServerContext object. Call Join()
    // on all ServerContext objects before allowing the client api to shut down.
    std::atomic<const api::session::Client*> client_;
    network::ServerConnection& connection_;
    std::mutex message_lock_;
    UnallocatedCString admin_password_;
    OTFlag admin_attempted_;
    OTFlag admin_success_;
    std::atomic<std::uint64_t> revision_;
    std::atomic<TransactionNumber> highest_transaction_number_;
    TransactionNumbers tentative_transaction_numbers_;
    std::atomic<protobuf::DeliveryState> state_;
    std::atomic<otx::LastReplyStatus> last_status_;
    std::shared_ptr<opentxs::Message> pending_message_;
    ExtraArgs pending_args_;
    std::promise<DeliveryResult> pending_result_;
    std::atomic<bool> pending_result_set_;
    std::atomic<bool> process_nymbox_;
    std::atomic<bool> enable_otx_push_;
    std::atomic<int> failure_counter_;
    std::shared_ptr<Ledger> inbox_;
    std::shared_ptr<Ledger> outbox_;
    UnallocatedSet<otx::context::ManagedNumber>* numbers_;
    OTZMQPushSocket find_nym_;
    OTZMQPushSocket find_server_;
    OTZMQPushSocket find_unit_definition_;

    ServerPrivate(
        const api::session::Client& api,
        const VersionNumber targetVersion,
        const network::zeromq::socket::Publish& requestSent,
        const network::zeromq::socket::Publish& replyReceived,
        network::ServerConnection& connection) noexcept;
    ServerPrivate(
        const api::session::Client& api,
        const VersionNumber targetVersion,
        const network::zeromq::socket::Publish& requestSent,
        const network::zeromq::socket::Publish& replyReceived,
        const protobuf::Context& serialized,
        network::ServerConnection& connection) noexcept;
    ServerPrivate() = delete;
    ServerPrivate(const ServerPrivate&) = delete;
    ServerPrivate(ServerPrivate&&) = delete;
    auto operator=(const ServerPrivate&) -> ServerPrivate& = delete;
    auto operator=(ServerPrivate&&) -> ServerPrivate& = delete;

    ~ServerPrivate() final = default;
};
}  // namespace opentxs::otx::context
