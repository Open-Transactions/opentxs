// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/core/identifier/Nym.hpp"
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
ui::implementation::MessagableExternalInterface* Factory::MessagableList(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const identifier::Nym& nymID)
{
    return new ui::implementation::MessagableList(api, publisher, nymID);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
MessagableList::MessagableList(
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const identifier::Nym& nymID)
    : MessagableListList(api, publisher, nymID)
    , listeners_({
          {api_.Endpoints().ContactUpdate(),
           new MessageProcessor<MessagableList>(
               &MessagableList::process_contact)},
          {api_.Endpoints().NymDownload(),
           new MessageProcessor<MessagableList>(&MessagableList::process_nym)},
      })
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
        id, Factory::ContactListItem(*this, api_, publisher_, id, index));
}

const Identifier& MessagableList::ID() const { return owner_contact_id_; }

void MessagableList::process_contact(
    const MessagableListRowID& id,
    const MessagableListSortKey& key)
{
    if (owner_contact_id_ == id) { return; }

    switch (api_.OTX().CanMessage(nym_id_, id, false)) {
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
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Skipping unmessagable contact ")(id)
                .Flush();
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

    const auto name = api_.Contacts().ContactName(contactID);
    process_contact(contactID, name);
}

void MessagableList::process_nym(const network::zeromq::Message& message)
{
    wait_for_startup();

    OT_ASSERT(1 == message.Body().size());

    const std::string id(*message.Body().begin());
    const auto nymID = identifier::Nym::Factory(id);

    OT_ASSERT(false == nymID->empty())

    const auto contactID = api_.Contacts().ContactID(nymID);
    const auto name = api_.Contacts().ContactName(contactID);
    process_contact(contactID, name);
}

void MessagableList::startup()
{
    const auto contacts = api_.Contacts().ContactList();
    LogDetail(OT_METHOD)(__FUNCTION__)(": Loading ")(contacts.size())(
        " contacts.")
        .Flush();

    for (const auto& [id, alias] : contacts) {
        process_contact(Identifier::Factory(id), alias);
    }

    startup_complete_->On();
}

MessagableList::~MessagableList()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
