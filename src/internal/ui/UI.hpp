// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"
// IWYU pragma: no_include "opentxs/contact/ClaimType.hpp"
// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountList.hpp"
#include "opentxs/ui/AccountListItem.hpp"
#include "opentxs/ui/AccountSummary.hpp"
#include "opentxs/ui/AccountSummaryItem.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivitySummaryItem.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"
#include "opentxs/ui/BalanceItem.hpp"
#include "opentxs/ui/BlockchainAccountStatus.hpp"
#include "opentxs/ui/BlockchainSelection.hpp"
#include "opentxs/ui/BlockchainSelectionItem.hpp"
#include "opentxs/ui/BlockchainStatistics.hpp"
#include "opentxs/ui/BlockchainStatisticsItem.hpp"
#include "opentxs/ui/BlockchainSubaccount.hpp"
#include "opentxs/ui/BlockchainSubaccountSource.hpp"
#include "opentxs/ui/BlockchainSubchain.hpp"
#include "opentxs/ui/Contact.hpp"
#include "opentxs/ui/ContactItem.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/ContactListItem.hpp"
#include "opentxs/ui/ContactSection.hpp"
#include "opentxs/ui/ContactSubsection.hpp"
#include "opentxs/ui/IssuerItem.hpp"
#include "opentxs/ui/List.hpp"
#include "opentxs/ui/ListRow.hpp"
#include "opentxs/ui/MessagableList.hpp"
#include "opentxs/ui/PayableList.hpp"
#include "opentxs/ui/PayableListItem.hpp"
#include "opentxs/ui/Profile.hpp"
#include "opentxs/ui/ProfileItem.hpp"
#include "opentxs/ui/ProfileSection.hpp"
#include "opentxs/ui/ProfileSubsection.hpp"
#include "opentxs/ui/Types.hpp"
#include "opentxs/ui/UnitList.hpp"
#include "opentxs/ui/UnitListItem.hpp"
#include "opentxs/ui/Widget.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/PaymentWorkflowEnums.pb.h"
#include "ui/qt/SendMonitor.hpp"
#include "util/Blank.hpp"

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
class Server;
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
struct AccountListItem;
struct AccountSummaryItem;
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
struct PayableListItem;
struct ProfileItem;
struct ProfileSection;
struct ProfileSubsection;
struct UnitListItem;
}  // namespace blank

struct AccountActivity;
struct AccountList;
struct AccountListItem;
struct AccountSummary;
struct AccountSummaryItem;
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
struct PayableList;
struct PayableListItem;
struct Profile;
struct ProfileItem;
struct ProfileSection;
struct ProfileSubsection;
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
class PayableListQt;
class ProfileQt;
class UnitListQt;
}  // namespace ui

class Flag;
}  // namespace opentxs

class QObject;
class QVariant;

namespace opentxs::ui
{
auto claim_ownership(QObject* object) noexcept -> void;
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
using CustomData = std::pmr::vector<void*>;

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

private:
    ChildObjectData() = delete;
    ChildObjectData(const ChildObjectData&) = delete;
    auto operator=(const ChildObjectData&) -> ChildObjectData& = delete;
    auto operator=(ChildObjectData&&) -> ChildObjectData& = delete;
};

// Account activity
using AccountActivityPrimaryID = OTNymID;
using AccountActivityExternalInterface = ui::AccountActivity;
using AccountActivityInternalInterface = ui::internal::AccountActivity;
/** WorkflowID, state */
using AccountActivityRowID = std::pair<OTIdentifier, proto::PaymentEventType>;
using AccountActivityRowInterface = ui::BalanceItem;
using AccountActivityRowInternal = ui::internal::BalanceItem;
using AccountActivityRowBlank = ui::internal::blank::BalanceItem;
using AccountActivitySortKey = Time;

// Account list
using AccountListPrimaryID = OTNymID;
using AccountListExternalInterface = ui::AccountList;
using AccountListInternalInterface = ui::internal::AccountList;
using AccountListRowID = OTIdentifier;
using AccountListRowInterface = ui::AccountListItem;
using AccountListRowInternal = ui::internal::AccountListItem;
using AccountListRowBlank = ui::internal::blank::AccountListItem;
// type, notary ID
using AccountListSortKey = std::pair<core::UnitType, std::string>;

