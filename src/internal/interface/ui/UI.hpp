// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"
// IWYU pragma: no_include "opentxs/identity/wot/claim/ClaimType.hpp"
// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <PaymentWorkflowEnums.pb.h>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "interface/qt/SendMonitor.hpp"
#include "internal/interface/ui/AccountActivity.hpp"
#include "internal/interface/ui/AccountCurrency.hpp"
#include "internal/interface/ui/AccountList.hpp"
#include "internal/interface/ui/AccountListItem.hpp"
#include "internal/interface/ui/AccountSummary.hpp"
#include "internal/interface/ui/AccountSummaryItem.hpp"
#include "internal/interface/ui/AccountTree.hpp"
#include "internal/interface/ui/AccountTreeItem.hpp"
#include "internal/interface/ui/ActivitySummary.hpp"
#include "internal/interface/ui/ActivitySummaryItem.hpp"
#include "internal/interface/ui/ActivityThread.hpp"
#include "internal/interface/ui/ActivityThreadItem.hpp"
#include "internal/interface/ui/BalanceItem.hpp"
#include "internal/interface/ui/BlockchainAccountStatus.hpp"
#include "internal/interface/ui/BlockchainSelection.hpp"
#include "internal/interface/ui/BlockchainSelectionItem.hpp"
#include "internal/interface/ui/BlockchainStatistics.hpp"
#include "internal/interface/ui/BlockchainStatisticsItem.hpp"
#include "internal/interface/ui/BlockchainSubaccount.hpp"
#include "internal/interface/ui/BlockchainSubaccountSource.hpp"
#include "internal/interface/ui/BlockchainSubchain.hpp"
#include "internal/interface/ui/Contact.hpp"
#include "internal/interface/ui/ContactItem.hpp"
#include "internal/interface/ui/ContactList.hpp"
#include "internal/interface/ui/ContactListItem.hpp"
#include "internal/interface/ui/ContactSection.hpp"
#include "internal/interface/ui/ContactSubsection.hpp"
#include "internal/interface/ui/IssuerItem.hpp"
#include "internal/interface/ui/List.hpp"
#include "internal/interface/ui/ListRow.hpp"
#include "internal/interface/ui/MessagableList.hpp"
#include "internal/interface/ui/NymList.hpp"
#include "internal/interface/ui/NymListItem.hpp"
#include "internal/interface/ui/PayableList.hpp"
#include "internal/interface/ui/PayableListItem.hpp"
#include "internal/interface/ui/Profile.hpp"
#include "internal/interface/ui/ProfileItem.hpp"
#include "internal/interface/ui/ProfileSection.hpp"
#include "internal/interface/ui/ProfileSubsection.hpp"
#include "internal/interface/ui/SeedTree.hpp"
#include "internal/interface/ui/SeedTreeItem.hpp"
#include "internal/interface/ui/SeedTreeNym.hpp"
#include "internal/interface/ui/UnitList.hpp"
#include "internal/interface/ui/UnitListItem.hpp"
#include "internal/interface/ui/Widget.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/interface/ui/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "util/Blank.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session

class Session;
}  // namespace api

namespace contract
{
class Server;
class Unit;
}  // namespace contract

namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
namespace implementation
{
class SendMonitor;
}  // namespace implementation

namespace internal
{
namespace blank
{
struct AccountCurrency;
struct AccountListItem;
struct AccountSummaryItem;
struct AccountTreeItem;
struct ActivitySummaryItem;
struct ActivityThreadItem;
struct BalanceItem;
struct BlockchainSelectionItem;
struct BlockchainStatisticsItem;
struct BlockchainSubaccount;
struct BlockchainSubaccountSource;
struct BlockchainSubchain;
struct ContactItem;
struct ContactListItem;
struct ContactSection;
struct ContactSubsection;
struct IssuerItem;
struct NymListItem;
struct PayableListItem;
struct ProfileItem;
struct ProfileSection;
struct ProfileSubsection;
struct SeedTreeItem;
struct SeedTreeNym;
struct UnitListItem;
}  // namespace blank

struct AccountActivity;
struct AccountCurrency;
struct AccountList;
struct AccountListItem;
struct AccountSummary;
struct AccountSummaryItem;
struct AccountTree;
struct AccountTreeItem;
struct ActivitySummary;
struct ActivitySummaryItem;
struct ActivityThread;
struct ActivityThreadItem;
struct BalanceItem;
struct BlockchainAccountStatus;
struct BlockchainSelection;
struct BlockchainSelectionItem;
struct BlockchainStatistics;
struct BlockchainStatisticsItem;
struct BlockchainSubaccount;
struct BlockchainSubaccountSource;
struct BlockchainSubchain;
struct Contact;
struct ContactItem;
struct ContactList;
struct ContactListItem;
struct ContactListType;
struct ContactSection;
struct ContactSubsection;
struct IssuerItem;
struct MessagableList;
struct NymList;
struct NymListItem;
struct PayableList;
struct PayableListItem;
struct Profile;
struct ProfileItem;
struct ProfileSection;
struct ProfileSubsection;
struct SeedTree;
struct SeedTreeItem;
struct SeedTreeNym;
struct UnitList;
struct UnitListItem;
}  // namespace internal

namespace qt
{
namespace internal
{
struct Model;
}  // namespace internal

class Model;
}  // namespace qt

class AccountActivityQt;
class AccountListQt;
class AccountSummaryQt;
class AccountTreeQt;
class ActivitySummaryQt;
class ActivityThreadQt;
class AmountValidator;
class BlockchainAccountStatusQt;
class BlockchainSelectionQt;
class BlockchainStatisticsQt;
class ContactListQt;
class ContactQt;
class DestinationValidator;
class DisplayScaleQt;
class MessagableListQt;
class NymListQt;
class PayableListQt;
class ProfileQt;
class SeedTreeQt;
class UnitListQt;
}  // namespace ui

class Flag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class QObject;
class QVariant;

namespace opentxs::ui
{
auto claim_ownership(QObject* object) noexcept -> void;
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
using CustomData = UnallocatedVector<void*>;

template <typename RowID, typename SortKey>
struct ChildObjectData {
    RowID id_;
    SortKey key_;
    CustomData custom_;
    CustomData children_;

