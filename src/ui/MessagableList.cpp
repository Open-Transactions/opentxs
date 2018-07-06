/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "stdafx.hpp"

#include "opentxs/api/client/Sync.hpp"
#include "opentxs/api/ContactManager.hpp"
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
#include "opentxs/ui/ContactListItem.hpp"
#include "opentxs/ui/MessagableList.hpp"

#include "ContactListItemBlank.hpp"
#include "ContactListParent.hpp"
#include "List.hpp"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "MessagableList.hpp"

#define OT_METHOD "opentxs::ui::implementation::MessagableList::"

namespace opentxs
{
ui::MessagableList* Factory::MessagableList(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const api::client::Sync& sync,
    const Identifier& nymID)
{
    return new ui::implementation::MessagableList(
        zmq, publisher, contact, sync, nymID);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
const Widget::ListenerDefinitions MessagableList::listeners_{
    {network::zeromq::Socket::ContactUpdateEndpoint,
     new MessageProcessor<MessagableList>(&MessagableList::process_contact)},
    {network::zeromq::Socket::NymDownloadEndpoint,
     new MessageProcessor<MessagableList>(&MessagableList::process_nym)},
};

MessagableList::MessagableList(
    const network::zeromq::Context& zmq,
    const network::zeromq::PublishSocket& publisher,
    const api::ContactManager& contact,
    const api::client::Sync& sync,
    const Identifier& nymID)
    : MessagableListList(nymID, zmq, publisher, contact)
    , sync_(sync)
    , owner_contact_id_(Identifier::Factory(last_id_))
{
    init();
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&MessagableList::startup, this));

    OT_ASSERT(startup_)
}

void MessagableList::construct_row(
    const MessagableListRowID& id,
    const MessagableListSortKey& index,
    const CustomData&) const
{
    names_.emplace(id, index);
    items_[index].emplace(
        id,
        Factory::ContactListItem(
            *this, zmq_, publisher_, contact_manager_, id, index));
}

const Identifier& MessagableList::ID() const { return owner_contact_id_; }

void MessagableList::process_contact(
    const MessagableListRowID& id,
    const MessagableListSortKey& key)
{
    if (owner_contact_id_ == id) { return; }

    switch (sync_.CanMessage(nym_id_, id)) {
        case Messagability::READY:
        case Messagability::MISSING_RECIPIENT:
        case Messagability::UNREGISTERED: {
            add_item(id, key, {});
        } break;
        case Messagability::MISSING_SENDER:
        case Messagability::INVALID_SENDER:
        case Messagability::NO_SERVER_CLAIM:
        case Messagability::CONTACT_LACKS_NYM:
        case Messagability::MISSING_CONTACT:
        default: {
            otWarn << OT_METHOD << __FUNCTION__
                   << ": Skipping unmessagable contact " << id->str()
                   << std::endl;
        }
    }
}

void MessagableList::process_contact(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto contactID = Identifier::Factory(id);

    OT_ASSERT(false == contactID->empty())

    const auto name = contact_manager_.ContactName(contactID);
    process_contact(contactID, name);
}

void MessagableList::process_nym(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto nymID = Identifier::Factory(id);

    OT_ASSERT(false == nymID->empty())

    const auto contactID = contact_manager_.ContactID(nymID);
    const auto name = contact_manager_.ContactName(contactID);
    process_contact(contactID, name);
}

void MessagableList::startup()
{
    const auto contacts = contact_manager_.ContactList();
    otWarn << OT_METHOD << __FUNCTION__ << ": Loading " << contacts.size()
           << " contacts." << std::endl;

    for (const auto& [id, alias] : contacts) {
        process_contact(Identifier::Factory(id), alias);
    }

    startup_complete_->On();
}
}  // namespace opentxs::ui::implementation
