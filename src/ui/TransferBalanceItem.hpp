// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

namespace opentxs::ui::implementation
{
class TransferBalanceItem : public BalanceItem
{
public:
    opentxs::Amount Amount() const override { return effective_amount(); }
    std::string Memo() const override;
    std::string UUID() const override;
    std::string Workflow() const override { return workflow_; }

    void reindex(
        const implementation::AccountActivitySortKey& key,
        const implementation::CustomData& custom) override;

    ~TransferBalanceItem() = default;

private:
    friend opentxs::Factory;

    std::unique_ptr<const opentxs::Item> transfer_{nullptr};

    opentxs::Amount effective_amount() const override;
    bool get_contract() const override;

    void startup(const CustomData& custom);

    TransferBalanceItem(
        const AccountActivityInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const AccountActivityRowID& rowID,
        const AccountActivitySortKey& sortKey,
        const CustomData& custom,
        const identifier::Nym& nymID,
        const Identifier& accountID);

    TransferBalanceItem() = delete;
    TransferBalanceItem(const TransferBalanceItem&) = delete;
    TransferBalanceItem(TransferBalanceItem&&) = delete;
    TransferBalanceItem& operator=(const TransferBalanceItem&) = delete;
    TransferBalanceItem& operator=(TransferBalanceItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
