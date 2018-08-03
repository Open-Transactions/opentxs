// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Issuer.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/AccountSummary.hpp"
#include "opentxs/ui/IssuerItem.hpp"
#include "opentxs/Types.hpp"

#include "InternalUI.hpp"
#include "IssuerItemBlank.hpp"
#include "List.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <thread>
#include <tuple>
#include <vector>

#include "AccountSummary.hpp"

#define DEFAULT_ISSUER_NAME "Connecting to Stash Node..."

#define OT_METHOD "opentxs::ui::implementation::AccountSummary::"

namespace opentxs
{
ui::implementation::AccountSummaryExternalInterface* Factory::AccountSummary(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::Wallet& wallet,
    const api::network::ZMQ& connection,
    const api::storage::Storage& storage,
    const api::client::Contacts& contact,
    const api::Legacy& legacy,
    const Identifier& nymID,
    const proto::ContactItemType currency)
{
    return new ui::implementation::AccountSummary(
        zmq,
        publisher,
        wallet,
        connection,
        storage,
        contact,
        legacy,
        nymID,
        currency);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
const Widget::ListenerDefinitions AccountSummary::listeners_{
    {network::zeromq::Socket::IssuerUpdateEndpoint,
     new MessageProcessor<AccountSummary>(&AccountSummary::process_issuer)},
    {network::zeromq::Socket::ServerUpdateEndpoint,
     new MessageProcessor<AccountSummary>(&AccountSummary::process_server)},
    {network::zeromq::Socket::ConnectionStatusEndpoint,
     new MessageProcessor<AccountSummary>(&AccountSummary::process_connection)},
    {network::zeromq::Socket::NymDownloadEndpoint,
     new MessageProcessor<AccountSummary>(&AccountSummary::process_nym)},
};

AccountSummary::AccountSummary(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::Wallet& wallet,
    const api::network::ZMQ& connection,
    const api::storage::Storage& storage,
    const api::client::Contacts& contact,
    const api::Legacy& legacy,
    const Identifier& nymID,
    const proto::ContactItemType currency)
    : AccountSummaryList(nymID, zmq, publisher, contact)
    , wallet_{wallet}
    , connection_{connection}
    , storage_{storage}
    , legacy_{legacy}
    , currency_{currency}
    , issuers_{}
    , server_issuer_map_{}
    , nym_server_map_{}
{
    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&AccountSummary::startup, this));

    OT_ASSERT(startup_)
}

void AccountSummary::construct_row(
    const AccountSummaryRowID& id,
    const AccountSummarySortKey& index,
    const CustomData& custom) const
{
    items_[index].emplace(
        id,
        Factory::IssuerItem(
            *this,
            zmq_,
            publisher_,
            contact_manager_,
            id,
            index,
            custom,
            wallet_,
            storage_,
            legacy_,
            currency_));
    names_.emplace(id, index);
}

AccountSummarySortKey AccountSummary::extract_key(
    const Identifier& nymID,
    const Identifier& issuerID)
{
    AccountSummarySortKey output{false, DEFAULT_ISSUER_NAME};
    auto& [state, name] = output;

    const auto issuer = wallet_.Issuer(nymID, issuerID);

    if (false == bool(issuer)) { return output; }

    const auto serverID = issuer->PrimaryServer();

    if (serverID->empty()) { return output; }

    auto server = wallet_.Server(serverID);

    if (false == bool(server)) { return output; }

    const auto& serverNymID = server->Nym()->ID();
    eLock lock(shared_lock_);
    nym_server_map_.emplace(serverNymID, serverID);
    server_issuer_map_.emplace(serverID, Identifier::Factory(issuerID));
    lock.unlock();

    switch (connection_.Status(serverID->str())) {
        case ConnectionState::ACTIVE: {
            state = true;
        }
        case ConnectionState::NOT_ESTABLISHED:
        case ConnectionState::STALLED:
        default: {
        }
    }

    name = server->Alias();

    return output;
}

void AccountSummary::process_connection(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(2 == message.Body().size());

    const auto serverID = Identifier::Factory(message.Body().at(0));

    return process_server(serverID);
}

void AccountSummary::process_issuer(const Identifier& issuerID)
{
    issuers_.emplace(Identifier::Factory(issuerID));
    const CustomData custom{};
    add_item(issuerID, extract_key(nym_id_, issuerID), custom);
}

void AccountSummary::process_issuer(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(2 == message.Body().size());

    const auto nymID = Identifier::Factory(message.Body().at(0));
    const auto issuerID = Identifier::Factory(message.Body().at(1));

    OT_ASSERT(false == nymID->empty())
    OT_ASSERT(false == issuerID->empty())

    if (nymID.get() != nym_id_) { return; }

    auto existing = names_.count(issuerID);

    if (0 == existing) { process_issuer(issuerID); }
}

void AccountSummary::process_nym(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const auto nymID =
        Identifier::Factory(std::string(*message.Body().begin()));

    sLock lock(shared_lock_);
    const auto it = nym_server_map_.find(nymID);

    if (nym_server_map_.end() == it) { return; }

    const auto serverID = it->second;
    lock.unlock();

    process_server(serverID);
}

void AccountSummary::process_server(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const auto serverID =
        Identifier::Factory(std::string(*message.Body().begin()));

    OT_ASSERT(false == serverID->empty())

    process_server(serverID);
}

void AccountSummary::process_server(const OTIdentifier& serverID)
{
    sLock lock(shared_lock_);
    const auto it = server_issuer_map_.find(serverID);

    if (server_issuer_map_.end() == it) { return; }

    const auto issuerID = it->second;
    lock.unlock();
    const CustomData custom{};
    add_item(issuerID, extract_key(nym_id_, issuerID), custom);
}

void AccountSummary::startup()
{
    const auto issuers = wallet_.IssuerList(nym_id_);
    otWarn << OT_METHOD << __FUNCTION__ << ": Loading " << issuers.size()
           << " issuers." << std::endl;

    for (const auto& id : issuers) { process_issuer(id); }

    startup_complete_->On();
}
}  // namespace opentxs::ui::implementation
