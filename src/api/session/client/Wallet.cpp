// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "api/session/client/Wallet.hpp"  // IWYU pragma: associated

#include <exception>
#include <functional>
#include <string_view>

#include "api/session/Wallet.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/PaymentCode.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/otx/ConsensusType.hpp"
#include "opentxs/otx/consensus/Base.hpp"
#include "opentxs/otx/consensus/Server.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/Context.pb.h"
#include "serialization/protobuf/ServerContext.pb.h"

namespace opentxs::factory
{
auto WalletAPI(const api::session::Client& parent) noexcept
    -> std::unique_ptr<api::session::Wallet>
{
    using ReturnType = api::session::client::Wallet;

    try {

        return std::make_unique<ReturnType>(parent);
    } catch (const std::exception& e) {
        LogError()("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::api::session::client
{
Wallet::Wallet(const api::session::Client& parent)
    : ot_super(parent)
    , client_(parent)
    , request_sent_(client_.Network().ZeroMQ().PublishSocket())
    , reply_received_(client_.Network().ZeroMQ().PublishSocket())
{
    auto bound =
        request_sent_->Start(api_.Endpoints().ServerRequestSent().data());
    bound &=
        reply_received_->Start(api_.Endpoints().ServerReplyReceived().data());

    OT_ASSERT(bound);
}

auto Wallet::Context(
    const identifier::Notary& notaryID,
    const identifier::Nym& clientNymID) const
    -> std::shared_ptr<const otx::context::Base>
{
    auto serverID = Identifier::Factory(notaryID);

    return context(clientNymID, server_to_nym(serverID));
}

void Wallet::instantiate_server_context(
    const proto::Context& serialized,
    const Nym_p& localNym,
    const Nym_p& remoteNym,
    std::shared_ptr<otx::context::internal::Base>& output) const
{
    auto& zmq = client_.ZMQ();
    const auto& server = serialized.servercontext().serverid();
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

auto Wallet::mutable_Context(
    const identifier::Notary& notaryID,
    const identifier::Nym& clientNymID,
    const PasswordPrompt& reason) const -> Editor<otx::context::Base>
{
    auto serverID = Identifier::Factory(notaryID);
    auto base = context(clientNymID, server_to_nym(serverID));
    std::function<void(otx::context::Base*)> callback =
        [&](otx::context::Base* in) -> void {
        this->save(reason, dynamic_cast<otx::context::internal::Base*>(in));
    };

    OT_ASSERT(base);

    return Editor<otx::context::Base>(base.get(), callback);
}

auto Wallet::mutable_ServerContext(
    const identifier::Nym& localNymID,
    const Identifier& remoteID,
    const PasswordPrompt& reason) const -> Editor<otx::context::Server>
{
    Lock lock(context_map_lock_);

    auto serverID = Identifier::Factory(remoteID.str());
    const auto remoteNymID = server_to_nym(serverID);

    auto base = context(localNymID, remoteNymID);

    std::function<void(otx::context::Base*)> callback =
        [&](otx::context::Base* in) -> void {
        this->save(reason, dynamic_cast<otx::context::internal::Base*>(in));
    };

    if (base) {
        OT_ASSERT(otx::ConsensusType::Server == base->Type());
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
        entry.reset(factory::ServerContext(
            client_,
            request_sent_,
            reply_received_,
            localNym,
            remoteNym,
            identifier::Notary::Factory(serverID->str()),  // TODO conversion
            connection));
        base = entry;
    }

    OT_ASSERT(base);

    auto child = dynamic_cast<otx::context::Server*>(base.get());

    OT_ASSERT(nullptr != child);

    return Editor<otx::context::Server>(child, callback);
}

void Wallet::nym_to_contact(
    const identity::Nym& nym,
    const UnallocatedCString& name) const noexcept
{
    auto code = api_.Factory().PaymentCode(nym.PaymentCode());
    client_.Contacts().NewContact(name, nym.ID(), code);
}

auto Wallet::ServerContext(
    const identifier::Nym& localNymID,
    const Identifier& remoteID) const
    -> std::shared_ptr<const otx::context::Server>
{
    auto serverID = Identifier::Factory(remoteID);
    auto remoteNymID = server_to_nym(serverID);
    auto base = context(localNymID, remoteNymID);

    auto output = std::dynamic_pointer_cast<const otx::context::Server>(base);

    return output;
}

auto Wallet::signer_nym(const identifier::Nym& id) const -> Nym_p
{
    return Nym(id);
}
}  // namespace opentxs::api::session::client
