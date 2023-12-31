// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/client/WalletPrivate.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Context.pb.h>
#include <opentxs/protobuf/ServerContext.pb.h>
#include <functional>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/otx/consensus/Base.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/ZeroMQ.hpp"
#include "opentxs/core/PaymentCode.hpp"  // IWYU pragma: keep
#include "opentxs/identifier/Notary.hpp"
#include "opentxs/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/otx/ConsensusType.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::api::session::client
{
WalletPrivate::WalletPrivate(const api::session::Client& parent)
    : ot_super(parent)
    , client_(parent)
    , request_sent_(
          client_.Network().ZeroMQ().Context().Internal().PublishSocket())
    , reply_received_(
          client_.Network().ZeroMQ().Context().Internal().PublishSocket())
{
    auto bound =
        request_sent_->Start(api_.Endpoints().ServerRequestSent().data());
    bound &=
        reply_received_->Start(api_.Endpoints().ServerReplyReceived().data());

    assert_true(bound);
}

auto WalletPrivate::Context(
    const identifier::Notary& notaryID,
    const identifier::Nym& clientNymID) const
    -> std::shared_ptr<const otx::context::Base>
{
    auto handle = context_map_.lock();
    auto& map = *handle;
    auto serverID{notaryID};

    return context(clientNymID, server_to_nym(serverID), map);
}

void WalletPrivate::instantiate_server_context(
    const protobuf::Context& serialized,
    const Nym_p& localNym,
    const Nym_p& remoteNym,
    std::shared_ptr<otx::context::internal::Base>& output) const
{
    const auto& zmq = client_.ZMQ();
    const auto server = client_.Factory().Internal().NotaryID(
        serialized.servercontext().serverid());
    auto& connection = zmq.Server(server);
    output.reset(factory::ServerContext(
        client_,
        request_sent_,
        reply_received_,
        serialized,
        localNym,
        remoteNym,
        connection));
}

auto WalletPrivate::mutable_Context(
    const identifier::Notary& notaryID,
    const identifier::Nym& clientNymID,
    const PasswordPrompt& reason) const -> Editor<otx::context::Base>
{
    auto handle = context_map_.lock();
    auto& map = *handle;
    auto serverID{notaryID};
    auto base = context(clientNymID, server_to_nym(serverID), map);
    const std::function<void(otx::context::Base*)> callback =
        [&](otx::context::Base* in) -> void {
        this->save(reason, dynamic_cast<otx::context::internal::Base*>(in));
    };

    assert_false(nullptr == base);

    return {base.get(), callback};
}

auto WalletPrivate::mutable_ServerContext(
    const identifier::Nym& localNymID,
    const identifier::Generic& remoteID,
    const PasswordPrompt& reason) const -> Editor<otx::context::Server>
{
    auto handle = context_map_.lock();
    auto& map = *handle;
    auto serverID = api_.Factory().Internal().NotaryIDConvertSafe(remoteID);
    const auto remoteNymID = server_to_nym(serverID);
    auto base = context(localNymID, remoteNymID, map);
    const std::function<void(otx::context::Base*)> callback =
        [&](otx::context::Base* in) -> void {
        this->save(reason, dynamic_cast<otx::context::internal::Base*>(in));
    };

    if (base) {
        assert_true(otx::ConsensusType::Server == base->Type());
    } else {
        // Obtain nyms.
        const auto localNym = Nym(localNymID);

        assert_false(
            nullptr == localNym, "Local nym does not exist in the wallet.");

        const auto remoteNym = Nym(remoteNymID);

        assert_false(
            nullptr == remoteNym, "Remote nym does not exist in the wallet.");

        // Create a new Context
        const ContextID contextID = {
            localNymID.asBase58(api_.Crypto()),
            remoteNymID.asBase58(api_.Crypto())};
        auto& entry = map[contextID];
        const auto& zmq = client_.ZMQ();
        auto& connection = zmq.Server(serverID);
        entry.reset(factory::ServerContext(
            client_,
            request_sent_,
            reply_received_,
            localNym,
            remoteNym,
            serverID,
            connection));
        base = entry;
    }

    assert_false(nullptr == base);

    auto* child = dynamic_cast<otx::context::Server*>(base.get());

    assert_false(nullptr == child);

    return {child, callback};
}

auto WalletPrivate::nym_to_contact(
    const identity::Nym& nym,
    const UnallocatedCString& name) const noexcept -> void
{
    auto code = nym.PaymentCodePublic();
    client_.Contacts().NewContact(name, nym.ID(), code);
}

auto WalletPrivate::ServerContext(
    const identifier::Nym& localNymID,
    const identifier::Generic& remoteID) const
    -> std::shared_ptr<const otx::context::Server>
{
    auto serverID{remoteID};
    auto remoteNymID = server_to_nym(serverID);
    auto handle = context_map_.lock();
    auto& map = *handle;
    auto base = context(localNymID, remoteNymID, map);
    auto output = std::dynamic_pointer_cast<const otx::context::Server>(base);

    return output;
}

auto WalletPrivate::signer_nym(const identifier::Nym& id) const -> Nym_p
{
    return Nym(id);
}
}  // namespace opentxs::api::session::client
