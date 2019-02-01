// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "ActivityThreadItem.hpp"

namespace opentxs::ui::implementation
{
class PaymentItem final : public ActivityThreadItem
{
public:
    opentxs::Amount Amount() const override;
    std::string DisplayAmount() const override;
    std::string Memo() const override;

    ~PaymentItem();

private:
    friend opentxs::Factory;

    std::string display_amount_{};
    std::string memo_{};
    opentxs::Amount amount_{0};
    std::unique_ptr<std::thread> load_{nullptr};

    void load();

    PaymentItem(
        const ActivityThreadInternalInterface& parent,
        const api::client::Manager& api,
        const network::zeromq::PublishSocket& publisher,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        const CustomData& custom);
    PaymentItem() = delete;
    PaymentItem(const PaymentItem&) = delete;
    PaymentItem(PaymentItem&&) = delete;
    PaymentItem& operator=(const PaymentItem&) = delete;
    PaymentItem& operator=(PaymentItem&&) = delete;
};
}  // namespace opentxs::ui::implementation
