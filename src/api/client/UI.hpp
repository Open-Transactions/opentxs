// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::api::client::implementation
{
class UI : virtual public opentxs::api::client::UI, Lockable
{
public:
    const ui::AccountActivity& AccountActivity(
        const identifier::Nym& nymID,
        const Identifier& accountID) const override;
    const ui::AccountSummary& AccountSummary(
        const identifier::Nym& nymID,
        const proto::ContactItemType currency) const override;
    const ui::ActivitySummary& ActivitySummary(
        const identifier::Nym& nymID) const override;
    const ui::ActivityThread& ActivityThread(
        const identifier::Nym& nymID,
        const Identifier& threadID) const override;
    const ui::Contact& Contact(const Identifier& contactID) const override;
    const ui::ContactList& ContactList(
        const identifier::Nym& nymID) const override;
    const ui::MessagableList& MessagableList(
        const identifier::Nym& nymID) const override;
    const ui::PayableList& PayableList(
        const identifier::Nym& nymID,
        proto::ContactItemType currency) const override;
    const ui::Profile& Profile(const identifier::Nym& contactID) const override;

    ~UI();

private:
    friend opentxs::Factory;

    /** NymID, AccountID */
    using AccountKey = std::pair<OTNymID, OTIdentifier>;
    using AccountActivityMap =
        std::map<AccountKey, std::unique_ptr<ui::AccountActivity>>;
    /** NymID, currency*/
    using AccountSummaryKey = std::pair<OTNymID, proto::ContactItemType>;
    using AccountSummaryMap =
        std::map<AccountSummaryKey, std::unique_ptr<ui::AccountSummary>>;
    using ActivitySummaryMap =
        std::map<OTIdentifier, std::unique_ptr<ui::ActivitySummary>>;
    using ActivityThreadID = std::pair<OTIdentifier, OTIdentifier>;
    using ActivityThreadMap =
        std::map<ActivityThreadID, std::unique_ptr<ui::ActivityThread>>;
    using ContactMap = std::map<OTIdentifier, std::unique_ptr<ui::Contact>>;
    using ContactListMap =
        std::map<OTIdentifier, std::unique_ptr<ui::ContactList>>;
    using MessagableListMap =
        std::map<OTIdentifier, std::unique_ptr<ui::MessagableList>>;
    using PayableListMap = std::map<
        std::pair<OTIdentifier, proto::ContactItemType>,
        std::unique_ptr<ui::PayableList>>;
    using ProfileMap = std::map<OTIdentifier, std::unique_ptr<ui::Profile>>;

    const api::client::Manager& api_;
    const Flag& running_;
    mutable AccountActivityMap accounts_{};
    mutable AccountSummaryMap accounts_summaries_{};
    mutable ActivitySummaryMap activity_summaries_{};
    mutable ContactMap contacts_{};
    mutable ContactListMap contact_lists_{};
    mutable MessagableListMap messagable_lists_{};
    mutable PayableListMap payable_lists_{};
    mutable ActivityThreadMap activity_threads_{};
    mutable ProfileMap profiles_{};
    OTZMQPublishSocket widget_update_publisher_;

    UI(const api::client::Manager& api, const Flag& running);
    UI() = delete;
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
};
}  // namespace opentxs::api::client::implementation
