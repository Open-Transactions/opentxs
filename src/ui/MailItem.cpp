// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"

#include "InternalUI.hpp"
#include "Row.hpp"

#include <memory>
#include <thread>

#include "MailItem.hpp"

namespace opentxs
{
ui::implementation::ActivityThreadRowInternal* Factory::MailItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    const ui::implementation::CustomData& custom,
    const bool loading,
    const bool pending)
{
    return new ui::implementation::MailItem(
        parent,
        api,
        publisher,
        nymID,
        rowID,
        sortKey,
        custom,
        loading,
        pending);
}

ui::implementation::ActivityThreadRowInternal* Factory::MailItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    const ui::implementation::CustomData& custom)
{
    return new ui::implementation::MailItem(
        parent, api, publisher, nymID, rowID, sortKey, custom);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
MailItem::MailItem(
    const ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    const CustomData& custom,
    const bool loading,
    const bool pending)
    : ActivityThreadItem(
          parent,
          api,
          publisher,
          nymID,
          rowID,
          sortKey,
          custom,
          loading,
          pending)
    , load_(nullptr)
{
    OT_ASSERT(false == nym_id_.empty());
    OT_ASSERT(false == item_id_.empty())
}

MailItem::MailItem(
    const ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    const CustomData& custom)
    : MailItem(
          parent,
          api,
          publisher,
          nymID,
          rowID,
          sortKey,
          custom,
          true,
          false)
{
    switch (box_) {
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX: {
            load_.reset(new std::thread(&MailItem::load, this));
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
        }
    }

    OT_ASSERT(load_)
}

void MailItem::load()
{
    std::shared_ptr<const std::string> text{nullptr};

    switch (box_) {
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX: {
            text = api_.Activity().MailText(nym_id_, item_id_, box_);
        } break;
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::INCOMINGBLOCKCHAIN:
        case StorageBox::OUTGOINGBLOCKCHAIN:
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE:
        case StorageBox::DRAFT:
        case StorageBox::UNKNOWN:
        default: {
            OT_FAIL
        }
    }

    OT_ASSERT(text)

    eLock lock(shared_lock_);
    text_ = *text;
    loading_->Off();
    pending_->Off();
    UpdateNotify();
}

MailItem::~MailItem()
{
    if (load_ && load_->joinable()) { load_->join(); }
}
}  // namespace opentxs::ui::implementation
