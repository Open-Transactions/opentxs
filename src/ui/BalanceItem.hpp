// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
using BalanceItemRow =
    Row<AccountActivityRowInternal,
        AccountActivityInternalInterface,
        AccountActivityRowID>;

class BalanceItem : public BalanceItemRow
{
public:
    static const proto::PaymentEvent& recover_event(const CustomData& custom);
    static const proto::PaymentWorkflow& recover_workflow(
        const CustomData& custom);

    std::vector<std::string> Contacts() const override { return contacts_; }
    std::string DisplayAmount() const override;
    std::string Text() const override;
    std::chrono::system_clock::time_point Timestamp() const override;
    StorageBox Type() const override { return type_; }

    void reindex(
        const implementation::AccountActivitySortKey& key,
        const implementation::CustomData& custom) override;

    virtual ~BalanceItem() override;

protected:
    const OTNymID nym_id_;
    const std::string workflow_{""};
    const StorageBox type_{StorageBox::UNKNOWN};
    std::string text_{""};
    std::chrono::system_clock::time_point time_;
    mutable std::shared_ptr<const UnitDefinition> contract_{nullptr};
    std::unique_ptr<std::thread> startup_{nullptr};

    static StorageBox extract_type(const proto::PaymentWorkflow& workflow);

    std::string get_contact_name(const identifier::Nym& nymID) const;

    BalanceItem(
        const AccountActivityInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const AccountActivityRowID& rowID,
        const AccountActivitySortKey& sortKey,
        const CustomData& custom,
        const identifier::Nym& nymID,
        const Identifier& accountID);

private:
    const OTIdentifier account_id_;
    const std::vector<std::string> contacts_;

    static std::vector<std::string> extract_contacts(
        const api::client::Manager& api,
        const proto::PaymentWorkflow& workflow);

    virtual opentxs::Amount effective_amount() const = 0;
    virtual bool get_contract() const = 0;

    BalanceItem(const BalanceItem&) = delete;
    BalanceItem(BalanceItem&&) = delete;
    BalanceItem& operator=(const BalanceItem&) = delete;
    BalanceItem& operator=(BalanceItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
