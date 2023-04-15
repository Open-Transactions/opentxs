// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Wallet

#include "api/session/client/Wallet.hpp"  // IWYU pragma: associated

#include <Context.pb.h>
#include <ServerContext.pb.h>
#include <exception>
#include <functional>

#include "internal/api/FactoryAPI.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/otx/consensus/Base.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/PaymentCode.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/otx/ConsensusType.hpp"  // IWYU pragma: keep
#include "opentxs/otx/Types.hpp"
#include "opentxs/util/Log.hpp"

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
    , request_sent_(client_.Network().ZeroMQ().Internal().PublishSocket())
    , reply_received_(client_.Network().ZeroMQ().Internal().PublishSocket())
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
    auto serverID{notaryID};

    return context(clientNymID, server_to_nym(serverID));
}

void Wallet::instantiate_server_context(
    const proto::Context& serialized,
    const Nym_p& localNym,
    const Nym_p& remoteNym,
    std::shared_ptr<otx::context::internal::Base>& output) const
{
    const auto& zmq = client_.ZMQ();
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
    auto serverID{notaryID};
    auto base = context(clientNymID, server_to_nym(serverID));
    std::function<void(otx::context::Base*)> callback =
        [&](otx::context::Base* in) -> void {
        this->save(reason, dynamic_cast<otx::context::internal::Base*>(in));
    };

    OT_ASSERT(base);

    return {base.get(), callback};
}

auto Wallet::mutable_ServerContext(
    const identifier::Nym& localNymID,
    const identifier::Generic& remoteID,
    const PasswordPrompt& reason) const -> Editor<otx::context::Server>
{
    Lock lock(context_map_lock_);
    auto serverID = api_.Factory().Internal().NotaryIDConvertSafe(remoteID);
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
        const ContextID contextID = {
            localNymID.asBase58(api_.Crypto()),
            remoteNymID.asBase58(api_.Crypto())};
        auto& entry = context_map_[contextID];
        const auto& zmq = client_.ZMQ();
        auto& connection = zmq.Server(serverID.asBase58(api_.Crypto()));
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

    OT_ASSERT(base);

    auto* child = dynamic_cast<otx::context::Server*>(base.get());

    OT_ASSERT(nullptr != child);

    return {child, callback};
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
    const identifier::Generic& remoteID) const
    -> std::shared_ptr<const otx::context::Server>
{
    auto serverID{remoteID};
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