// Account summary
using AccountSummaryPrimaryID = OTNymID;
using AccountSummaryExternalInterface = ui::AccountSummary;
using AccountSummaryInternalInterface = ui::internal::AccountSummary;
using AccountSummaryRowID = OTNymID;
using AccountSummaryRowInterface = ui::IssuerItem;
using AccountSummaryRowInternal = ui::internal::IssuerItem;
using AccountSummaryRowBlank = ui::internal::blank::IssuerItem;
using AccountSummarySortKey = std::pair<bool, std::string>;

using IssuerItemPrimaryID = OTNymID;
using IssuerItemExternalInterface = AccountSummaryRowInterface;
using IssuerItemInternalInterface = ui::internal::IssuerItem;
using IssuerItemRowID = std::pair<OTIdentifier, core::UnitType>;
using IssuerItemRowInterface = ui::AccountSummaryItem;
using IssuerItemRowInternal = ui::internal::AccountSummaryItem;
using IssuerItemRowBlank = ui::internal::blank::AccountSummaryItem;
using IssuerItemSortKey = std::string;

// Activity summary
using ActivitySummaryPrimaryID = OTNymID;
using ActivitySummaryExternalInterface = ui::ActivitySummary;
using ActivitySummaryInternalInterface = ui::internal::ActivitySummary;
using ActivitySummaryRowID = OTIdentifier;
using ActivitySummaryRowInterface = ui::ActivitySummaryItem;
using ActivitySummaryRowInternal = ui::internal::ActivitySummaryItem;
using ActivitySummaryRowBlank = ui::internal::blank::ActivitySummaryItem;
using ActivitySummarySortKey = std::pair<Time, std::string>;

// Activity thread
using ActivityThreadPrimaryID = OTNymID;
using ActivityThreadExternalInterface = ui::ActivityThread;
using ActivityThreadInternalInterface = ui::internal::ActivityThread;
/** item id, box, accountID, taskID */
using ActivityThreadRowID = std::tuple<OTIdentifier, StorageBox, OTIdentifier>;
using ActivityThreadRowInterface = ui::ActivityThreadItem;
using ActivityThreadRowInternal = ui::internal::ActivityThreadItem;
using ActivityThreadRowBlank = ui::internal::blank::ActivityThreadItem;
/** timestamp, index */
using ActivityThreadSortKey = std::pair<Time, std::uint64_t>;

// Blockchain account status

using BlockchainAccountStatusPrimaryID = OTNymID;
using BlockchainAccountStatusExternalInterface = ui::BlockchainAccountStatus;
using BlockchainAccountStatusInternalInterface =
    ui::internal::BlockchainAccountStatus;
// NOTE: seed id, local payment code id, or private key id
using BlockchainAccountStatusRowID = OTIdentifier;
using BlockchainAccountStatusRowInterface = ui::BlockchainSubaccountSource;
using BlockchainAccountStatusRowInternal =
    ui::internal::BlockchainSubaccountSource;
using BlockchainAccountStatusRowBlank =
    ui::internal::blank::BlockchainSubaccountSource;
using BlockchainAccountStatusSortKey =
    std::pair<blockchain::crypto::SubaccountType, std::string>;

using BlockchainSubaccountSourcePrimaryID = BlockchainAccountStatusPrimaryID;
using BlockchainSubaccountSourceExternalInterface =
    ui::BlockchainSubaccountSource;
using BlockchainSubaccountSourceInternalInterface =
    ui::internal::BlockchainSubaccountSource;
// NOTE: subaccount id
using BlockchainSubaccountSourceRowID = OTIdentifier;
using BlockchainSubaccountSourceRowInterface = ui::BlockchainSubaccount;
using BlockchainSubaccountSourceRowInternal =
    ui::internal::BlockchainSubaccount;
using BlockchainSubaccountSourceRowBlank =
    ui::internal::blank::BlockchainSubaccount;
using BlockchainSubaccountSourceSortKey = std::string;
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
using BlockchainSubaccountSortKey = std::string;
using BlockchainSubaccountRowData =
    ChildObjectData<BlockchainSubaccountRowID, BlockchainSubaccountSortKey>;

// Blockchain selection
using BlockchainSelectionPrimaryID = OTIdentifier;
using BlockchainSelectionExternalInterface = ui::BlockchainSelection;
using BlockchainSelectionInternalInterface = ui::internal::BlockchainSelection;
using BlockchainSelectionRowID = blockchain::Type;
using BlockchainSelectionRowInterface = ui::BlockchainSelectionItem;
using BlockchainSelectionRowInternal = ui::internal::BlockchainSelectionItem;
using BlockchainSelectionRowBlank =
    ui::internal::blank::BlockchainSelectionItem;
