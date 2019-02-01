// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactListList = List<
    ContactListExternalInterface,
    ContactListInternalInterface,
    ContactListRowID,
    ContactListRowInterface,
    ContactListRowInternal,
    ContactListRowBlank,
    ContactListSortKey>;

class ContactList final : public ContactListList
{
public:
    const Identifier& ID() const override { return owner_contact_id_; }

    ~ContactList();

private:
    friend opentxs::Factory;

    const ListenerDefinitions listeners_;
    const OTIdentifier owner_contact_id_;
    std::shared_ptr<ContactListRowInternal> owner_;

    void construct_row(
        const ContactListRowID& id,
        const ContactListSortKey& index,
        const CustomData& custom) const override;
    std::shared_ptr<const ContactListRowInternal> first(
        const Lock& lock) const override;
    bool last(const ContactListRowID& id) const override
    {
        return ContactListList::last(id);
    }

    void add_item(
        const ContactListRowID& id,
        const ContactListSortKey& index,
        const CustomData& custom) override;
    void process_contact(const network::zeromq::Message& message);
    void startup();

    ContactList(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const identifier::Nym& nymID);
    ContactList() = delete;
    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    ContactList& operator=(const ContactList&) = delete;
    ContactList& operator=(ContactList&&) = delete;
};
}  // namespace opentxs::ui::implementation
