// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using PayableListList = List<
    PayableExternalInterface,
    PayableInternalInterface,
    PayableListRowID,
    PayableListRowInterface,
    PayableListRowInternal,
    PayableListRowBlank,
    PayableListSortKey>;

class PayableList final : public PayableListList
{
public:
    const Identifier& ID() const override;

    ~PayableList();

private:
    friend opentxs::Factory;

    const ListenerDefinitions listeners_;
    const OTIdentifier owner_contact_id_;
    const proto::ContactItemType currency_;

    void construct_row(
        const PayableListRowID& id,
        const PayableListSortKey& index,
        const CustomData& custom) const override;
    bool last(const PayableListRowID& id) const override
    {
        return PayableListList::last(id);
    }

    void process_contact(
        const PayableListRowID& id,
        const PayableListSortKey& key);
    void process_contact(const network::zeromq::Message& message);
    void process_nym(const network::zeromq::Message& message);
    void startup();

    PayableList(
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const identifier::Nym& nymID,
        const proto::ContactItemType& currency);
    PayableList() = delete;
    PayableList(const PayableList&) = delete;
    PayableList(PayableList&&) = delete;
    PayableList& operator=(const PayableList&) = delete;
    PayableList& operator=(PayableList&&) = delete;
};
}  // namespace opentxs::ui::implementation