using BlockchainSelectionSortKey = std::pair<std::string, bool>;

// Blockchain statistics
using BlockchainStatisticsPrimaryID = OTIdentifier;
using BlockchainStatisticsExternalInterface = ui::BlockchainStatistics;
using BlockchainStatisticsInternalInterface =
    ui::internal::BlockchainStatistics;
using BlockchainStatisticsRowID = blockchain::Type;
using BlockchainStatisticsRowInterface = ui::BlockchainStatisticsItem;
using BlockchainStatisticsRowInternal = ui::internal::BlockchainStatisticsItem;
using BlockchainStatisticsRowBlank =
    ui::internal::blank::BlockchainStatisticsItem;
using BlockchainStatisticsSortKey = std::string;

// Contact
using ContactPrimaryID = OTIdentifier;
using ContactExternalInterface = ui::Contact;
using ContactInternalInterface = ui::internal::Contact;
using ContactRowID = contact::SectionType;
using ContactRowInterface = ui::ContactSection;
using ContactRowInternal = ui::internal::ContactSection;
using ContactRowBlank = ui::internal::blank::ContactSection;
using ContactSortKey = int;

using ContactSectionPrimaryID = ContactPrimaryID;
using ContactSectionExternalInterface = ContactRowInterface;
using ContactSectionInternalInterface = ui::internal::ContactSection;
using ContactSectionRowID = std::pair<contact::SectionType, contact::ClaimType>;
using ContactSectionRowInterface = ui::ContactSubsection;
using ContactSectionRowInternal = ui::internal::ContactSubsection;
using ContactSectionRowBlank = ui::internal::blank::ContactSubsection;
using ContactSectionSortKey = int;

using ContactSubsectionPrimaryID = ContactSectionPrimaryID;
using ContactSubsectionExternalInterface = ContactSectionRowInterface;
using ContactSubsectionInternalInterface = ui::internal::ContactSubsection;
using ContactSubsectionRowID = OTIdentifier;
using ContactSubsectionRowInterface = ui::ContactItem;
using ContactSubsectionRowInternal = ui::internal::ContactItem;
using ContactSubsectionRowBlank = ui::internal::blank::ContactItem;
using ContactSubsectionSortKey = int;

// Contact list
using ContactListPrimaryID = OTNymID;
using ContactListExternalInterface = ui::ContactList;
using ContactListInternalInterface = ui::internal::ContactListType;
using ContactListRowID = OTIdentifier;
using ContactListRowInterface = ui::ContactListItem;
using ContactListRowInternal = ui::internal::ContactListItem;
using ContactListRowBlank = ui::internal::blank::ContactListItem;
// Items with the first value set to true will be sorted first
using ContactListSortKey = std::pair<bool, std::string>;

// Messagable list
using MessagableListPrimaryID = OTNymID;
using MessagableExternalInterface = ui::MessagableList;
using MessagableInternalInterface = ui::internal::MessagableList;
using MessagableListRowID = ContactListRowID;
using MessagableListRowInterface = ContactListRowInterface;
using MessagableListRowInternal = ContactListRowInternal;
using MessagableListRowBlank = ContactListRowBlank;
using MessagableListSortKey = ContactListSortKey;

// Payable list
using PayablePrimaryID = OTNymID;
using PayableExternalInterface = ui::PayableList;
using PayableInternalInterface = ui::internal::PayableList;
using PayableListRowID = ContactListRowID;
using PayableListRowInterface = ui::PayableListItem;
using PayableListRowInternal = ui::internal::PayableListItem;
using PayableListRowBlank = ui::internal::blank::PayableListItem;
using PayableListSortKey = ContactListSortKey;

// Profile
using ProfilePrimaryID = OTNymID;
using ProfileExternalInterface = ui::Profile;
using ProfileInternalInterface = ui::internal::Profile;
using ProfileRowID = contact::SectionType;
using ProfileRowInterface = ui::ProfileSection;
using ProfileRowInternal = ui::internal::ProfileSection;
using ProfileRowBlank = ui::internal::blank::ProfileSection;
using ProfileSortKey = int;

using ProfileSectionPrimaryID = ProfilePrimaryID;
using ProfileSectionExternalInterface = ProfileRowInterface;
using ProfileSectionInternalInterface = ui::internal::ProfileSection;
using ProfileSectionRowID = std::pair<contact::SectionType, contact::ClaimType>;
using ProfileSectionRowInterface = ui::ProfileSubsection;
using ProfileSectionRowInternal = ui::internal::ProfileSubsection;
using ProfileSectionRowBlank = ui::internal::blank::ProfileSubsection;
using ProfileSectionSortKey = int;

