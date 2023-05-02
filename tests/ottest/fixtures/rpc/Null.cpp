// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/rpc/Helpers.hpp"  // IWYU pragma: associated

namespace ottest
{
auto check_account_activity_rpc(
    const User&,
    const ot::identifier::Account&,
    const AccountActivityData&) noexcept -> bool
{
    return true;
}
auto check_account_list_rpc(const User&, const AccountListData&) noexcept
    -> bool
{
    return true;
}
}  // namespace ottest
