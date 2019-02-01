// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/core/identifier/Nym.hpp"

#include "api/Wallet.hpp"

#include "Wallet.hpp"

//#define OT_METHOD "opentxs::api::client::implementation::Wallet::"

namespace opentxs
{
api::Wallet* Factory::Wallet(const api::client::Manager& client)
{
    return new api::client::implementation::Wallet(client);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
Wallet::Wallet(const api::client::Manager& client)
    : ot_super(client)
    , client_(client)
    , request_sent_(client.ZeroMQ().PublishSocket())
    , reply_received_(client.ZeroMQ().PublishSocket())
{
    auto bound = request_sent_->Start(api_.Endpoints().ServerRequestSent());
    bound &= reply_received_->Start(api_.Endpoints().ServerReplyReceived());

    OT_ASSERT(bound);
}

std::shared_ptr<const opentxs::Context> Wallet::Context(
    const Identifier& notaryID,
    const identifier::Nym& clientNymID) const
{
    auto serverID = Identifier::Factory(notaryID);

    return context(clientNymID, server_to_nym(serverID));
}

void Wallet::instantiate_server_context(
    const proto::Context& serialized,
    const std::shared_ptr<const opentxs::Nym>& localNym,
    const std::shared_ptr<const opentxs::Nym>& remoteNym,
    std::shared_ptr<opentxs::internal::Context>& output) const
{
    auto& zmq = client_.ZMQ();
    const auto& server = serialized.servercontext().serverid();
    auto& connection = zmq.Server(server);
    output.reset(opentxs::Factory::ServerContext(
        client_,
        request_sent_,
        reply_received_,
        serialized,
        localNym,
        remoteNym,
        connection));
}

Editor<opentxs::Context> Wallet::mutable_Context(
    const Identifier& notaryID,
    const identifier::Nym& clientNymID) const
{
    auto serverID = Identifier::Factory(notaryID);
    auto base = context(clientNymID, server_to_nym(serverID));
    std::function<void(opentxs::Context*)> callback =
        [&](opentxs::Context* in) -> void {
        this->save(dynamic_cast<opentxs::internal::Context*>(in));
    };

    OT_ASSERT(base);

    return Editor<opentxs::Context>(base.get(), callback);
}

Editor<opentxs::ServerContext> Wallet::mutable_ServerContext(
    const identifier::Nym& localNymID,
    const Identifier& remoteID) const
{
    Lock lock(context_map_lock_);

    auto serverID = Identifier::Factory(remoteID);
    auto remoteNymID = server_to_nym(serverID);

    auto base = context(localNymID, remoteNymID);

    std::function<void(opentxs::Context*)> callback =
        [&](opentxs::Context* in) -> void {
        this->save(dynamic_cast<opentxs::internal::Context*>(in));
    };

    if (base) {
        OT_ASSERT(proto::CONSENSUSTYPE_SERVER == base->Type());
    } else {
        // Obtain nyms.
        const auto localNym = Nym(localNymID);

        OT_ASSERT_MSG(localNym, "Local nym does not exist in the wallet.");

        const auto remoteNym = Nym(remoteNymID);

        OT_ASSERT_MSG(remoteNym, "Remote nym does not exist in the wallet.");

        // Create a new Context
        const ContextID contextID = {localNymID.str(), remoteNymID->str()};
        auto& entry = context_map_[contextID];
        auto& zmq = client_.ZMQ();
        auto& connection = zmq.Server(serverID->str());
        entry.reset(opentxs::Factory::ServerContext(
            client_,
            request_sent_,
            reply_received_,
            localNym,
            remoteNym,
            serverID,
            connection));
        base = entry;
    }

    OT_ASSERT(base);

    auto child = dynamic_cast<opentxs::ServerContext*>(base.get());

    OT_ASSERT(nullptr != child);

    return Editor<opentxs::ServerContext>(child, callback);
}

std::shared_ptr<const opentxs::ServerContext> Wallet::ServerContext(
    const identifier::Nym& localNymID,
    const Identifier& remoteID) const
{
    auto serverID = Identifier::Factory(remoteID);
    auto remoteNymID = server_to_nym(serverID);
    auto base = context(localNymID, remoteNymID);

    auto output = std::dynamic_pointer_cast<const opentxs::ServerContext>(base);

    return output;
}

std::shared_ptr<const opentxs::Nym> Wallet::signer_nym(
    const identifier::Nym& id) const
{
    return Nym(id);
}
}  // namespace opentxs::api::client::implementation
