// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare ottest::User

#include "opentxs/opentxs.hpp"
#include "ottest/fixtures/ui/AccountActivity.hpp"
#include "ottest/fixtures/ui/AccountList.hpp"
#include "ottest/fixtures/ui/AccountTree.hpp"
#include "ottest/fixtures/ui/BlockchainAccountStatus.hpp"
#include "ottest/fixtures/ui/BlockchainSelection.hpp"
#include "ottest/fixtures/ui/ContactActivity.hpp"
#include "ottest/fixtures/ui/ContactList.hpp"
#include "ottest/fixtures/ui/NymList.hpp"
#include "ottest/fixtures/ui/SeedTree.hpp"

namespace ottest
{
auto check_account_activity_qt(
    const User&,
    const ot::identifier::Account&,
    const AccountActivityData&) noexcept -> bool
{
    return true;
}

auto check_account_list_qt(const User&, const AccountListData&) noexcept -> bool
{
    return true;
}

auto check_account_tree_qt(const User&, const AccountTreeData&) noexcept -> bool
{
    return true;
}

auto check_contact_activity_qt(
    const User&,
    const ot::identifier::Generic&,
    const ContactActivityData&) noexcept -> bool
{
    return true;
}

auto check_blockchain_account_status_qt(
    const User&,
    const ot::blockchain::Type,
    const BlockchainAccountStatusData&) noexcept -> bool
{
    return true;
}

auto check_blockchain_selection_qt(
    const ot::api::session::Client&,
    const ot::ui::Blockchains,
    const BlockchainSelectionData&) noexcept -> bool
{
    return true;
}

auto check_contact_list_qt(const User&, const ContactListData&) noexcept -> bool
{
    return true;
}

auto check_messagable_list_qt(const User&, const ContactListData&) noexcept
    -> bool
{
    return true;
}

auto check_nym_list_qt(
    const ot::api::session::Client&,
    const NymListData&) noexcept -> bool
{
    return true;
}

auto check_seed_tree_qt(
    const ot::api::session::Client&,
    const SeedTreeData&) noexcept -> bool
{
    return true;
}
}  // namespace ottest
