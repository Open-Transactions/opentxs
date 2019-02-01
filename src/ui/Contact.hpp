// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using ContactType = List<
    ContactExternalInterface,
    ContactInternalInterface,
    ContactRowID,
    ContactRowInterface,
    ContactRowInternal,
    ContactRowBlank,
    ContactSortKey>;

class Contact : public ContactType
{
public:
    std::string ContactID() const override;
    std::string DisplayName() const override;
    std::string PaymentCode() const override;

    ~Contact();

private:
    friend opentxs::Factory;

    static const std::set<proto::ContactSectionName> allowed_types_;
    static const std::map<proto::ContactSectionName, int> sort_keys_;

    const ListenerDefinitions listeners_;
    std::string name_;
    std::string payment_code_;

    static int sort_key(const proto::ContactSectionName type);
    static bool check_type(const proto::ContactSectionName type);

    void construct_row(
        const ContactRowID& id,
        const ContactSortKey& index,
        const CustomData& custom) const override;

    bool last(const ContactRowID& id) const override
    {
        return ContactType::last(id);
    }
    void update(ContactRowInterface& row, const CustomData& custom) const;

    void process_contact(const opentxs::Contact& contact);
    void process_contact(const network::zeromq::Message& message);
    void startup();

    Contact(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const identifier::Nym& nymID);
    Contact() = delete;
    Contact(const Contact&) = delete;
    Contact(Contact&&) = delete;
    Contact& operator=(const Contact&) = delete;
    Contact& operator=(Contact&&) = delete;
};
}  // namespace opentxs::ui::implementation