using ProfileSubsectionPrimaryID = ProfileSectionPrimaryID;
using ProfileSubsectionExternalInterface = ProfileSectionRowInterface;
using ProfileSubsectionInternalInterface = ui::internal::ProfileSubsection;
using ProfileSubsectionRowID = OTIdentifier;
using ProfileSubsectionRowInterface = ui::ProfileItem;
using ProfileSubsectionRowInternal = ui::internal::ProfileItem;
using ProfileSubsectionRowBlank = ui::internal::blank::ProfileItem;
using ProfileSubsectionSortKey = int;

// Unit list
using UnitListPrimaryID = OTNymID;
using UnitListExternalInterface = ui::UnitList;
using UnitListInternalInterface = ui::internal::UnitList;
using UnitListRowID = core::UnitType;
using UnitListRowInterface = ui::UnitListItem;
using UnitListRowInternal = ui::internal::UnitListItem;
using UnitListRowBlank = ui::internal::blank::UnitListItem;
using UnitListSortKey = std::string;
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
        using BalanceCallback = std::function<void(std::string)>;
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
        const std::string& address,
        const std::string& amount,
        const std::string& memo,
        Scale scale,
        implementation::SendMonitor::Callback cb) const noexcept -> int = 0;
    virtual auto Send(
        const Identifier& contact,
        const std::string& amount,
        const std::string& memo,
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
    virtual auto Currency() const -> core::UnitType = 0;
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
        using Callback = std::function<void(std::string)>;

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
    virtual auto ID() const noexcept -> const Identifier& = 0;
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
    virtual auto ContactID() const noexcept -> std::string = 0;
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
        using Callback = std::function<void(std::string)>;

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
    virtual auto Section() const noexcept -> contact::SectionType = 0;

    ~ProfileSubsection() override = default;
};
struct ProfileItem : virtual public Row, virtual public ui::ProfileItem {
    virtual auto reindex(
        const implementation::ProfileSubsectionSortKey& key,
        implementation::CustomData& custom) noexcept -> bool = 0;

