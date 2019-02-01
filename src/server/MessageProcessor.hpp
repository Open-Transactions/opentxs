// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/network/zeromq/Socket.hpp"
#include "opentxs/Proto.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace opentxs::server
{
class MessageProcessor : Lockable
{
public:
    void DropIncoming(const int count) const;
    void DropOutgoing(const int count) const;

    void cleanup();
    void init(const bool inproc, const int port, const OTPassword& privkey);
    void Start();

    explicit MessageProcessor(
        Server& server,
        const network::zeromq::Context& context,
        const Flag& running);

    ~MessageProcessor();

private:
    Server& server_;
    const Flag& running_;
    [[maybe_unused]] const network::zeromq::Context& context_;
    OTZMQListenCallback frontend_callback_;
    OTZMQRouterSocket frontend_socket_;
    OTZMQReplyCallback backend_callback_;
    OTZMQReplySocket backend_socket_;
    OTZMQListenCallback internal_callback_;
    OTZMQDealerSocket internal_socket_;
    OTZMQListenCallback notification_callback_;
    OTZMQPullSocket notification_socket_;
    std::thread thread_;
    const std::string internal_endpoint_;
    mutable std::mutex counter_lock_;
    mutable int drop_incoming_{0};
    mutable int drop_outgoing_{0};
    // nym id, connection identifier
    std::map<OTNymID, OTData> active_connections_;
    mutable std::shared_mutex connection_map_lock_;

    static OTData get_connection(const network::zeromq::Message& incoming);

    proto::ServerRequest extract_proto(
        const network::zeromq::Frame& incoming) const;

    void associate_connection(
        const identifier::Nym& nymID,
        const Data& connection);
    OTZMQMessage process_backend(const network::zeromq::Message& incoming);
    bool process_command(
        const proto::ServerRequest& request,
        identifier::Nym& nymID);
    void process_frontend(const network::zeromq::Message& incoming);
    void process_internal(const network::zeromq::Message& incoming);
    void process_legacy(
        const Data& id,
        const network::zeromq::Message& incoming);
    bool process_message(const std::string& messageString, std::string& reply);
    void process_notification(const network::zeromq::Message& incoming);
    void process_proto(
        const Data& id,
        const network::zeromq::Message& incoming);
    OTData query_connection(const identifier::Nym& nymID);
    void run();

    MessageProcessor() = delete;
};
}  // namespace opentxs::server
