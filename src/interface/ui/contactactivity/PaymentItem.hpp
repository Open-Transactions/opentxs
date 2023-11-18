// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <tuple>

#include "interface/ui/contactactivity/ContactActivityItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

class OTPayment;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
class PaymentItem final : public ContactActivityItem
{
public:
    static auto extract(
        const api::session::Client& api,
        const identifier::Nym& nym,
        const ContactActivityRowID& row,
        CustomData& custom) noexcept
        -> std::tuple<
            opentxs::Amount,
            UnallocatedCString,
            UnallocatedCString,
            std::shared_ptr<const OTPayment>>;

    auto Amount() const noexcept -> opentxs::Amount final;
    auto Deposit() const noexcept -> bool final;
    auto DisplayAmount() const noexcept -> UnallocatedCString final;
    auto Memo() const noexcept -> UnallocatedCString final;

    PaymentItem(
        const ContactActivityInternalInterface& parent,
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const ContactActivityRowID& rowID,
        const ContactActivitySortKey& sortKey,
        CustomData& custom,
        opentxs::Amount amount,
        UnallocatedCString&& display,
        UnallocatedCString&& memo,
        std::shared_ptr<const OTPayment>&& contract) noexcept;
    PaymentItem() = delete;
    PaymentItem(const PaymentItem&) = delete;
    PaymentItem(PaymentItem&&) = delete;
    auto operator=(const PaymentItem&) -> PaymentItem& = delete;
    auto operator=(PaymentItem&&) -> PaymentItem& = delete;

    ~PaymentItem() final;

private:
    opentxs::Amount amount_;
    UnallocatedCString display_amount_;
    UnallocatedCString memo_;
    std::shared_ptr<const OTPayment> payment_;

    auto reindex(const ContactActivitySortKey& key, CustomData& custom) noexcept
        -> bool final;
};
}  // namespace opentxs::ui::implementation
