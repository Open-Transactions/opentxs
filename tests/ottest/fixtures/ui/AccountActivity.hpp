// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <optional>
#include <utility>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace ottest
{
class User;
struct Counter;
}  // namespace ottest
// NOLINTEND(modernize-concat-nested-namespaces)

namespace ot = opentxs;

namespace ottest
{
struct OPENTXS_EXPORT AccountActivityRow {
    ot::otx::client::StorageBox type_{};
    int polarity_{};
    ot::Amount amount_{};
    ot::UnallocatedCString display_amount_{};
    ot::UnallocatedVector<ot::UnallocatedCString> contacts_{};
    ot::UnallocatedCString memo_{};
    ot::UnallocatedCString workflow_{};
    ot::UnallocatedCString text_{};
    ot::UnallocatedCString uuid_{};
    std::optional<ot::Time> timestamp_{};
    int confirmations_{};
};

struct OPENTXS_EXPORT AccountActivityData {
    ot::AccountType type_;
    ot::UnallocatedCString id_{};
    ot::UnallocatedCString name_{};
    ot::UnitType unit_;
    ot::UnallocatedCString contract_id_{};
    ot::UnallocatedCString contract_name_{};
    ot::UnallocatedCString notary_id_{};
    ot::UnallocatedCString notary_name_{};
    int polarity_{};
    ot::Amount balance_{};
    ot::UnallocatedCString display_balance_{};
    ot::UnallocatedCString default_deposit_address_{};
    ot::UnallocatedMap<ot::blockchain::Type, ot::UnallocatedCString>
        deposit_addresses_{};
    ot::UnallocatedVector<ot::blockchain::Type> deposit_chains_{};
    double sync_{};
    std::pair<int, int> progress_{};
    ot::UnallocatedVector<std::pair<ot::UnallocatedCString, bool>>
        addresses_to_validate_{};
    ot::UnallocatedVector<
        std::pair<ot::UnallocatedCString, ot::UnallocatedCString>>
        amounts_to_validate_{};
    ot::UnallocatedVector<AccountActivityRow> rows_{};
};

OPENTXS_EXPORT auto check_account_activity(
    const User& user,
    const ot::identifier::Account& account,
    const AccountActivityData& expected) noexcept -> bool;
OPENTXS_EXPORT auto check_account_activity_qt(
    const User& user,
    const ot::identifier::Account& account,
    const AccountActivityData& expected) noexcept -> bool;
OPENTXS_EXPORT auto init_account_activity(
    const User& user,
    const ot::identifier::Account& account,
    Counter& counter) noexcept -> void;
}  // namespace ottest