    ~ProfileItem() override = default;
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
    auto WidgetID() const noexcept -> OTIdentifier override
    {
        return Identifier::Factory();
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
    auto WidgetID() const noexcept -> OTIdentifier final
    {
        return blank::Widget::WidgetID();
    }

    auto GetQt() const noexcept -> ui::qt::internal::Model* final
    {
        return nullptr;
    }
};
struct AccountListItem final : virtual public Row,
                               virtual public internal::AccountListItem {
    auto AccountID() const noexcept -> std::string final { return {}; }
    auto Balance() const noexcept -> Amount final { return {}; }
    auto ContractID() const noexcept -> std::string final { return {}; }
    auto DisplayBalance() const noexcept -> std::string final { return {}; }
    auto DisplayUnit() const noexcept -> std::string final { return {}; }
    auto Name() const noexcept -> std::string final { return {}; }
    auto NotaryID() const noexcept -> std::string final { return {}; }
    auto NotaryName() const noexcept -> std::string final { return {}; }
    auto Type() const noexcept -> AccountType final { return {}; }
    auto Unit() const noexcept -> core::UnitType final { return {}; }

    auto reindex(
        const implementation::AccountListSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct AccountSummaryItem final : public Row,
                                  public internal::AccountSummaryItem {
    auto AccountID() const noexcept -> std::string final { return {}; }
    auto Balance() const noexcept -> Amount final { return {}; }
    auto DisplayBalance() const noexcept -> std::string final { return {}; }
    auto Name() const noexcept -> std::string final { return {}; }

    auto reindex(
        const implementation::IssuerItemSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct ActivitySummaryItem final
    : virtual public Row,
      virtual public internal::ActivitySummaryItem {
    auto DisplayName() const noexcept -> std::string final { return {}; }
    auto ImageURI() const noexcept -> std::string final { return {}; }
    auto Text() const noexcept -> std::string final { return {}; }
    auto ThreadID() const noexcept -> std::string final { return {}; }
    auto Timestamp() const noexcept -> Time final { return {}; }
    auto Type() const noexcept -> StorageBox final
    {
        return StorageBox::UNKNOWN;
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
    auto DisplayAmount() const noexcept -> std::string final { return {}; }
    auto From() const noexcept -> std::string final { return {}; }
    auto Loading() const noexcept -> bool final { return false; }
    auto MarkRead() const noexcept -> bool final { return false; }
    auto Memo() const noexcept -> std::string final { return {}; }
    auto Outgoing() const noexcept -> bool final { return false; }
    auto Pending() const noexcept -> bool final { return false; }
    auto Text() const noexcept -> std::string final { return {}; }
    auto Timestamp() const noexcept -> Time final { return {}; }
    auto Type() const noexcept -> StorageBox final
    {
        return StorageBox::UNKNOWN;
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
    auto Contacts() const noexcept -> std::pmr::vector<std::string> final
    {
        return {};
    }
    auto DisplayAmount() const noexcept -> std::string final { return {}; }
    auto Memo() const noexcept -> std::string final { return {}; }
    auto Workflow() const noexcept -> std::string final { return {}; }
    auto Text() const noexcept -> std::string final { return {}; }
    auto Timestamp() const noexcept -> Time final { return {}; }
    auto Type() const noexcept -> StorageBox final
    {
        return StorageBox::UNKNOWN;
    }
    auto UUID() const noexcept -> std::string final { return {}; }

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

    auto Name() const noexcept -> std::string final { return {}; }
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
    auto Balance() const noexcept -> std::string final { return {}; }
    auto BlockDownloadQueue() const noexcept -> std::size_t final { return {}; }
    auto Chain() const noexcept -> blockchain::Type final { return {}; }
    auto ConnectedPeers() const noexcept -> std::size_t final { return {}; }
    auto Filters() const noexcept -> Position final { return {}; }
    auto Headers() const noexcept -> Position final { return {}; }
    auto Name() const noexcept -> std::string final { return {}; }

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
    auto Name() const noexcept -> std::string final { return {}; }
    auto NymID() const noexcept -> const identifier::Nym& final;
    auto SubaccountID() const noexcept -> const Identifier& final;

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
    auto Name() const noexcept -> std::string final { return {}; }
    auto NymID() const noexcept -> const identifier::Nym& final;
    auto SourceID() const noexcept -> const Identifier& final;
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
    auto Name() const noexcept -> std::string final { return {}; }
    auto Progress() const noexcept -> std::string final { return {}; }
    auto Type() const noexcept -> blockchain::crypto::Subchain final;

    auto reindex(
        const implementation::BlockchainSubaccountSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct ContactItem final : public Row, public internal::ContactItem {
    auto ClaimID() const noexcept -> std::string final { return {}; }
    auto IsActive() const noexcept -> bool final { return false; }
    auto IsPrimary() const noexcept -> bool final { return false; }
    auto Value() const noexcept -> std::string final { return {}; }

    auto reindex(
        const implementation::ContactSubsectionSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct ContactListItem : virtual public Row,
                         virtual public internal::ContactListItem {
    auto ContactID() const noexcept -> std::string final { return {}; }
    auto DisplayName() const noexcept -> std::string final { return {}; }
    auto ImageURI() const noexcept -> std::string final { return {}; }
    auto Section() const noexcept -> std::string final { return {}; }

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
    auto ContactID() const noexcept -> std::string final { return {}; }
    auto Name(const std::string& lang) const noexcept -> std::string final
    {
        return {};
    }
    auto Type() const noexcept -> contact::SectionType final { return {}; }

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
    auto Name(const std::string& lang) const noexcept -> std::string final
    {
        return {};
    }
    auto Type() const noexcept -> contact::ClaimType final { return {}; }

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
    auto Debug() const noexcept -> std::string final { return {}; }
    auto Name() const noexcept -> std::string final { return {}; }
    auto Trusted() const noexcept -> bool final { return {}; }

    auto reindex(
        const implementation::AccountSummarySortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct PayableListItem final : virtual public ContactListItem,
                               virtual public internal::PayableListItem {
    auto PaymentCode() const noexcept -> std::string final { return {}; }

    auto reindex(
        const implementation::PayableListSortKey&,
        implementation::CustomData&) noexcept -> bool final
    {
        return false;
    }
};
struct ProfileItem : virtual public Row, virtual public internal::ProfileItem {
    auto ClaimID() const noexcept -> std::string final { return {}; }
    auto Delete() const noexcept -> bool final { return false; }
    auto IsActive() const noexcept -> bool final { return false; }
    auto IsPrimary() const noexcept -> bool final { return false; }
    auto Value() const noexcept -> std::string final { return {}; }
    auto SetActive(const bool& active) const noexcept -> bool final
    {
        return false;
    }
    auto SetPrimary(const bool& primary) const noexcept -> bool final
    {
        return false;
    }
    auto SetValue(const std::string& value) const noexcept -> bool final
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
        const contact::ClaimType type,
        const std::string& value,
        const bool primary,
        const bool active) const noexcept -> bool final
    {
        return false;
    }
    auto Delete(const int, const std::string&) const noexcept -> bool final
    {
        return false;
    }
    auto Items(const std::string&) const noexcept -> ItemTypeList final
    {
        return {};
    }
    auto Name(const std::string& lang) const noexcept -> std::string final
    {
        return {};
    }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return nym_id_;
    }
    auto SetActive(const int, const std::string&, const bool) const noexcept
        -> bool final
    {
        return false;
    }
    auto SetPrimary(const int, const std::string&, const bool) const noexcept
        -> bool final
    {
        return false;
    }
    auto SetValue(const int, const std::string&, const std::string&)
        const noexcept -> bool final
    {
        return false;
    }
    auto Type() const noexcept -> contact::SectionType final { return {}; }

    auto reindex(
        const implementation::ProfileSortKey& key,
        implementation::CustomData& custom) noexcept -> bool final
    {
        return false;
    }

private:
    const OTNymID nym_id_{identifier::Nym::Factory()};
};
struct ProfileSubsection : public List<
                               internal::ProfileSubsection,
                               OTUIProfileItem,
                               implementation::ProfileSubsectionRowID> {
    auto AddItem(const std::string&, const bool, const bool) const noexcept
        -> bool final
    {
        return false;
    }
    auto Delete(const std::string&) const noexcept -> bool final
    {
        return false;
    }
    auto Name(const std::string&) const noexcept -> std::string final
    {
        return {};
    }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return nym_id_;
    }
    auto Type() const noexcept -> contact::ClaimType final { return {}; }
    auto Section() const noexcept -> contact::SectionType final { return {}; }
    auto SetActive(const std::string&, const bool) const noexcept -> bool final
    {
        return false;
    }
    auto SetPrimary(const std::string&, const bool) const noexcept -> bool final
    {
        return false;
    }
    auto SetValue(const std::string&, const std::string&) const noexcept
        -> bool final
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
    const OTNymID nym_id_{identifier::Nym::Factory()};
};
struct UnitListItem final : virtual public Row,
                            virtual public internal::UnitListItem {
    auto Name() const noexcept -> std::string final { return {}; }
    auto Unit() const noexcept -> core::UnitType final { return {}; }

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
    using RoleData = std::pmr::vector<std::pair<int, std::string>>;

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

    Model() = delete;
    Model(const Model&) = delete;
    Model(Model&&) = delete;
    Model& operator=(const Model&) = delete;
    Model& operator=(Model&&) = delete;
};
}  // namespace opentxs::ui::qt::internal

namespace opentxs::factory
{
auto AccountActivityQtModel(ui::internal::AccountActivity& parent) noexcept
    -> std::unique_ptr<ui::AccountActivityQt>;
auto AccountListItem(
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
    const core::UnitType currency,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountSummary>;
auto AccountSummaryQtModel(ui::internal::AccountSummary& parent) noexcept
    -> std::unique_ptr<ui::AccountSummaryQt>;
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
    const opentxs::Identifier& threadID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::ActivityThread>;
auto ActivityThreadQtModel(ui::internal::ActivityThread& parent) noexcept
    -> std::unique_ptr<ui::ActivityThreadQt>;
auto BlockchainAccountActivityModel(
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const opentxs::Identifier& accountID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountActivity>;
auto BlockchainAccountListItem(
    const ui::implementation::AccountListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountListRowID& rowID,
    const ui::implementation::AccountListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::AccountListRowInternal>;
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
    const opentxs::Identifier& accountID) noexcept
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
    const opentxs::Identifier& accountID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::AccountActivity>;
auto IssuerItem(
    const ui::implementation::AccountSummaryInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::AccountSummaryRowID& rowID,
    const ui::implementation::AccountSummarySortKey& sortKey,
    ui::implementation::CustomData& custom,
    const core::UnitType currency) noexcept
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
auto PayableListItem(
    const ui::implementation::PayableInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::PayableListRowID& rowID,
    const ui::implementation::PayableListSortKey& key,
    const std::string& paymentcode,
    const core::UnitType& currency) noexcept
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
    const core::UnitType& currency,
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