    ChildObjectData(
        const RowID& id,
        const SortKey& key,
        CustomData&& custom,
        CustomData&& children) noexcept
        : id_(id)
        , key_(key)
        , custom_(std::move(custom))
        , children_(std::move(children))
    {
    }
    ChildObjectData(
        RowID&& id,
        SortKey&& key,
        CustomData&& custom,
        CustomData&& children) noexcept
        : id_(std::move(id))
        , key_(std::move(key))
        , custom_(std::move(custom))
        , children_(std::move(children))
    {
    }
    ChildObjectData(ChildObjectData&& rhs) noexcept
        : id_(std::move(rhs.id_))
        , key_(std::move(rhs.key_))
        , custom_(std::move(rhs.custom_))
        , children_(std::move(rhs.children_))
    {
    }
    ChildObjectData() = delete;
    ChildObjectData(const ChildObjectData&) = delete;
    auto operator=(const ChildObjectData&) -> ChildObjectData& = delete;
    auto operator=(ChildObjectData&&) -> ChildObjectData& = delete;
};

// Account activity
using AccountActivityPrimaryID = identifier::Nym;
using AccountActivityExternalInterface = ui::AccountActivity;
using AccountActivityInternalInterface = ui::internal::AccountActivity;
/** WorkflowID, state */
using AccountActivityRowID =
    std::pair<identifier::Generic, proto::PaymentEventType>;
using AccountActivityRowInterface = ui::BalanceItem;
using AccountActivityRowInternal = ui::internal::BalanceItem;
using AccountActivityRowBlank = ui::internal::blank::BalanceItem;
using AccountActivitySortKey = Time;

// Account list
using AccountListPrimaryID = identifier::Nym;
using AccountListExternalInterface = ui::AccountList;
using AccountListInternalInterface = ui::internal::AccountList;
using AccountListRowID = identifier::Generic;
using AccountListRowInterface = ui::AccountListItem;
using AccountListRowInternal = ui::internal::AccountListItem;
using AccountListRowBlank = ui::internal::blank::AccountListItem;
// type, account name
using AccountListSortKey = std::pair<UnitType, UnallocatedCString>;

// Account summary
using AccountSummaryPrimaryID = identifier::Nym;
using AccountSummaryExternalInterface = ui::AccountSummary;
using AccountSummaryInternalInterface = ui::internal::AccountSummary;
using AccountSummaryRowID = identifier::Nym;
using AccountSummaryRowInterface = ui::IssuerItem;
using AccountSummaryRowInternal = ui::internal::IssuerItem;
using AccountSummaryRowBlank = ui::internal::blank::IssuerItem;
using AccountSummarySortKey = std::pair<bool, UnallocatedCString>;

using IssuerItemPrimaryID = identifier::Nym;
using IssuerItemExternalInterface = AccountSummaryRowInterface;
using IssuerItemInternalInterface = ui::internal::IssuerItem;
using IssuerItemRowID = std::pair<identifier::Generic, UnitType>;
using IssuerItemRowInterface = ui::AccountSummaryItem;
using IssuerItemRowInternal = ui::internal::AccountSummaryItem;
using IssuerItemRowBlank = ui::internal::blank::AccountSummaryItem;
using IssuerItemSortKey = UnallocatedCString;

// Account tree
using AccountTreePrimaryID = identifier::Nym;
using AccountTreeExternalInterface = ui::AccountTree;
using AccountTreeInternalInterface = ui::internal::AccountTree;
using AccountTreeRowID = UnitType;
using AccountTreeRowInterface = ui::AccountCurrency;
using AccountTreeRowInternal = ui::internal::AccountCurrency;
using AccountTreeRowBlank = ui::internal::blank::AccountCurrency;
// sort index, currency name
using AccountTreeSortKey = std::pair<int, UnallocatedCString>;

using AccountCurrencyPrimaryID = AccountTreePrimaryID;
using AccountCurrencyExternalInterface = AccountTreeRowInterface;
using AccountCurrencyInternalInterface = AccountTreeRowInternal;
using AccountCurrencyRowID = identifier::Generic;
using AccountCurrencyRowInterface = ui::AccountTreeItem;
using AccountCurrencyRowInternal = ui::internal::AccountTreeItem;
using AccountCurrencyRowBlank = ui::internal::blank::AccountTreeItem;
// sort index, account type, account name
using AccountCurrencySortKey = std::tuple<int, AccountType, UnallocatedCString>;
using AccountCurrencyRowData =
    ChildObjectData<AccountCurrencyRowID, AccountCurrencySortKey>;

// Activity summary
using ActivitySummaryPrimaryID = identifier::Nym;
using ActivitySummaryExternalInterface = ui::ActivitySummary;
using ActivitySummaryInternalInterface = ui::internal::ActivitySummary;
using ActivitySummaryRowID = identifier::Generic;
using ActivitySummaryRowInterface = ui::ActivitySummaryItem;
using ActivitySummaryRowInternal = ui::internal::ActivitySummaryItem;
using ActivitySummaryRowBlank = ui::internal::blank::ActivitySummaryItem;
using ActivitySummarySortKey = std::pair<Time, UnallocatedCString>;

// Activity thread
using ActivityThreadPrimaryID = identifier::Nym;
using ActivityThreadExternalInterface = ui::ActivityThread;
using ActivityThreadInternalInterface = ui::internal::ActivityThread;
/** item id, box, accountID, taskID */
using ActivityThreadRowID = std::
    tuple<identifier::Generic, otx::client::StorageBox, identifier::Generic>;
using ActivityThreadRowInterface = ui::ActivityThreadItem;
using ActivityThreadRowInternal = ui::internal::ActivityThreadItem;
using ActivityThreadRowBlank = ui::internal::blank::ActivityThreadItem;
/** timestamp, index */
using ActivityThreadSortKey = std::pair<Time, std::uint64_t>;

// Blockchain account status

using BlockchainAccountStatusPrimaryID = identifier::Nym;
using BlockchainAccountStatusExternalInterface = ui::BlockchainAccountStatus;
using BlockchainAccountStatusInternalInterface =
    ui::internal::BlockchainAccountStatus;
// NOTE: seed id, local payment code id, or private key id
using BlockchainAccountStatusRowID = identifier::Generic;
using BlockchainAccountStatusRowInterface = ui::BlockchainSubaccountSource;
using BlockchainAccountStatusRowInternal =
    ui::internal::BlockchainSubaccountSource;
using BlockchainAccountStatusRowBlank =
    ui::internal::blank::BlockchainSubaccountSource;
using BlockchainAccountStatusSortKey =
    std::pair<blockchain::crypto::SubaccountType, UnallocatedCString>;

using BlockchainSubaccountSourcePrimaryID = BlockchainAccountStatusPrimaryID;
using BlockchainSubaccountSourceExternalInterface =
    ui::BlockchainSubaccountSource;
using BlockchainSubaccountSourceInternalInterface =
    ui::internal::BlockchainSubaccountSource;
// NOTE: subaccount id
using BlockchainSubaccountSourceRowID = identifier::Generic;
using BlockchainSubaccountSourceRowInterface = ui::BlockchainSubaccount;
using BlockchainSubaccountSourceRowInternal =
    ui::internal::BlockchainSubaccount;
using BlockchainSubaccountSourceRowBlank =
    ui::internal::blank::BlockchainSubaccount;
using BlockchainSubaccountSourceSortKey = UnallocatedCString;
using BlockchainSubaccountSourceRowData = ChildObjectData<
    BlockchainSubaccountSourceRowID,
    BlockchainSubaccountSourceSortKey>;

using BlockchainSubaccountPrimaryID = BlockchainSubaccountSourcePrimaryID;
using BlockchainSubaccountExternalInterface = ui::BlockchainSubaccount;
using BlockchainSubaccountInternalInterface =
    ui::internal::BlockchainSubaccount;
using BlockchainSubaccountRowID = blockchain::crypto::Subchain;
using BlockchainSubaccountRowInterface = ui::BlockchainSubchain;
using BlockchainSubaccountRowInternal = ui::internal::BlockchainSubchain;
using BlockchainSubaccountRowBlank = ui::internal::blank::BlockchainSubchain;
using BlockchainSubaccountSortKey = UnallocatedCString;
using BlockchainSubaccountRowData =
    ChildObjectData<BlockchainSubaccountRowID, BlockchainSubaccountSortKey>;

// Blockchain selection
using BlockchainSelectionPrimaryID = identifier::Generic;
using BlockchainSelectionExternalInterface = ui::BlockchainSelection;
using BlockchainSelectionInternalInterface = ui::internal::BlockchainSelection;
using BlockchainSelectionRowID = blockchain::Type;
using BlockchainSelectionRowInterface = ui::BlockchainSelectionItem;
using BlockchainSelectionRowInternal = ui::internal::BlockchainSelectionItem;
using BlockchainSelectionRowBlank =
    ui::internal::blank::BlockchainSelectionItem;
using BlockchainSelectionSortKey = std::pair<UnallocatedCString, bool>;

// Blockchain statistics
using BlockchainStatisticsPrimaryID = identifier::Generic;
using BlockchainStatisticsExternalInterface = ui::BlockchainStatistics;
using BlockchainStatisticsInternalInterface =
    ui::internal::BlockchainStatistics;
using BlockchainStatisticsRowID = blockchain::Type;
using BlockchainStatisticsRowInterface = ui::BlockchainStatisticsItem;
using BlockchainStatisticsRowInternal = ui::internal::BlockchainStatisticsItem;
using BlockchainStatisticsRowBlank =
    ui::internal::blank::BlockchainStatisticsItem;
using BlockchainStatisticsSortKey = UnallocatedCString;

// Contact
using ContactPrimaryID = identifier::Generic;
using ContactExternalInterface = ui::Contact;
using ContactInternalInterface = ui::internal::Contact;
using ContactRowID = identity::wot::claim::SectionType;
using ContactRowInterface = ui::ContactSection;
using ContactRowInternal = ui::internal::ContactSection;
using ContactRowBlank = ui::internal::blank::ContactSection;
using ContactSortKey = int;

using ContactSectionPrimaryID = ContactPrimaryID;
using ContactSectionExternalInterface = ContactRowInterface;
using ContactSectionInternalInterface = ui::internal::ContactSection;
using ContactSectionRowID = std::
    pair<identity::wot::claim::SectionType, identity::wot::claim::ClaimType>;
using ContactSectionRowInterface = ui::ContactSubsection;
using ContactSectionRowInternal = ui::internal::ContactSubsection;
using ContactSectionRowBlank = ui::internal::blank::ContactSubsection;
using ContactSectionSortKey = int;

using ContactSubsectionPrimaryID = ContactSectionPrimaryID;
using ContactSubsectionExternalInterface = ContactSectionRowInterface;
using ContactSubsectionInternalInterface = ui::internal::ContactSubsection;
using ContactSubsectionRowID = identifier::Generic;
using ContactSubsectionRowInterface = ui::ContactItem;
using ContactSubsectionRowInternal = ui::internal::ContactItem;
using ContactSubsectionRowBlank = ui::internal::blank::ContactItem;
using ContactSubsectionSortKey = int;

// Contact list
using ContactListPrimaryID = identifier::Nym;
using ContactListExternalInterface = ui::ContactList;
using ContactListInternalInterface = ui::internal::ContactListType;
using ContactListRowID = identifier::Generic;
using ContactListRowInterface = ui::ContactListItem;
using ContactListRowInternal = ui::internal::ContactListItem;
using ContactListRowBlank = ui::internal::blank::ContactListItem;
// Items with the first value set to true will be sorted first
using ContactListSortKey = std::pair<bool, UnallocatedCString>;

// Messagable list
using MessagableListPrimaryID = identifier::Nym;
using MessagableExternalInterface = ui::MessagableList;
using MessagableInternalInterface = ui::internal::MessagableList;
using MessagableListRowID = ContactListRowID;
using MessagableListRowInterface = ContactListRowInterface;
using MessagableListRowInternal = ContactListRowInternal;
using MessagableListRowBlank = ContactListRowBlank;
using MessagableListSortKey = ContactListSortKey;

// Nym list
using NymListPrimaryID = identifier::Generic;
using NymListExternalInterface = ui::NymList;
using NymListInternalInterface = ui::internal::NymList;
using NymListRowID = identifier::Nym;
using NymListRowInterface = ui::NymListItem;
using NymListRowInternal = ui::internal::NymListItem;
using NymListRowBlank = ui::internal::blank::NymListItem;
using NymListSortKey = UnallocatedCString;

// Payable list
using PayablePrimaryID = identifier::Nym;
using PayableExternalInterface = ui::PayableList;
using PayableInternalInterface = ui::internal::PayableList;
using PayableListRowID = ContactListRowID;
using PayableListRowInterface = ui::PayableListItem;
using PayableListRowInternal = ui::internal::PayableListItem;
using PayableListRowBlank = ui::internal::blank::PayableListItem;
using PayableListSortKey = ContactListSortKey;

// Profile
using ProfilePrimaryID = identifier::Nym;
using ProfileExternalInterface = ui::Profile;
using ProfileInternalInterface = ui::internal::Profile;
using ProfileRowID = identity::wot::claim::SectionType;
using ProfileRowInterface = ui::ProfileSection;
using ProfileRowInternal = ui::internal::ProfileSection;
using ProfileRowBlank = ui::internal::blank::ProfileSection;
using ProfileSortKey = int;

using ProfileSectionPrimaryID = ProfilePrimaryID;
using ProfileSectionExternalInterface = ProfileRowInterface;
using ProfileSectionInternalInterface = ui::internal::ProfileSection;
using ProfileSectionRowID = std::
    pair<identity::wot::claim::SectionType, identity::wot::claim::ClaimType>;
using ProfileSectionRowInterface = ui::ProfileSubsection;
using ProfileSectionRowInternal = ui::internal::ProfileSubsection;
using ProfileSectionRowBlank = ui::internal::blank::ProfileSubsection;
using ProfileSectionSortKey = int;

using ProfileSubsectionPrimaryID = ProfileSectionPrimaryID;
using ProfileSubsectionExternalInterface = ProfileSectionRowInterface;
using ProfileSubsectionInternalInterface = ui::internal::ProfileSubsection;
using ProfileSubsectionRowID = identifier::Generic;
using ProfileSubsectionRowInterface = ui::ProfileItem;
using ProfileSubsectionRowInternal = ui::internal::ProfileItem;
using ProfileSubsectionRowBlank = ui::internal::blank::ProfileItem;
using ProfileSubsectionSortKey = int;

// Seed tree
using SeedTreePrimaryID = identifier::Generic;
using SeedTreeExternalInterface = ui::SeedTree;
using SeedTreeInternalInterface = ui::internal::SeedTree;
// seed id
using SeedTreeRowID = identifier::Generic;
using SeedTreeRowInterface = ui::SeedTreeItem;
using SeedTreeRowInternal = ui::internal::SeedTreeItem;
using SeedTreeRowBlank = ui::internal::blank::SeedTreeItem;
// primary, seed name
using SeedTreeSortKey = std::pair<bool, UnallocatedCString>;

using SeedTreeItemPrimaryID = SeedTreePrimaryID;
using SeedTreeItemExternalInterface = SeedTreeRowInterface;
using SeedTreeItemInternalInterface = SeedTreeRowInternal;
using SeedTreeItemRowID = identifier::Nym;
using SeedTreeItemRowInterface = ui::SeedTreeNym;
using SeedTreeItemRowInternal = ui::internal::SeedTreeNym;
using SeedTreeItemRowBlank = ui::internal::blank::SeedTreeNym;
/// nym index
using SeedTreeItemSortKey = std::size_t;
using SeedTreeItemRowData =
    ChildObjectData<SeedTreeItemRowID, SeedTreeItemSortKey>;

// Unit list
using UnitListPrimaryID = identifier::Nym;
using UnitListExternalInterface = ui::UnitList;
using UnitListInternalInterface = ui::internal::UnitList;
using UnitListRowID = UnitType;
using UnitListRowInterface = ui::UnitListItem;
using UnitListRowInternal = ui::internal::UnitListItem;
using UnitListRowBlank = ui::internal::blank::UnitListItem;
using UnitListSortKey = UnallocatedCString;

auto account_name_blockchain(blockchain::Type) noexcept -> UnallocatedCString;
auto account_name_custodial(
    const api::Session& api,
    const identifier::Notary& notary,
    const identifier::UnitDefinition& unit,
    UnallocatedCString&& alias) noexcept -> UnallocatedCString;
}  // namespace opentxs::ui::implementation

