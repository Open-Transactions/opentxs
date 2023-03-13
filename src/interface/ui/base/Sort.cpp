// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "interface/ui/activitythread/ActivityThread.hpp"  // IWYU pragma: keep
#include "interface/ui/base/Items.hpp"
#include "interface/ui/blockchainselection/BlockchainSelection.hpp"  // IWYU pragma: keep
#include "interface/ui/contactlist/ContactList.hpp"  // IWYU pragma: keep
#include "internal/interface/ui/UI.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    AccountActivityRowID,
    AccountActivitySortKey,
    std::shared_ptr<AccountActivityRowInternal>>::
    compare_id(const AccountActivityRowID& lhs, const AccountActivityRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<AccountActivityRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    AccountActivityRowID,
    AccountActivitySortKey,
    std::shared_ptr<AccountActivityRowInternal>>::
    compare_key(
        const AccountActivitySortKey& lhs,
        const AccountActivitySortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<AccountActivitySortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    AccountCurrencyRowID,
    AccountCurrencySortKey,
    std::shared_ptr<AccountCurrencyRowInternal>>::
    compare_id(const AccountCurrencyRowID& lhs, const AccountCurrencyRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<AccountCurrencyRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    AccountCurrencyRowID,
    AccountCurrencySortKey,
    std::shared_ptr<AccountCurrencyRowInternal>>::
    compare_key(
        const AccountCurrencySortKey& lhs,
        const AccountCurrencySortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<AccountCurrencySortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    AccountListRowID,
    AccountListSortKey,
    std::shared_ptr<AccountListRowInternal>>::
    compare_id(const AccountListRowID& lhs, const AccountListRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<AccountListRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    AccountListRowID,
    AccountListSortKey,
    std::shared_ptr<AccountListRowInternal>>::
    compare_key(const AccountListSortKey& lhs, const AccountListSortKey& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<AccountListSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    AccountSummaryRowID,
    AccountSummarySortKey,
    std::shared_ptr<AccountSummaryRowInternal>>::
    compare_id(const AccountSummaryRowID& lhs, const AccountSummaryRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<AccountSummaryRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    AccountSummaryRowID,
    AccountSummarySortKey,
    std::shared_ptr<AccountSummaryRowInternal>>::
    compare_key(
        const AccountSummarySortKey& lhs,
        const AccountSummarySortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<AccountSummarySortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    AccountTreeRowID,
    AccountTreeSortKey,
    std::shared_ptr<AccountTreeRowInternal>>::
    compare_id(const AccountTreeRowID& lhs, const AccountTreeRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<AccountTreeRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    AccountTreeRowID,
    AccountTreeSortKey,
    std::shared_ptr<AccountTreeRowInternal>>::
    compare_key(const AccountTreeSortKey& lhs, const AccountTreeSortKey& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<AccountTreeSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    IssuerItemRowID,
    IssuerItemSortKey,
    std::shared_ptr<IssuerItemRowInternal>>::
    compare_id(const IssuerItemRowID& lhs, const IssuerItemRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<IssuerItemRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    IssuerItemRowID,
    IssuerItemSortKey,
    std::shared_ptr<IssuerItemRowInternal>>::
    compare_key(const IssuerItemSortKey& lhs, const IssuerItemSortKey& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<IssuerItemSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    ActivitySummaryRowID,
    ActivitySummarySortKey,
    std::shared_ptr<ActivitySummaryRowInternal>>::
    compare_id(const ActivitySummaryRowID& lhs, const ActivitySummaryRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<ActivitySummaryRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    ActivitySummaryRowID,
    ActivitySummarySortKey,
    std::shared_ptr<ActivitySummaryRowInternal>>::
    compare_key(
        const ActivitySummarySortKey& lhs,
        const ActivitySummarySortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<ActivitySummarySortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    ActivityThreadRowID,
    ActivityThreadSortKey,
    std::shared_ptr<ActivityThreadRowInternal>>::
    compare_id(const ActivityThreadRowID& lhs, const ActivityThreadRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<ActivityThreadRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    ActivityThreadRowID,
    ActivityThreadSortKey,
    std::shared_ptr<ActivityThreadRowInternal>>::
    compare_key(
        const ActivityThreadSortKey& lhs,
        const ActivityThreadSortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<ActivityThreadSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    BlockchainAccountStatusRowID,
    BlockchainAccountStatusSortKey,
    std::shared_ptr<BlockchainAccountStatusRowInternal>>::
    compare_id(
        const BlockchainAccountStatusRowID& lhs,
        const BlockchainAccountStatusRowID& rhs) const noexcept -> bool
{
    static const auto compare = std::less<BlockchainAccountStatusRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    BlockchainAccountStatusRowID,
    BlockchainAccountStatusSortKey,
    std::shared_ptr<BlockchainAccountStatusRowInternal>>::
    compare_key(
        const BlockchainAccountStatusSortKey& lhs,
        const BlockchainAccountStatusSortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<BlockchainAccountStatusSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    BlockchainSubaccountSourceRowID,
    BlockchainSubaccountSourceSortKey,
    std::shared_ptr<BlockchainSubaccountSourceRowInternal>>::
    compare_id(
        const BlockchainSubaccountSourceRowID& lhs,
        const BlockchainSubaccountSourceRowID& rhs) const noexcept -> bool
{
    static const auto compare = std::less<BlockchainSubaccountSourceRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    BlockchainSubaccountSourceRowID,
    BlockchainSubaccountSourceSortKey,
    std::shared_ptr<BlockchainSubaccountSourceRowInternal>>::
    compare_key(
        const BlockchainSubaccountSourceSortKey& lhs,
        const BlockchainSubaccountSourceSortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<BlockchainSubaccountSourceSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    BlockchainSubaccountRowID,
    BlockchainSubaccountSortKey,
    std::shared_ptr<BlockchainSubaccountRowInternal>>::
    compare_id(
        const BlockchainSubaccountRowID& lhs,
        const BlockchainSubaccountRowID& rhs) const noexcept -> bool
{
    static const auto compare = std::less<BlockchainSubaccountRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    BlockchainSubaccountRowID,
    BlockchainSubaccountSortKey,
    std::shared_ptr<BlockchainSubaccountRowInternal>>::
    compare_key(
        const BlockchainSubaccountSortKey& lhs,
        const BlockchainSubaccountSortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<BlockchainSubaccountSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    BlockchainSelectionRowID,
    BlockchainSelectionSortKey,
    std::shared_ptr<BlockchainSelectionRowInternal>>::
    compare_id(
        const BlockchainSelectionRowID& lhs,
        const BlockchainSelectionRowID& rhs) const noexcept -> bool
{
    static const auto compare = std::less<BlockchainSelectionRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    BlockchainSelectionRowID,
    BlockchainSelectionSortKey,
    std::shared_ptr<BlockchainSelectionRowInternal>>::
    compare_key(
        const BlockchainSelectionSortKey& lhs,
        const BlockchainSelectionSortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<BlockchainSelectionSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    BlockchainStatisticsRowID,
    BlockchainStatisticsSortKey,
    std::shared_ptr<BlockchainStatisticsRowInternal>>::
    compare_id(
        const BlockchainStatisticsRowID& lhs,
        const BlockchainStatisticsRowID& rhs) const noexcept -> bool
{
    static const auto compare = std::less<BlockchainStatisticsRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    BlockchainStatisticsRowID,
    BlockchainStatisticsSortKey,
    std::shared_ptr<BlockchainStatisticsRowInternal>>::
    compare_key(
        const BlockchainStatisticsSortKey& lhs,
        const BlockchainStatisticsSortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<BlockchainStatisticsSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    ContactRowID,
    ContactSortKey,
    std::shared_ptr<ContactRowInternal>>::
    compare_id(const ContactRowID& lhs, const ContactRowID& rhs) const noexcept
    -> bool
{
    static const auto compare = std::less<ContactRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    ContactRowID,
    ContactSortKey,
    std::shared_ptr<ContactRowInternal>>::
    compare_key(const ContactSortKey& lhs, const ContactSortKey& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<ContactSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    ContactSectionRowID,
    ContactSectionSortKey,
    std::shared_ptr<ContactSectionRowInternal>>::
    compare_id(const ContactSectionRowID& lhs, const ContactSectionRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<ContactSectionRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    ContactSectionRowID,
    ContactSectionSortKey,
    std::shared_ptr<ContactSectionRowInternal>>::
    compare_key(
        const ContactSectionSortKey& lhs,
        const ContactSectionSortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<ContactSectionSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    ContactSubsectionRowID,
    ContactSubsectionSortKey,
    std::shared_ptr<ContactSubsectionRowInternal>>::
    compare_id(
        const ContactSubsectionRowID& lhs,
        const ContactSubsectionRowID& rhs) const noexcept -> bool
{
    static const auto compare = std::less<ContactSubsectionRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    ContactSubsectionRowID,
    ContactSubsectionSortKey,
    std::shared_ptr<ContactSubsectionRowInternal>>::
    compare_key(
        const ContactSubsectionSortKey& lhs,
        const ContactSubsectionSortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<ContactSubsectionSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    ContactListRowID,
    ContactListSortKey,
    std::shared_ptr<ContactListRowInternal>>::
    compare_id(const ContactListRowID& lhs, const ContactListRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<ContactListRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    ContactListRowID,
    ContactListSortKey,
    std::shared_ptr<ContactListRowInternal>>::
    compare_key(const ContactListSortKey& lhs, const ContactListSortKey& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<ContactListSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    NymListRowID,
    NymListSortKey,
    std::shared_ptr<NymListRowInternal>>::
    compare_id(const NymListRowID& lhs, const NymListRowID& rhs) const noexcept
    -> bool
{
    static const auto compare = std::less<NymListRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    NymListRowID,
    NymListSortKey,
    std::shared_ptr<NymListRowInternal>>::
    compare_key(const NymListSortKey& lhs, const NymListSortKey& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<NymListSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    PayableListRowID,
    PayableListSortKey,
    std::shared_ptr<PayableListRowInternal>>::
    compare_id(const PayableListRowID& lhs, const PayableListRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<PayableListRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    PayableListRowID,
    PayableListSortKey,
    std::shared_ptr<PayableListRowInternal>>::
    compare_key(const PayableListSortKey& lhs, const PayableListSortKey& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<PayableListSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    ProfileRowID,
    ProfileSortKey,
    std::shared_ptr<ProfileRowInternal>>::
    compare_id(const ProfileRowID& lhs, const ProfileRowID& rhs) const noexcept
    -> bool
{
    static const auto compare = std::less<ProfileRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    ProfileRowID,
    ProfileSortKey,
    std::shared_ptr<ProfileRowInternal>>::
    compare_key(const ProfileSortKey& lhs, const ProfileSortKey& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<ProfileSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    ProfileSectionRowID,
    ProfileSectionSortKey,
    std::shared_ptr<ProfileSectionRowInternal>>::
    compare_id(const ProfileSectionRowID& lhs, const ProfileSectionRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<ProfileSectionRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    ProfileSectionRowID,
    ProfileSectionSortKey,
    std::shared_ptr<ProfileSectionRowInternal>>::
    compare_key(
        const ProfileSectionSortKey& lhs,
        const ProfileSectionSortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<ProfileSectionSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    ProfileSubsectionRowID,
    ProfileSubsectionSortKey,
    std::shared_ptr<ProfileSubsectionRowInternal>>::
    compare_id(
        const ProfileSubsectionRowID& lhs,
        const ProfileSubsectionRowID& rhs) const noexcept -> bool
{
    static const auto compare = std::less<ProfileSubsectionRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    ProfileSubsectionRowID,
    ProfileSubsectionSortKey,
    std::shared_ptr<ProfileSubsectionRowInternal>>::
    compare_key(
        const ProfileSubsectionSortKey& lhs,
        const ProfileSubsectionSortKey& rhs) const noexcept -> bool
{
    static const auto compare = std::less<ProfileSubsectionSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    SeedListRowID,
    SeedListSortKey,
    std::shared_ptr<SeedListRowInternal>>::
    compare_id(const SeedListRowID& lhs, const SeedListRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<SeedListRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    SeedListRowID,
    SeedListSortKey,
    std::shared_ptr<SeedListRowInternal>>::
    compare_key(const SeedListSortKey& lhs, const SeedListSortKey& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<SeedListSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    SeedTreeRowID,
    SeedTreeSortKey,
    std::shared_ptr<SeedTreeRowInternal>>::
    compare_id(const SeedTreeRowID& lhs, const SeedTreeRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<SeedTreeRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    SeedTreeRowID,
    SeedTreeSortKey,
    std::shared_ptr<SeedTreeRowInternal>>::
    compare_key(const SeedTreeSortKey& lhs, const SeedTreeSortKey& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<SeedTreeSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    SeedTreeItemRowID,
    SeedTreeItemSortKey,
    std::shared_ptr<SeedTreeItemRowInternal>>::
    compare_id(const SeedTreeItemRowID& lhs, const SeedTreeItemRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<SeedTreeItemRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    SeedTreeItemRowID,
    SeedTreeItemSortKey,
    std::shared_ptr<SeedTreeItemRowInternal>>::
    compare_key(const SeedTreeItemSortKey& lhs, const SeedTreeItemSortKey& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<SeedTreeItemSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::implementation
{
template <>
auto ListItems<
    UnitListRowID,
    UnitListSortKey,
    std::shared_ptr<UnitListRowInternal>>::
    compare_id(const UnitListRowID& lhs, const UnitListRowID& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<UnitListRowID>{};

    return compare(lhs, rhs);
}

template <>
auto ListItems<
    UnitListRowID,
    UnitListSortKey,
    std::shared_ptr<UnitListRowInternal>>::
    compare_key(const UnitListSortKey& lhs, const UnitListSortKey& rhs)
        const noexcept -> bool
{
    static const auto compare = std::less<UnitListSortKey>{};

    return compare(lhs, rhs);
}
}  // namespace opentxs::ui::implementation
