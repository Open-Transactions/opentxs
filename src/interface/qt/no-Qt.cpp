// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "interface/ui/accountactivity/AccountActivity.hpp"  // IWYU pragma: associated
#include "interface/ui/accountactivity/BalanceItem.hpp"  // IWYU pragma: associated
#include "interface/ui/accountactivity/BlockchainAccountActivity.hpp"  // IWYU pragma: associated
#include "interface/ui/accountlist/AccountListItem.hpp"  // IWYU pragma: associated
#include "interface/ui/accountlist/BlockchainAccountListItem.hpp"  // IWYU pragma: associated
#include "interface/ui/accountsummary/AccountSummaryItem.hpp"  // IWYU pragma: associated
#include "interface/ui/accountsummary/IssuerItem.hpp"  // IWYU pragma: associated
#include "interface/ui/accounttree/AccountCurrency.hpp"  // IWYU pragma: associated
#include "interface/ui/accounttree/AccountTreeItem.hpp"  // IWYU pragma: associated
#include "interface/ui/activitysummary/ActivitySummaryItem.hpp"  // IWYU pragma: associated
#include "interface/ui/activitythread/ActivityThreadItem.hpp"  // IWYU pragma: associated
#include "interface/ui/blockchainaccountstatus/BlockchainAccountStatus.hpp"  // IWYU pragma: associated
#include "interface/ui/blockchainaccountstatus/BlockchainSubaccount.hpp"  // IWYU pragma: associated
#include "interface/ui/blockchainaccountstatus/BlockchainSubaccountSource.hpp"  // IWYU pragma: associated
#include "interface/ui/blockchainaccountstatus/BlockchainSubchain.hpp"  // IWYU pragma: associated
#include "interface/ui/blockchainselection/BlockchainSelectionItem.hpp"  // IWYU pragma: associated
#include "interface/ui/blockchainstatistics/BlockchainStatisticsItem.hpp"  // IWYU pragma: associated
#include "interface/ui/contactlist/ContactListItem.hpp"  // IWYU pragma: associated
#include "interface/ui/nymlist/NymListItem.hpp"  // IWYU pragma: associated
#include "interface/ui/payablelist/PayableListItem.hpp"  // IWYU pragma: associated
#include "interface/ui/seedtree/SeedTreeItem.hpp"  // IWYU pragma: associated
#include "interface/ui/seedtree/SeedTreeNym.hpp"   // IWYU pragma: associated
#include "interface/ui/unitlist/UnitListItem.hpp"  // IWYU pragma: associated

#include "interface/qt/SendMonitor.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::ui::implementation
{
auto AccountActivity::DisplayScaleQt() noexcept -> ui::DisplayScaleQt&
{
    OT_FAIL;
}

auto AccountActivity::AmountValidator() noexcept -> ui::AmountValidator&
{
    OT_FAIL;
}

auto AccountActivity::DestinationValidator() noexcept
    -> ui::DestinationValidator&
{
    OT_FAIL;
}

auto AccountActivity::init_qt() noexcept -> void {}

auto AccountActivity::shutdown_qt() noexcept -> void {}

auto AccountActivity::SendMonitor() const noexcept
    -> implementation::SendMonitor&
{
    OT_FAIL;
}

auto AccountActivity::SendMonitor() noexcept -> implementation::SendMonitor&
{
    OT_FAIL;
}

auto AccountCurrency::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto AccountListItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto AccountSummaryItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto ActivitySummaryItem::qt_data(const int, const int, QVariant&)
    const noexcept -> void
{
}

auto AccountTreeItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto ActivityThreadItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto BalanceItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto BlockchainAccountActivity::Send(
    const UnallocatedCString&,
    const UnallocatedCString&,
    const UnallocatedCString&,
    Scale,
    SendMonitor::Callback) const noexcept -> int
{
    return -1;
}

auto BlockchainSelectionItem::qt_data(const int, const int, QVariant&)
    const noexcept -> void
{
}

auto BlockchainStatisticsItem::qt_data(const int, const int, QVariant&)
    const noexcept -> void
{
}

auto BlockchainSubaccount::qt_data(const int, const int, QVariant&)
    const noexcept -> void
{
}

auto BlockchainSubaccountSource::qt_data(const int, const int, QVariant&)
    const noexcept -> void
{
}

auto BlockchainSubchain::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto ContactListItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto NymListItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto IssuerItem::qt_data(const int, const int, QVariant&) const noexcept -> void
{
}

auto PayableListItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto SeedTreeItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto SeedTreeNym::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}

auto UnitListItem::qt_data(const int, const int, QVariant&) const noexcept
    -> void
{
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::internal
{
auto List::MakeQT(const api::Session&) noexcept -> ui::qt::internal::Model*
{
    return nullptr;
}
}  // namespace opentxs::ui::internal

namespace opentxs::ui::qt::internal
{
auto Model::ChangeRow(ui::internal::Row*, ui::internal::Row*) noexcept -> void
{
}

auto Model::DeleteRow(ui::internal::Row*) noexcept -> void {}

auto Model::InsertRow(
    ui::internal::Row*,
    ui::internal::Row*,
    std::shared_ptr<ui::internal::Row>) noexcept -> void
{
}

auto Model::MoveRow(
    ui::internal::Row*,
    ui::internal::Row*,
    ui::internal::Row*) noexcept -> void
{
}

Model::~Model() = default;
}  // namespace opentxs::ui::qt::internal
