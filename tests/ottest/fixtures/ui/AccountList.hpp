// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>

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
struct OPENTXS_EXPORT AccountListRow {
    ot::UnallocatedCString account_id_{};
    ot::UnallocatedCString contract_id_{};
    ot::UnallocatedCString display_unit_{};
    ot::UnallocatedCString name_{};
    ot::UnallocatedCString notary_id_{};
    ot::UnallocatedCString notary_name_{};
    ot::AccountType type_{};
    ot::UnitType unit_{};
    int polarity_{};
    ot::Amount balance_{};
    ot::UnallocatedCString display_balance_{};
};

struct OPENTXS_EXPORT AccountListData {
    ot::UnallocatedVector<AccountListRow> rows_{};
};

OPENTXS_EXPORT auto check_account_list(
    const User& user,
    const AccountListData& expected) noexcept -> bool;
OPENTXS_EXPORT auto check_account_list_qt(
    const User& user,
    const AccountListData& expected) noexcept -> bool;
OPENTXS_EXPORT auto init_account_list(
    const User& user,
    Counter& counter) noexcept -> void;
}  // namespace ottest
