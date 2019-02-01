// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/network/ZMQ.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"
#include "opentxs/network/zeromq/PublishSocket.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountSummary.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/Contact.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/MessagableList.hpp"
#include "opentxs/ui/PayableList.hpp"
#include "opentxs/ui/Profile.hpp"
#include "opentxs/Types.hpp"

#include <map>
#include <memory>
#include <tuple>

#include "UI.hpp"

//#define OT_METHOD "opentxs::api::implementation::UI"

namespace opentxs
{
api::client::UI* Factory::UI(
    const api::client::Manager& api,
    const Flag& running)
{
    return new api::client::implementation::UI(api, running);
}
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
UI::UI(const api::client::Manager& api, const Flag& running)
    : api_(api)
    , running_(running)
    , accounts_()
    , accounts_summaries_()
    , activity_summaries_()
    , contacts_()
    , contact_lists_()
    , messagable_lists_()
    , widget_update_publisher_(api_.ZeroMQ().PublishSocket())
{
    // WARNING: do not access api_.Wallet() during construction
    widget_update_publisher_->Start(api_.Endpoints().WidgetUpdate());
}

const ui::AccountActivity& UI::AccountActivity(
    const identifier::Nym& nymID,
    const Identifier& accountID) const
{
    Lock lock(lock_);
    const AccountKey key(nymID, accountID);
    auto& output = accounts_[key];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::AccountActivity(
            api_, widget_update_publisher_, nymID, accountID));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::AccountSummary& UI::AccountSummary(
    const identifier::Nym& nymID,
    const proto::ContactItemType currency) const
{
    Lock lock(lock_);
    const AccountSummaryKey key{nymID, currency};
    auto& output = accounts_summaries_[key];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::AccountSummary(
            api_, widget_update_publisher_, nymID, currency));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::ActivitySummary& UI::ActivitySummary(
    const identifier::Nym& nymID) const
{
    Lock lock(lock_);
    auto& output = activity_summaries_[nymID];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::ActivitySummary(
            api_, widget_update_publisher_, running_, nymID));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::ActivityThread& UI::ActivityThread(
    const identifier::Nym& nymID,
    const Identifier& threadID) const
{
    Lock lock(lock_);
    auto& output = activity_threads_[{nymID, threadID}];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::ActivityThread(
            api_, widget_update_publisher_, nymID, threadID));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::Contact& UI::Contact(const Identifier& contactID) const
{
    Lock lock(lock_);
    auto id = Identifier::Factory(contactID);
    auto it = contacts_.find(id);

    if (contacts_.end() == it) {
        it = contacts_
                 .emplace(
                     std::move(id),
                     opentxs::Factory::ContactWidget(
                         api_, widget_update_publisher_, contactID))
                 .first;
    }

    OT_ASSERT(it->second)

    return *(it->second);
}

const ui::ContactList& UI::ContactList(const identifier::Nym& nymID) const
{
    Lock lock(lock_);
    auto& output = contact_lists_[nymID];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::ContactList(
            api_, widget_update_publisher_, nymID));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::MessagableList& UI::MessagableList(const identifier::Nym& nymID) const
{
    Lock lock(lock_);
    auto& output = messagable_lists_[nymID];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::MessagableList(
            api_, widget_update_publisher_, nymID));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::PayableList& UI::PayableList(
    const identifier::Nym& nymID,
    proto::ContactItemType currency) const
{
    Lock lock(lock_);
    auto& output =
        payable_lists_[std::pair<OTIdentifier, proto::ContactItemType>(
            nymID, currency)];

    if (false == bool(output)) {
        output.reset(opentxs::Factory::PayableList(
            api_, widget_update_publisher_, nymID, currency));
    }

    OT_ASSERT(output)

    return *output;
}

const ui::Profile& UI::Profile(const identifier::Nym& contactID) const
{
    Lock lock(lock_);
    auto id = Identifier::Factory(contactID);
    auto it = profiles_.find(id);

    if (profiles_.end() == it) {
        it = profiles_
                 .emplace(
                     std::move(id),
                     opentxs::Factory::ProfileWidget(
                         api_, widget_update_publisher_, contactID))
                 .first;
    }

    OT_ASSERT(it->second)

    return *(it->second);
}

UI::~UI() {}
}  // namespace opentxs::api::client::implementation