namespace opentxs::ui::internal
{
struct List : virtual public ui::List {
    static auto MakeQT(const api::Session& api) noexcept
        -> ui::qt::internal::Model*;

    virtual auto API() const noexcept(false) -> const api::Session& = 0;
    virtual auto GetQt() const noexcept -> ui::qt::internal::Model* = 0;

    ~List() override = default;
};

struct Row : virtual public ui::ListRow {
    static auto next_index() noexcept -> std::ptrdiff_t;

    virtual auto index() const noexcept -> std::ptrdiff_t = 0;
    virtual auto qt_data(
        [[maybe_unused]] const int column,
        [[maybe_unused]] const int role,
        [[maybe_unused]] QVariant& out) const noexcept -> void
    {
    }

    virtual auto AddChildren(implementation::CustomData&& data) noexcept
        -> void = 0;
    // NOTE: lock belongs to the associated qt::internal::Model object
    virtual auto InitAfterAdd(const Lock& lock) noexcept -> void {}

    ~Row() override = default;
};
struct AccountActivity : virtual public List,
                         virtual public ui::AccountActivity {
    struct Callbacks {
        using SyncCallback = std::function<void(int, int, double)>;
        using BalanceCallback = std::function<void(UnallocatedCString)>;
        using PolarityCallback = std::function<void(int)>;

        SyncCallback sync_{};
        BalanceCallback balance_{};
        PolarityCallback polarity_{};
    };

    virtual auto last(const implementation::AccountActivityRowID& id)
        const noexcept -> bool = 0;
    // WARNING potential race condition. Child rows must never call this
    // except when directed by parent object
    virtual auto Contract() const noexcept -> const contract::Unit& = 0;
    // WARNING potential race condition. Child rows must never call this
    // except when directed by parent object
    virtual auto Notary() const noexcept -> const contract::Server& = 0;
    using ui::AccountActivity::Send;
    virtual auto Send(
        const UnallocatedCString& address,
        const UnallocatedCString& amount,
        const UnallocatedCString& memo,
        Scale scale,
        implementation::SendMonitor::Callback cb) const noexcept -> int = 0;
    virtual auto Send(
        const identifier::Generic& contact,
        const UnallocatedCString& amount,
        const UnallocatedCString& memo,
        Scale scale,
        implementation::SendMonitor::Callback cb) const noexcept -> int = 0;
    virtual auto SendMonitor() const noexcept
        -> implementation::SendMonitor& = 0;

    virtual auto AmountValidator() noexcept -> ui::AmountValidator& = 0;
    virtual auto DestinationValidator() noexcept
        -> ui::DestinationValidator& = 0;
    virtual auto DisplayScaleQt() noexcept -> ui::DisplayScaleQt& = 0;
    virtual auto SendMonitor() noexcept -> implementation::SendMonitor& = 0;
    virtual auto SetCallbacks(Callbacks&&) noexcept -> void = 0;

    ~AccountActivity() override = default;
};
struct AccountCurrency : virtual public List,
                         virtual public Row,
                         virtual public ui::AccountCurrency {
    virtual auto last(const implementation::AccountCurrencyRowID& id)
        const noexcept -> bool = 0;

    virtual auto reindex(
        const implementation::AccountTreeSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~AccountCurrency() override = default;
};
struct AccountList : virtual public List, virtual public ui::AccountList {
    virtual auto last(const implementation::AccountListRowID& id) const noexcept
        -> bool = 0;

    ~AccountList() override = default;
};
struct AccountListItem : virtual public Row,
                         virtual public ui::AccountListItem {
    virtual auto reindex(
        const implementation::AccountListSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~AccountListItem() override = default;
};
struct AccountSummary : virtual public List, virtual public ui::AccountSummary {
    virtual auto Currency() const -> UnitType = 0;
    virtual auto last(const implementation::AccountSummaryRowID& id)
        const noexcept -> bool = 0;
    virtual auto NymID() const -> const identifier::Nym& = 0;

    ~AccountSummary() override = default;
};
struct AccountSummaryItem : virtual public Row,
                            virtual public ui::AccountSummaryItem {
    virtual auto reindex(
        const implementation::IssuerItemSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~AccountSummaryItem() override = default;
};
struct AccountTree : virtual public List, virtual public ui::AccountTree {
    virtual auto last(const implementation::AccountTreeRowID& id) const noexcept
        -> bool = 0;

    ~AccountTree() override = default;
};
struct AccountTreeItem : virtual public Row,
                         virtual public ui::AccountTreeItem {
    virtual auto reindex(
        const implementation::AccountCurrencySortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~AccountTreeItem() override = default;
};
struct ActivitySummary : virtual public List,
                         virtual public ui::ActivitySummary {
    virtual auto last(const implementation::ActivitySummaryRowID& id)
        const noexcept -> bool = 0;

    ~ActivitySummary() override = default;
};
struct ActivitySummaryItem : virtual public Row,
                             virtual public ui::ActivitySummaryItem {
    virtual auto reindex(
        const implementation::ActivitySummarySortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~ActivitySummaryItem() override = default;
};
struct ActivityThread : virtual public List, virtual public ui::ActivityThread {
    struct Callbacks {
        using MCallback = std::function<void(bool)>;

        SimpleCallback general_{};
        SimpleCallback display_name_{};
        SimpleCallback draft_{};
        MCallback messagability_{};
    };

    virtual auto last(const implementation::ActivityThreadRowID& id)
        const noexcept -> bool = 0;

    virtual auto SetCallbacks(Callbacks&&) noexcept -> void = 0;

    ~ActivityThread() override = default;
};
struct ActivityThreadItem : virtual public Row,
                            virtual public ui::ActivityThreadItem {
    virtual auto reindex(
        const implementation::ActivityThreadSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~ActivityThreadItem() override = default;
};
struct BalanceItem : virtual public Row, virtual public ui::BalanceItem {
    virtual auto reindex(
        const implementation::AccountActivitySortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~BalanceItem() override = default;
};
struct BlockchainAccountStatus : virtual public List,
                                 virtual public ui::BlockchainAccountStatus {
    virtual auto last(const implementation::BlockchainAccountStatusRowID& id)
        const noexcept -> bool = 0;

    ~BlockchainAccountStatus() override = default;
};
struct BlockchainSelection : virtual public List,
                             virtual public ui::BlockchainSelection {
    using EnabledCallback =
        std::function<void(blockchain::Type, bool, std::size_t)>;

    virtual auto EnabledCount() const noexcept -> std::size_t = 0;
    virtual auto last(const implementation::BlockchainSelectionRowID& id)
        const noexcept -> bool = 0;

    virtual auto Set(EnabledCallback&& cb) const noexcept -> void = 0;

    ~BlockchainSelection() override = default;
};
struct BlockchainSelectionItem : virtual public Row,
                                 virtual public ui::BlockchainSelectionItem {
    virtual auto reindex(
        const implementation::BlockchainSelectionSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~BlockchainSelectionItem() override = default;
};
struct BlockchainStatistics : virtual public List,
                              virtual public ui::BlockchainStatistics {
    virtual auto last(const implementation::BlockchainStatisticsRowID& id)
        const noexcept -> bool = 0;

    ~BlockchainStatistics() override = default;
};
struct BlockchainStatisticsItem : virtual public Row,
                                  virtual public ui::BlockchainStatisticsItem {
    virtual auto reindex(
        const implementation::BlockchainStatisticsSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~BlockchainStatisticsItem() override = default;
};
struct BlockchainSubaccount : virtual public List,
                              virtual public Row,
                              virtual public ui::BlockchainSubaccount {
    virtual auto last(const implementation::BlockchainSubaccountRowID& id)
        const noexcept -> bool = 0;
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;
    virtual auto reindex(
        const implementation::BlockchainSubaccountSourceSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~BlockchainSubaccount() override = default;
};
struct BlockchainSubaccountSource
    : virtual public List,
      virtual public Row,
      virtual public ui::BlockchainSubaccountSource {
    virtual auto last(const implementation::BlockchainSubaccountSourceRowID& id)
        const noexcept -> bool = 0;
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;

    virtual auto reindex(
        const implementation::BlockchainAccountStatusSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~BlockchainSubaccountSource() override = default;
};
struct BlockchainSubchain : virtual public Row,
                            virtual public ui::BlockchainSubchain {
    virtual auto reindex(
        const implementation::BlockchainSubaccountSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~BlockchainSubchain() override = default;
};
struct Contact : virtual public List, virtual public ui::Contact {
    struct Callbacks {
        using Callback = std::function<void(UnallocatedCString)>;

        Callback name_{};
        Callback payment_code_{};
    };

    virtual auto last(const implementation::ContactRowID& id) const noexcept
        -> bool = 0;

    virtual auto SetCallbacks(Callbacks&&) noexcept -> void = 0;

    ~Contact() override = default;
};
struct ContactItem : virtual public Row, virtual public ui::ContactItem {
    virtual auto reindex(
        const implementation::ContactSubsectionSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~ContactItem() override = default;
};
struct ContactListType : virtual public List {
    virtual auto ID() const noexcept -> const identifier::Generic& = 0;
    virtual auto last(const implementation::ContactListRowID& id) const noexcept
        -> bool = 0;

    ~ContactListType() override = default;
};
struct ContactList : virtual public ContactListType,
                     virtual public ui::ContactList {
    ~ContactList() override = default;
};
struct ContactListItem : virtual public Row,
                         virtual public ui::ContactListItem {
    virtual auto reindex(
        const implementation::ContactListSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~ContactListItem() override = default;
};
struct ContactSection : virtual public List,
                        virtual public Row,
                        virtual public ui::ContactSection {
    virtual auto ContactID() const noexcept -> UnallocatedCString = 0;
    virtual auto last(const implementation::ContactSectionRowID& id)
        const noexcept -> bool = 0;

    virtual auto reindex(
        const implementation::ContactSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~ContactSection() override = default;
};
struct ContactSubsection : virtual public List,
                           virtual public Row,
                           virtual public ui::ContactSubsection {
    virtual auto reindex(
        const implementation::ContactSectionSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;
    // List
    virtual auto last(const implementation::ContactSubsectionRowID& id)
        const noexcept -> bool = 0;

    ~ContactSubsection() override = default;
};
struct IssuerItem : virtual public List,
                    virtual public Row,
                    virtual public ui::IssuerItem {
    // Row
    virtual auto reindex(
        const implementation::AccountSummarySortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;
    // List
    virtual auto last(const implementation::IssuerItemRowID& id) const noexcept
        -> bool = 0;

    ~IssuerItem() override = default;
};
struct MessagableList : virtual public ContactListType,
                        virtual public ui::MessagableList {
    ~MessagableList() override = default;
};
struct NymList : virtual public List, virtual public ui::NymList {
    virtual auto last(const implementation::NymListRowID& id) const noexcept
        -> bool = 0;

    ~NymList() override = default;
};
struct NymListItem : virtual public Row, virtual public ui::NymListItem {
    virtual auto reindex(
        const implementation::NymListSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~NymListItem() override = default;
};
struct PayableList : virtual public ContactListType,
                     virtual public ui::PayableList {
    ~PayableList() override = default;
};
struct PayableListItem : virtual public Row,
                         virtual public ui::PayableListItem {
    virtual auto reindex(
        const implementation::PayableListSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~PayableListItem() override = default;
};
struct Profile : virtual public List, virtual public ui::Profile {
    struct Callbacks {
        using Callback = std::function<void(UnallocatedCString)>;

        Callback name_{};
        Callback payment_code_{};
    };

    virtual auto last(const implementation::ProfileRowID& id) const noexcept
        -> bool = 0;
    virtual auto NymID() const -> const identifier::Nym& = 0;

    virtual auto SetCallbacks(Callbacks&&) noexcept -> void = 0;

    ~Profile() override = default;
};
struct ProfileSection : virtual public List,
                        virtual public Row,
                        virtual public ui::ProfileSection {
    virtual auto last(const implementation::ProfileSectionRowID& id)
        const noexcept -> bool = 0;
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;

    virtual auto reindex(
        const implementation::ProfileSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~ProfileSection() override = default;
};
struct ProfileSubsection : virtual public List,
                           virtual public Row,
                           virtual public ui::ProfileSubsection {
    // Row
    virtual auto reindex(
        const implementation::ProfileSectionSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;
    // List
    virtual auto last(const implementation::ProfileSubsectionRowID& id)
        const noexcept -> bool = 0;
    // custom
    virtual auto NymID() const noexcept -> const identifier::Nym& = 0;
    virtual auto Section() const noexcept
        -> identity::wot::claim::SectionType = 0;

    ~ProfileSubsection() override = default;
};
struct ProfileItem : virtual public Row, virtual public ui::ProfileItem {
    virtual auto reindex(
        const implementation::ProfileSubsectionSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~ProfileItem() override = default;
};
struct SeedTree : virtual public List, virtual public ui::SeedTree {
    struct Callbacks {
        using NymCB = std::function<void(const identifier::Nym&)>;
        using SeedCB = std::function<void(const identifier::Generic&)>;

        NymCB nym_changed_{};
        SeedCB seed_changed_{};
    };

    virtual auto last(const implementation::SeedTreeRowID& id) const noexcept
        -> bool = 0;

    virtual auto SetCallbacks(Callbacks&&) noexcept -> void = 0;

    ~SeedTree() override = default;
};
struct SeedTreeItem : virtual public List,
                      virtual public Row,
                      virtual public ui::SeedTreeItem {
    virtual auto last(
        const implementation::SeedTreeItemRowID& id) const noexcept -> bool = 0;

    virtual auto reindex(
        const implementation::SeedTreeSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~SeedTreeItem() override = default;
};
struct SeedTreeNym : virtual public Row, virtual public ui::SeedTreeNym {
    virtual auto reindex(
        const implementation::SeedTreeItemSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~SeedTreeNym() override = default;
};
struct UnitList : virtual public List, virtual public ui::UnitList {
    virtual auto last(const implementation::UnitListRowID& id) const noexcept
        -> bool = 0;

    ~UnitList() override = default;
};
struct UnitListItem : virtual public Row, virtual public ui::UnitListItem {
    virtual auto reindex(
        const implementation::UnitListSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~UnitListItem() override = default;
};

namespace blank
{
struct Widget : virtual public ui::Widget {
    auto ClearCallbacks() const noexcept -> void final {}
    auto SetCallback(SimpleCallback) const noexcept -> void final {}
    auto WidgetID() const noexcept -> identifier::Generic override
    {
        return {};
    }
};
struct Row : virtual public ui::internal::Row, public Widget {
    auto index() const noexcept -> std::ptrdiff_t final { return -1; }
    auto Last() const noexcept -> bool final { return true; }
    auto Valid() const noexcept -> bool final { return false; }

    auto AddChildren(implementation::CustomData&& data) noexcept -> void final
    {
    }
};
template <typename ListType, typename RowType, typename RowIDType>
struct List : virtual public ListType, public Row {
    auto API() const noexcept(false) -> const api::Session& final
    {
        throw std::out_of_range{"blank model"};
    }
    auto First() const noexcept -> RowType final { return RowType{nullptr}; }
    auto last(const RowIDType&) const noexcept -> bool final { return false; }
    auto Next() const noexcept -> RowType final { return RowType{nullptr}; }
    virtual auto WaitForStartup() const noexcept -> void final { return; }
    auto WidgetID() const noexcept -> identifier::Generic final
    {
        return blank::Widget::WidgetID();
    }

    auto GetQt() const noexcept -> ui::qt::internal::Model* final
    {
        return nullptr;
    }
};
struct AccountCurrency final : public List<
                                   internal::AccountCurrency,
                                   SharedPimpl<ui::AccountTreeItem>,
                                   implementation::AccountCurrencyRowID> {
    auto Currency() const noexcept -> UnitType final { return {}; }
    auto Debug() const noexcept -> UnallocatedCString final { return {}; }
    auto Name() const noexcept -> UnallocatedCString final { return {}; }

    auto reindex(
        const implementation::AccountTreeSortKey& key,
        implementation::CustomData& custom) noexcept -> bool final
    {
        return {};
    }

    ~AccountCurrency() final = default;
};
struct AccountListItem final : virtual public Row,
                               virtual public internal::AccountListItem {
    auto AccountID() const noexcept -> UnallocatedCString final { return {}; }
    auto Balance() const noexcept -> Amount final { return {}; }
    auto ContractID() const noexcept -> UnallocatedCString final { return {}; }
    auto DisplayBalance() const noexcept -> UnallocatedCString final
    {
        return {};
    }
    auto DisplayUnit() const noexcept -> UnallocatedCString final { return {}; }
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto NotaryID() const noexcept -> UnallocatedCString final { return {}; }
    auto NotaryName() const noexcept -> UnallocatedCString final { return {}; }
    auto Type() const noexcept -> AccountType final { return {}; }
    auto Unit() const noexcept -> UnitType final { return {}; }

    auto reindex(
        const implementation::AccountListSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct AccountSummaryItem final : public Row,
                                  public internal::AccountSummaryItem {
    auto AccountID() const noexcept -> UnallocatedCString final { return {}; }
    auto Balance() const noexcept -> Amount final { return {}; }
    auto DisplayBalance() const noexcept -> UnallocatedCString final
    {
        return {};
    }
    auto Name() const noexcept -> UnallocatedCString final { return {}; }

    auto reindex(
        const implementation::IssuerItemSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct AccountTreeItem final : public Row, public internal::AccountTreeItem {
    auto AccountID() const noexcept -> UnallocatedCString final { return {}; }
    auto Balance() const noexcept -> Amount final { return {}; }
    auto ContractID() const noexcept -> UnallocatedCString final { return {}; }
    auto DisplayBalance() const noexcept -> UnallocatedCString final
    {
        return {};
    }
    auto DisplayUnit() const noexcept -> UnallocatedCString final { return {}; }
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto NotaryID() const noexcept -> UnallocatedCString final { return {}; }
    auto NotaryName() const noexcept -> UnallocatedCString final { return {}; }
    auto Type() const noexcept -> AccountType final { return {}; }
    auto Unit() const noexcept -> UnitType final { return {}; }

    auto reindex(
        const implementation::AccountCurrencySortKey& key,
        implementation::CustomData& custom) noexcept -> bool final
    {
        return {};
    }

    ~AccountTreeItem() final = default;
};
struct ActivitySummaryItem final
    : virtual public Row,
      virtual public internal::ActivitySummaryItem {
    auto DisplayName() const noexcept -> UnallocatedCString final { return {}; }
    auto ImageURI() const noexcept -> UnallocatedCString final { return {}; }
    auto Text() const noexcept -> UnallocatedCString final { return {}; }
    auto ThreadID() const noexcept -> UnallocatedCString final { return {}; }
    auto Timestamp() const noexcept -> Time final { return {}; }
    auto Type() const noexcept -> otx::client::StorageBox final
    {
        return otx::client::StorageBox::UNKNOWN;
    }

    auto reindex(
        const implementation::ActivitySummarySortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct ActivityThreadItem final : public Row,
                                  public internal::ActivityThreadItem {
    auto Amount() const noexcept -> opentxs::Amount final { return 0; }
    auto Deposit() const noexcept -> bool final { return false; }
    auto DisplayAmount() const noexcept -> UnallocatedCString final
    {
        return {};
    }
    auto From() const noexcept -> UnallocatedCString final { return {}; }
    auto Loading() const noexcept -> bool final { return false; }
    auto MarkRead() const noexcept -> bool final { return false; }
    auto Memo() const noexcept -> UnallocatedCString final { return {}; }
    auto Outgoing() const noexcept -> bool final { return false; }
    auto Pending() const noexcept -> bool final { return false; }
    auto Text() const noexcept -> UnallocatedCString final { return {}; }
    auto Timestamp() const noexcept -> Time final { return {}; }
    auto Type() const noexcept -> otx::client::StorageBox final
    {
        return otx::client::StorageBox::UNKNOWN;
    }

    auto reindex(
        const implementation::ActivityThreadSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct BalanceItem final : public Row, public internal::BalanceItem {
    auto Amount() const noexcept -> opentxs::Amount final { return {}; }
    auto Confirmations() const noexcept -> int override { return -1; }
    auto Contacts() const noexcept
        -> UnallocatedVector<UnallocatedCString> final
    {
        return {};
    }
    auto DisplayAmount() const noexcept -> UnallocatedCString final
    {
        return {};
    }
    auto Memo() const noexcept -> UnallocatedCString final { return {}; }
    auto Workflow() const noexcept -> UnallocatedCString final { return {}; }
    auto Text() const noexcept -> UnallocatedCString final { return {}; }
    auto Timestamp() const noexcept -> Time final { return {}; }
    auto Type() const noexcept -> otx::client::StorageBox final
    {
        return otx::client::StorageBox::UNKNOWN;
    }
    auto UUID() const noexcept -> UnallocatedCString final { return {}; }

    auto reindex(
        const implementation::AccountActivitySortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct BlockchainSelectionItem final
    : virtual public Row,
      virtual public internal::BlockchainSelectionItem {

    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto IsEnabled() const noexcept -> bool final { return {}; }
    auto IsTestnet() const noexcept -> bool final { return {}; }
    auto Type() const noexcept -> blockchain::Type final { return {}; }

    auto reindex(
        const implementation::BlockchainSelectionSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct BlockchainStatisticsItem final
    : virtual public Row,
      virtual public internal::BlockchainStatisticsItem {

    auto ActivePeers() const noexcept -> std::size_t final { return {}; }
    auto Balance() const noexcept -> UnallocatedCString final { return {}; }
    auto BlockDownloadQueue() const noexcept -> std::size_t final { return {}; }
    auto Chain() const noexcept -> blockchain::Type final { return {}; }
    auto ConnectedPeers() const noexcept -> std::size_t final { return {}; }
    auto Filters() const noexcept -> Position final { return {}; }
    auto Headers() const noexcept -> Position final { return {}; }
    auto Name() const noexcept -> UnallocatedCString final { return {}; }

    auto reindex(
        const implementation::BlockchainStatisticsSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct BlockchainSubaccount final
    : public List<
          internal::BlockchainSubaccount,
          SharedPimpl<ui::BlockchainSubchain>,
          implementation::BlockchainSubaccountRowID> {
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto NymID() const noexcept -> const identifier::Nym& final;
    auto SubaccountID() const noexcept -> const identifier::Generic& final;

    auto reindex(
        const implementation::BlockchainSubaccountSourceSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct BlockchainSubaccountSource final
    : public List<
          internal::BlockchainSubaccountSource,
          SharedPimpl<ui::BlockchainSubaccount>,
          implementation::BlockchainSubaccountSourceRowID> {
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto NymID() const noexcept -> const identifier::Nym& final;
    auto SourceID() const noexcept -> const identifier::Generic& final;
    auto Type() const noexcept -> blockchain::crypto::SubaccountType final;

    auto reindex(
        const implementation::BlockchainAccountStatusSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct BlockchainSubchain final : public Row,
                                  public internal::BlockchainSubchain {
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto Progress() const noexcept -> UnallocatedCString final { return {}; }
    auto Type() const noexcept -> blockchain::crypto::Subchain final;

    auto reindex(
        const implementation::BlockchainSubaccountSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct ContactItem final : public Row, public internal::ContactItem {
    auto ClaimID() const noexcept -> UnallocatedCString final { return {}; }
    auto IsActive() const noexcept -> bool final { return false; }
    auto IsPrimary() const noexcept -> bool final { return false; }
    auto Value() const noexcept -> UnallocatedCString final { return {}; }

    auto reindex(
        const implementation::ContactSubsectionSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct ContactListItem : virtual public Row,
                         virtual public internal::ContactListItem {
    auto ContactID() const noexcept -> UnallocatedCString final { return {}; }
    auto DisplayName() const noexcept -> UnallocatedCString final { return {}; }
    auto ImageURI() const noexcept -> UnallocatedCString final { return {}; }
    auto Section() const noexcept -> UnallocatedCString final { return {}; }

    auto reindex(
        const implementation::ContactListSortKey&,
        implementation::CustomData&) noexcept -> bool override
    {
        return false;
    }
};
struct ContactSection final : public List<
                                  internal::ContactSection,
                                  OTUIContactSubsection,
                                  implementation::ContactSectionRowID> {
    auto ContactID() const noexcept -> UnallocatedCString final { return {}; }
    auto Name(const UnallocatedCString& lang) const noexcept
        -> UnallocatedCString final
    {
        return {};
    }
    auto Type() const noexcept -> identity::wot::claim::SectionType final
    {
        return {};
    }

    auto reindex(
        const implementation::ContactSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct ContactSubsection final : public List<
                                     internal::ContactSubsection,
                                     OTUIContactItem,
                                     implementation::ContactSubsectionRowID> {
    auto Name(const UnallocatedCString& lang) const noexcept
        -> UnallocatedCString final
    {
        return {};
    }
    auto Type() const noexcept -> identity::wot::claim::ClaimType final
    {
        return {};
    }

    auto reindex(
        const implementation::ContactSectionSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct IssuerItem final : public List<
                              internal::IssuerItem,
                              OTUIAccountSummaryItem,
                              implementation::IssuerItemRowID> {
    auto ConnectionState() const noexcept -> bool final { return {}; }
    auto Debug() const noexcept -> UnallocatedCString final { return {}; }
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto Trusted() const noexcept -> bool final { return {}; }

    auto reindex(
        const implementation::AccountSummarySortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct NymListItem : virtual public Row, virtual public internal::NymListItem {
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto NymID() const noexcept -> UnallocatedCString final { return {}; }

    auto reindex(
        const implementation::NymListSortKey&,
        implementation::CustomData&) noexcept -> bool override
    {
        return false;
    }
};
struct PayableListItem final : virtual public ContactListItem,
                               virtual public internal::PayableListItem {
    auto PaymentCode() const noexcept -> UnallocatedCString final { return {}; }

    auto reindex(
        const implementation::PayableListSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct ProfileItem : virtual public Row, virtual public internal::ProfileItem {
    auto ClaimID() const noexcept -> UnallocatedCString final { return {}; }
    auto Delete() const noexcept -> bool final { return false; }
    auto IsActive() const noexcept -> bool final { return false; }
    auto IsPrimary() const noexcept -> bool final { return false; }
    auto Value() const noexcept -> UnallocatedCString final { return {}; }
    auto SetActive(const bool& active) const noexcept -> bool final
    {
        return false;
    }
    auto SetPrimary(const bool& primary) const noexcept -> bool final
    {
        return false;
    }
    auto SetValue(const UnallocatedCString& value) const noexcept -> bool final
    {
        return false;
    }

    auto reindex(
        const implementation::ProfileSubsectionSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct ProfileSection : public List<
                            internal::ProfileSection,
                            OTUIProfileSubsection,
                            implementation::ProfileSectionRowID> {
    auto AddClaim(
        const identity::wot::claim::ClaimType type,
        const UnallocatedCString& value,
        const bool primary,
        const bool active) const noexcept -> bool final
    {
        return false;
    }
    auto Delete(const int, const UnallocatedCString&) const noexcept
        -> bool final
    {
        return false;
    }
    auto Items(const UnallocatedCString&) const noexcept -> ItemTypeList final
    {
        return {};
    }
    auto Name(const UnallocatedCString& lang) const noexcept
        -> UnallocatedCString final
    {
        return {};
    }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return nym_id_;
    }
    auto SetActive(const int, const UnallocatedCString&, const bool)
        const noexcept -> bool final
    {
        return false;
    }
    auto SetPrimary(const int, const UnallocatedCString&, const bool)
        const noexcept -> bool final
    {
        return false;
    }
    auto SetValue(
        const int,
        const UnallocatedCString&,
        const UnallocatedCString&) const noexcept -> bool final
    {
        return false;
    }
    auto Type() const noexcept -> identity::wot::claim::SectionType final
    {
        return {};
    }

    auto reindex(
        const implementation::ProfileSortKey& key,
        implementation::CustomData& custom) noexcept -> bool final
    {
        return false;
    }

private:
    const identifier::Nym nym_id_{};
};
struct ProfileSubsection : public List<
                               internal::ProfileSubsection,
                               OTUIProfileItem,
                               implementation::ProfileSubsectionRowID> {
    auto AddItem(const UnallocatedCString&, const bool, const bool)
        const noexcept -> bool final
    {
        return false;
    }
    auto Delete(const UnallocatedCString&) const noexcept -> bool final
    {
        return false;
    }
    auto Name(const UnallocatedCString&) const noexcept
        -> UnallocatedCString final
    {
        return {};
    }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return nym_id_;
    }
    auto Type() const noexcept -> identity::wot::claim::ClaimType final
    {
        return {};
    }
    auto Section() const noexcept -> identity::wot::claim::SectionType final
    {
        return {};
    }
    auto SetActive(const UnallocatedCString&, const bool) const noexcept
        -> bool final
    {
        return false;
    }
    auto SetPrimary(const UnallocatedCString&, const bool) const noexcept
        -> bool final
    {
        return false;
    }
    auto SetValue(const UnallocatedCString&, const UnallocatedCString&)
        const noexcept -> bool final
    {
        return false;
    }

    auto reindex(
        const implementation::ProfileSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }

private:
    const identifier::Nym nym_id_{};
};
struct SeedTreeItem final : public List<
                                internal::SeedTreeItem,
                                SharedPimpl<ui::SeedTreeNym>,
                                implementation::SeedTreeItemRowID> {
    auto SeedID() const noexcept -> UnallocatedCString final { return {}; }
    auto Debug() const noexcept -> UnallocatedCString final { return {}; }
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto Type() const noexcept -> crypto::SeedStyle final { return {}; }

    auto reindex(
        const implementation::SeedTreeSortKey& key,
        implementation::CustomData& custom) noexcept -> bool final
    {
        return {};
    }

    ~SeedTreeItem() final = default;
};
struct SeedTreeNym final : public Row, public internal::SeedTreeNym {
    auto NymID() const noexcept -> UnallocatedCString final { return {}; }
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto Index() const noexcept -> std::size_t final { return {}; }

    auto reindex(
        const implementation::SeedTreeItemSortKey& key,
        implementation::CustomData& custom) noexcept -> bool final
    {
        return {};
    }

    ~SeedTreeNym() final = default;
};
struct UnitListItem final : virtual public Row,
                            virtual public internal::UnitListItem {
    auto Name() const noexcept -> UnallocatedCString final { return {}; }
    auto Unit() const noexcept -> UnitType final { return {}; }

    auto reindex(
        const implementation::UnitListSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
}  // namespace blank

auto make_progress(
    blockchain::block::Height& actual,
    blockchain::block::Height& target) noexcept -> double;
}  // namespace opentxs::ui::internal

namespace opentxs::ui::qt::internal
{
struct Index {
    bool valid_{false};
    int row_{-1};
    int column_{-1};
    ui::internal::Row* ptr_{nullptr};
};

struct Model {
    using Row = ui::internal::Row;
    using RoleData = UnallocatedVector<std::pair<int, UnallocatedCString>>;

    static auto GetID(const ui::internal::Row* ptr) noexcept -> std::ptrdiff_t;

    auto GetChild(ui::internal::Row* parent, int index) const noexcept
        -> ui::internal::Row*;
    auto GetColumnCount(ui::internal::Row* row) const noexcept -> int;
    auto GetIndex(ui::internal::Row* row) const noexcept -> Index;
    auto GetParent(ui::internal::Row* row) const noexcept -> Index;
    auto GetRoot() const noexcept -> QObject*;
    auto GetRoleData() const noexcept -> RoleData;
    auto GetRowCount(ui::internal::Row* row) const noexcept -> int;

    auto ChangeRow(ui::internal::Row* parent, ui::internal::Row* row) noexcept
        -> void;
    auto ClearParent() noexcept -> void;
    auto DeleteRow(ui::internal::Row* row) noexcept -> void;
    auto InsertRow(
        ui::internal::Row* parent,
        ui::internal::Row* after,
        std::shared_ptr<ui::internal::Row> row) noexcept -> void;
    auto MoveRow(
        ui::internal::Row* newParent,
        ui::internal::Row* newBefore,
        ui::internal::Row* row) noexcept -> void;
    auto SetColumnCount(Row* parent, int count) noexcept -> void;
    auto SetColumnCount(const Lock& lock, Row* parent, int count) noexcept
        -> void;
    auto SetRoleData(RoleData&& data) noexcept -> void;
    auto SetParent(qt::Model& parent) noexcept -> void;

    Model(QObject* parent) noexcept;
    Model() = delete;
    Model(const Model&) = delete;
    Model(Model&&) = delete;
    auto operator=(const Model&) -> Model& = delete;
    auto operator=(Model&&) -> Model& = delete;

    ~Model();

private:
    friend ui::qt::Model;

    struct Imp;

    Imp* imp_;

    auto do_delete_row(ui::internal::Row* row) noexcept -> void;
    auto do_insert_row(
        ui::internal::Row* parent,
        ui::internal::Row* after,
        std::shared_ptr<ui::internal::Row> row) noexcept -> void;
    auto do_move_row(
        ui::internal::Row* newParent,
        ui::internal::Row* newBefore,
        ui::internal::Row* row) noexcept -> void;
};
}  // namespace opentxs::ui::qt::internal

namespace opentxs::factory
{
auto AccountActivityQtModel(ui::internal::AccountActivity& parent) noexcept
    -> std::unique_ptr<ui::AccountActivityQt>;
auto AccountCurrencyWidget(
    const ui::implementation::AccountTreeInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountTreeRowID& rowID,
    const ui::implementation::AccountTreeSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountTreeRowInternal>;
auto AccountListItem(
    const ui::implementation::AccountListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountListRowID& rowID,
    const ui::implementation::AccountListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountListRowInternal>;
auto AccountListItemBlockchain(
    const ui::implementation::AccountListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountListRowID& rowID,
    const ui::implementation::AccountListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountListRowInternal>;
auto AccountListItemCustodial(
    const ui::implementation::AccountListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountListRowID& rowID,
    const ui::implementation::AccountListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountListRowInternal>;
auto AccountListModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountList>;
auto AccountListQtModel(ui::internal::AccountList& parent) noexcept
    -> std::unique_ptr<ui::AccountListQt>;
auto AccountSummaryItem(
    const ui::implementation::IssuerItemInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::IssuerItemRowID& rowID,
    const ui::implementation::IssuerItemSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::IssuerItemRowInternal>;
auto AccountSummaryModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const UnitType currency,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountSummary>;
auto AccountSummaryQtModel(ui::internal::AccountSummary& parent) noexcept
    -> std::unique_ptr<ui::AccountSummaryQt>;
auto AccountTreeItem(
    const ui::implementation::AccountCurrencyInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountCurrencyRowID& rowID,
    const ui::implementation::AccountCurrencySortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountCurrencyRowInternal>;
auto AccountTreeItemBlockchain(
    const ui::implementation::AccountCurrencyInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountCurrencyRowID& rowID,
    const ui::implementation::AccountCurrencySortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountCurrencyRowInternal>;
auto AccountTreeItemCustodial(
    const ui::implementation::AccountCurrencyInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountCurrencyRowID& rowID,
    const ui::implementation::AccountCurrencySortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountCurrencyRowInternal>;
auto AccountTreeModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountTree>;
auto AccountTreeQtModel(ui::internal::AccountTree& parent) noexcept
    -> std::unique_ptr<ui::AccountTreeQt>;
auto ActivitySummaryItem(
    const ui::implementation::ActivitySummaryInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivitySummaryRowID& rowID,
    const ui::implementation::ActivitySummarySortKey& sortKey,
    ui::implementation::CustomData& custom,
    const Flag& running) noexcept
    -> std::shared_ptr<ui::implementation::ActivitySummaryRowInternal>;
auto ActivitySummaryModel(
    const api::session::Client& api,
    const Flag& running,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::ActivitySummary>;
auto ActivitySummaryQtModel(ui::internal::ActivitySummary& parent) noexcept
    -> std::unique_ptr<ui::ActivitySummaryQt>;
auto ActivityThreadModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Generic& threadID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::ActivityThread>;
auto ActivityThreadQtModel(ui::internal::ActivityThread& parent) noexcept
    -> std::unique_ptr<ui::ActivityThreadQt>;
auto BlockchainAccountActivityModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Generic& accountID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountActivity>;
auto BlockchainAccountStatusModel(
    const api::session::Client& api,
    const ui::implementation::BlockchainAccountStatusPrimaryID& id,
    const blockchain::Type chain,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::BlockchainAccountStatus>;
auto BlockchainAccountStatusQtModel(
    ui::internal::BlockchainAccountStatus& parent) noexcept
    -> std::unique_ptr<ui::BlockchainAccountStatusQt>;
auto BlockchainActivityThreadItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>;
auto BlockchainSelectionModel(
    const api::session::Client& api,
    const ui::Blockchains type,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::BlockchainSelection>;
auto BlockchainSelectionItem(
    const ui::implementation::BlockchainSelectionInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::BlockchainSelectionRowID& rowID,
    const ui::implementation::BlockchainSelectionSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::BlockchainSelectionRowInternal>;
auto BlockchainSelectionQtModel(
    ui::internal::BlockchainSelection& parent) noexcept
    -> std::unique_ptr<ui::BlockchainSelectionQt>;
auto BlockchainStatisticsModel(
    const api::session::Client& api,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::BlockchainStatistics>;
auto BlockchainStatisticsItem(
    const ui::implementation::BlockchainStatisticsInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::BlockchainStatisticsRowID& rowID,
    const ui::implementation::BlockchainStatisticsSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::BlockchainStatisticsRowInternal>;
auto BlockchainStatisticsQtModel(
    ui::internal::BlockchainStatistics& parent) noexcept
    -> std::unique_ptr<ui::BlockchainStatisticsQt>;
auto BlockchainSubaccountSourceWidget(
    const ui::implementation::BlockchainAccountStatusInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::BlockchainAccountStatusRowID& rowID,
    const ui::implementation::BlockchainAccountStatusSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::BlockchainAccountStatusRowInternal>;
auto BlockchainSubaccountWidget(
    const ui::implementation::BlockchainSubaccountSourceInternalInterface&
        parent,
    const api::session::Client& api,
    const ui::implementation::BlockchainSubaccountSourceRowID& rowID,
    const ui::implementation::BlockchainSubaccountSourceSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<
        ui::implementation::BlockchainSubaccountSourceRowInternal>;
auto BlockchainSubchainWidget(
    const ui::implementation::BlockchainSubaccountInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::BlockchainSubaccountRowID& rowID,
    const ui::implementation::BlockchainSubaccountSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::BlockchainSubaccountRowInternal>;
auto BalanceItem(
    const ui::implementation::AccountActivityInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountActivityRowID& rowID,
    const ui::implementation::AccountActivitySortKey& sortKey,
    ui::implementation::CustomData& custom,
    const identifier::Nym& nymID,
    const identifier::Generic& accountID) noexcept
    -> std::shared_ptr<ui::implementation::AccountActivityRowInternal>;
auto BalanceItemBlockchain(
    const ui::implementation::AccountActivityInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountActivityRowID& rowID,
    const ui::implementation::AccountActivitySortKey& sortKey,
    ui::implementation::CustomData& custom,
    const identifier::Nym& nymID,
    const identifier::Generic& accountID) noexcept
    -> std::shared_ptr<ui::implementation::AccountActivityRowInternal>;
auto BalanceItemCustodial(
    const ui::implementation::AccountActivityInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountActivityRowID& rowID,
    const ui::implementation::AccountActivitySortKey& sortKey,
    ui::implementation::CustomData& custom,
    const identifier::Nym& nymID,
    const identifier::Generic& accountID) noexcept
    -> std::shared_ptr<ui::implementation::AccountActivityRowInternal>;
auto ContactItemWidget(
    const ui::implementation::ContactSubsectionInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ContactSubsectionRowID& rowID,
    const ui::implementation::ContactSubsectionSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ContactSubsectionRowInternal>;
auto ContactListItem(
    const ui::implementation::ContactListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ContactListRowID& rowID,
    const ui::implementation::ContactListSortKey& key) noexcept
    -> std::shared_ptr<ui::implementation::ContactListRowInternal>;
auto ContactListModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::ContactList>;
auto ContactListQtModel(ui::internal::ContactList& parent) noexcept
    -> std::unique_ptr<ui::ContactListQt>;
auto ContactModel(
    const api::session::Client& api,
    const ui::implementation::ContactPrimaryID& contactID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::Contact>;
auto ContactQtModel(ui::internal::Contact& parent) noexcept
    -> std::unique_ptr<ui::ContactQt>;
auto ContactSectionWidget(
    const ui::implementation::ContactInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ContactRowID& rowID,
    const ui::implementation::ContactSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ContactRowInternal>;
auto ContactSubsectionWidget(
    const ui::implementation::ContactSectionInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ContactSectionRowID& rowID,
    const ui::implementation::ContactSectionSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ContactSectionRowInternal>;
auto CustodialAccountActivityModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const identifier::Generic& accountID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountActivity>;
auto IssuerItem(
    const ui::implementation::AccountSummaryInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountSummaryRowID& rowID,
    const ui::implementation::AccountSummarySortKey& sortKey,
    ui::implementation::CustomData& custom,
    const UnitType currency) noexcept
    -> std::shared_ptr<ui::implementation::AccountSummaryRowInternal>;
auto MailItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>;
auto MessagableListItem(
    const ui::implementation::ContactListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ContactListRowID& rowID,
    const ui::implementation::ContactListSortKey& key) noexcept
    -> std::shared_ptr<ui::implementation::MessagableListRowInternal>;
auto MessagableListModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::MessagableList>;
auto MessagableListQtModel(ui::internal::MessagableList& parent) noexcept
    -> std::unique_ptr<ui::MessagableListQt>;
auto NymListItem(
    const ui::implementation::NymListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::NymListRowID& rowID,
    const ui::implementation::NymListSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::NymListRowInternal>;
auto NymListModel(
    const api::session::Client& api,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::NymList>;
auto NymListQtModel(ui::internal::NymList& parent) noexcept
    -> std::unique_ptr<ui::NymListQt>;
auto PayableListItem(
    const ui::implementation::PayableInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::PayableListRowID& rowID,
    const ui::implementation::PayableListSortKey& key,
    const UnallocatedCString& paymentcode,
    const UnitType& currency) noexcept
    -> std::shared_ptr<ui::implementation::PayableListRowInternal>;
auto PaymentItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>;
auto PayableListModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const UnitType& currency,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::PayableList>;
auto PayableListQtModel(ui::internal::PayableList& parent) noexcept
    -> std::unique_ptr<ui::PayableListQt>;
auto PendingSend(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>;
auto ProfileModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::Profile>;
auto ProfileQtModel(ui::internal::Profile& parent) noexcept
    -> std::unique_ptr<ui::ProfileQt>;
auto ProfileItemWidget(
    const ui::implementation::ProfileSubsectionInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ProfileSubsectionRowID& rowID,
    const ui::implementation::ProfileSubsectionSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ProfileSubsectionRowInternal>;
auto ProfileSectionWidget(
    const ui::implementation::ProfileInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ProfileRowID& rowID,
    const ui::implementation::ProfileSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ProfileRowInternal>;
auto ProfileSubsectionWidget(
    const ui::implementation::ProfileSectionInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ProfileSectionRowID& rowID,
    const ui::implementation::ProfileSectionSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ProfileSectionRowInternal>;
auto SeedTreeModel(
    const api::session::Client& api,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::SeedTree>;
auto SeedTreeQtModel(ui::internal::SeedTree& parent) noexcept
    -> std::unique_ptr<ui::SeedTreeQt>;
auto SeedTreeItemModel(
    const ui::implementation::SeedTreeInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::SeedTreeRowID& rowID,
    const ui::implementation::SeedTreeSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::SeedTreeRowInternal>;
auto SeedTreeNym(
    const ui::implementation::SeedTreeItemInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::SeedTreeItemRowID& rowID,
    const ui::implementation::SeedTreeItemSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::SeedTreeItemRowInternal>;
auto UnitListItem(
    const ui::implementation::UnitListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::UnitListRowID& rowID,
    const ui::implementation::UnitListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::UnitListRowInternal>;
auto UnitListModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::UnitList>;
auto UnitListQtModel(ui::internal::UnitList& parent) noexcept
    -> std::unique_ptr<ui::UnitListQt>;
}  // namespace opentxs::factory

namespace std
{
// NOLINTBEGIN(cert-dcl58-cpp)
template <>
struct less<opentxs::ui::implementation::ActivityThreadRowID> {
    auto operator()(
        const opentxs::ui::implementation::ActivityThreadRowID& lhs,
        const opentxs::ui::implementation::ActivityThreadRowID& rhs) const
        -> bool;
};
template <>
struct less<opentxs::ui::implementation::BlockchainSelectionSortKey> {
    auto operator()(
        const opentxs::ui::implementation::BlockchainSelectionSortKey& lhs,
        const opentxs::ui::implementation::BlockchainSelectionSortKey& rhs)
        const -> bool;
};
template <>
struct less<opentxs::ui::implementation::ContactListSortKey> {
    auto operator()(
        const opentxs::ui::implementation::ContactListSortKey& lhs,
        const opentxs::ui::implementation::ContactListSortKey& rhs) const
        -> bool;
};
// NOLINTEND(cert-dcl58-cpp)
}  // namespace std
