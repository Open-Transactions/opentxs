// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

#include "api/session/ui/UI.hpp"
#include "api/session/ui/UpdateManager.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/interface/ui/AccountActivity.hpp"
#include "opentxs/interface/ui/AccountList.hpp"
#include "opentxs/interface/ui/AccountSummary.hpp"
#include "opentxs/interface/ui/AccountTree.hpp"
#include "opentxs/interface/ui/ActivitySummary.hpp"
#include "opentxs/interface/ui/ActivityThread.hpp"
#include "opentxs/interface/ui/Blockchains.hpp"
#include "opentxs/interface/ui/Contact.hpp"
#include "opentxs/interface/ui/ContactList.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/interface/ui/MessagableList.hpp"
#include "opentxs/interface/ui/NymList.hpp"
#include "opentxs/interface/ui/PayableList.hpp"
#include "opentxs/interface/ui/Profile.hpp"
#include "opentxs/interface/ui/Types.hpp"
#include "opentxs/interface/ui/UnitList.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

class QAbstractItemModel;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

namespace ui
{
namespace internal
{
struct AccountActivity;
struct AccountList;
struct AccountSummary;
struct AccountTree;
struct ActivitySummary;
struct ActivityThread;
struct BlockchainAccountStatus;
struct BlockchainSelection;
struct BlockchainStatistics;
struct Contact;
struct ContactList;
struct MessagableList;
struct NymList;
struct PayableList;
struct Profile;
struct SeedTree;
struct UnitList;
}  // namespace internal

class AccountActivity;
class AccountActivityQt;
class AccountList;
class AccountListQt;
class AccountSummary;
class AccountSummaryQt;
class AccountTree;
class AccountTreeQt;
class ActivitySummary;
class ActivitySummaryQt;
class ActivityThread;
class ActivityThreadQt;
class BlockchainAccountStatus;
class BlockchainAccountStatusQt;
class BlockchainSelection;
class BlockchainSelectionQt;
class BlockchainStatistics;
class BlockchainStatisticsQt;
class Contact;
class ContactList;
class ContactListQt;
class ContactQt;
class IdentityManagerQt;
class MessagableList;
class MessagableListQt;
class NymList;
class NymListQt;
class PayableList;
class PayableListQt;
class Profile;
class ProfileQt;
class SeedTree;
class SeedTreeQt;
class SeedValidator;
class UnitList;
class UnitListQt;
}  // namespace ui

class Flag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::imp
{
class UI::Imp : public Lockable
{
public:
    auto AccountActivity(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        const SimpleCallback cb) const noexcept
        -> const opentxs::ui::AccountActivity&;
    virtual auto AccountActivityQt(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::AccountActivityQt*
    {
        return nullptr;
    }
    auto AccountList(const identifier::Nym& nym, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::AccountList&;
    virtual auto AccountListQt(
        const identifier::Nym& nym,
        const SimpleCallback cb) const noexcept -> opentxs::ui::AccountListQt*
    {
        return nullptr;
    }
    auto AccountSummary(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback cb) const noexcept
        -> const opentxs::ui::AccountSummary&;
    virtual auto AccountSummaryQt(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::AccountSummaryQt*
    {
        return nullptr;
    }
    auto AccountTree(const identifier::Nym& nym, const SimpleCallback updateCB)
        const noexcept -> const opentxs::ui::AccountTree&;
    virtual auto AccountTreeQt(
        const identifier::Nym& nym,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::AccountTreeQt*
    {
        return nullptr;
    }
    auto ActivateUICallback(const identifier::Generic& widget) const noexcept
        -> void;
    auto ActivitySummary(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::ActivitySummary&;
    virtual auto ActivitySummaryQt(
        const identifier::Nym& nymID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::ActivitySummaryQt*
    {
        return nullptr;
    }
    auto ActivityThread(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const SimpleCallback cb) const noexcept
        -> const opentxs::ui::ActivityThread&;
    virtual auto ActivityThreadQt(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::ActivityThreadQt*
    {
        return nullptr;
    }
    virtual auto BlankModel(const std::size_t columns) const noexcept
        -> QAbstractItemModel*
    {
        return nullptr;
    }
    auto BlockchainAccountStatus(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain,
        const SimpleCallback cb) const noexcept
        -> const opentxs::ui::BlockchainAccountStatus&;
    virtual auto BlockchainAccountStatusQt(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::BlockchainAccountStatusQt*
    {
        return nullptr;
    }
    auto BlockchainIssuerID(const opentxs::blockchain::Type chain)
        const noexcept -> const identifier::Nym&;
    auto BlockchainNotaryID(const opentxs::blockchain::Type chain)
        const noexcept -> const identifier::Notary&;
    auto BlockchainSelection(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::BlockchainSelection&;
    virtual auto BlockchainSelectionQt(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::BlockchainSelectionQt*
    {
        return nullptr;
    }
    auto BlockchainStatistics(const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::BlockchainStatistics&;
    virtual auto BlockchainStatisticsQt(const SimpleCallback updateCB)
        const noexcept -> opentxs::ui::BlockchainStatisticsQt*
    {
        return nullptr;
    }
    auto BlockchainUnitID(const opentxs::blockchain::Type chain) const noexcept
        -> const identifier::UnitDefinition&;
    auto ClearUICallbacks(const identifier::Generic& widget) const noexcept
        -> void;
    auto Contact(const identifier::Generic& contactID, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::Contact&;
    virtual auto ContactQt(
        const identifier::Generic& contactID,
        const SimpleCallback cb) const noexcept -> opentxs::ui::ContactQt*
    {
        return nullptr;
    }
    auto ContactList(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::ContactList&;
    virtual auto ContactListQt(
        const identifier::Nym& nymID,
        const SimpleCallback cb) const noexcept -> opentxs::ui::ContactListQt*
    {
        return nullptr;
    }
    virtual auto IdentityManagerQt() const noexcept
        -> opentxs::ui::IdentityManagerQt*
    {
        return nullptr;
    }
    auto MessagableList(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::MessagableList&;
    virtual auto MessagableListQt(
        const identifier::Nym& nymID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::MessagableListQt*
    {
        return nullptr;
    }
    auto NymList(const SimpleCallback cb) const noexcept
        -> const opentxs::ui::NymList&;
    virtual auto NymListQt(const SimpleCallback cb) const noexcept
        -> opentxs::ui::NymListQt*
    {
        return nullptr;
    }
    auto PayableList(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback cb) const noexcept
        -> const opentxs::ui::PayableList&;
    virtual auto PayableListQt(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback cb) const noexcept -> opentxs::ui::PayableListQt*
    {
        return nullptr;
    }
    auto Profile(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::Profile&;
    virtual auto ProfileQt(
        const identifier::Nym& nymID,
        const SimpleCallback cb) const noexcept -> opentxs::ui::ProfileQt*
    {
        return nullptr;
    }
    auto RegisterUICallback(
        const identifier::Generic& widget,
        const SimpleCallback& cb) const noexcept -> void;
    auto SeedTree(const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::SeedTree&;
    virtual auto SeedTreeQt(const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::SeedTreeQt*
    {
        return nullptr;
    }
    virtual auto SeedValidator(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang) const noexcept
        -> const opentxs::ui::SeedValidator*
    {
        return nullptr;
    }
    auto UnitList(const identifier::Nym& nym, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::UnitList&;
    virtual auto UnitListQt(const identifier::Nym& nym, const SimpleCallback cb)
        const noexcept -> opentxs::ui::UnitListQt*
    {
        return nullptr;
    }

    auto Init() noexcept -> void {}
    auto Shutdown() noexcept -> void;

    Imp(const api::session::Client& api,
        const api::crypto::Blockchain& blockchain,
        const Flag& running) noexcept;
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() override;

protected:
    /** NymID, AccountID */
    using AccountActivityKey = std::pair<identifier::Nym, identifier::Generic>;
    using AccountListKey = identifier::Nym;
    /** NymID, currency*/
    using AccountSummaryKey = std::pair<identifier::Nym, UnitType>;
    using AccountTreeKey = identifier::Nym;
    using ActivitySummaryKey = identifier::Nym;
    using ActivityThreadKey = std::pair<identifier::Nym, identifier::Generic>;
    using BlockchainAccountStatusKey =
        std::pair<identifier::Nym, blockchain::Type>;
    using ContactKey = identifier::Generic;
    using ContactListKey = identifier::Nym;
    using MessagableListKey = identifier::Nym;
    /** NymID, currency*/
    using PayableListKey = std::pair<identifier::Nym, UnitType>;
    using ProfileKey = identifier::Nym;
    using UnitListKey = identifier::Nym;

    using AccountActivityPointer =
        std::unique_ptr<opentxs::ui::internal::AccountActivity>;
    using AccountListPointer =
        std::unique_ptr<opentxs::ui::internal::AccountList>;
    using AccountSummaryPointer =
        std::unique_ptr<opentxs::ui::internal::AccountSummary>;
    using AccountTreePointer =
        std::unique_ptr<opentxs::ui::internal::AccountTree>;
    using ActivitySummaryPointer =
        std::unique_ptr<opentxs::ui::internal::ActivitySummary>;
    using ActivityThreadPointer =
        std::unique_ptr<opentxs::ui::internal::ActivityThread>;
    using BlockchainAccountStatusPointer =
        std::unique_ptr<opentxs::ui::internal::BlockchainAccountStatus>;
    using BlockchainSelectionPointer =
        std::unique_ptr<opentxs::ui::internal::BlockchainSelection>;
    using BlockchainStatisticsPointer =
        std::unique_ptr<opentxs::ui::internal::BlockchainStatistics>;
    using ContactListPointer =
        std::unique_ptr<opentxs::ui::internal::ContactList>;
    using ContactPointer = std::unique_ptr<opentxs::ui::internal::Contact>;
    using MessagableListPointer =
        std::unique_ptr<opentxs::ui::internal::MessagableList>;
    using NymListPointer = std::unique_ptr<opentxs::ui::internal::NymList>;
    using PayableListPointer =
        std::unique_ptr<opentxs::ui::internal::PayableList>;
    using ProfilePointer = std::unique_ptr<opentxs::ui::internal::Profile>;
    using SeedTreePointer = std::unique_ptr<opentxs::ui::internal::SeedTree>;
    using UnitListPointer = std::unique_ptr<opentxs::ui::internal::UnitList>;

    using AccountActivityMap =
        UnallocatedMap<AccountActivityKey, AccountActivityPointer>;
    using AccountListMap = UnallocatedMap<AccountListKey, AccountListPointer>;
    using AccountSummaryMap =
        UnallocatedMap<AccountSummaryKey, AccountSummaryPointer>;
    using AccountTreeMap = UnallocatedMap<AccountTreeKey, AccountTreePointer>;
    using ActivitySummaryMap =
        UnallocatedMap<ActivitySummaryKey, ActivitySummaryPointer>;
    using ActivityThreadMap =
        UnallocatedMap<ActivityThreadKey, ActivityThreadPointer>;
    using BlockchainAccountStatusMap = UnallocatedMap<
        BlockchainAccountStatusKey,
        BlockchainAccountStatusPointer>;
    using BlockchainSelectionMap =
        UnallocatedMap<opentxs::ui::Blockchains, BlockchainSelectionPointer>;
    using ContactListMap = UnallocatedMap<ContactListKey, ContactListPointer>;
    using ContactMap = UnallocatedMap<ContactKey, ContactPointer>;
    using MessagableListMap =
        UnallocatedMap<MessagableListKey, MessagableListPointer>;
    using PayableListMap = UnallocatedMap<PayableListKey, PayableListPointer>;
    using ProfileMap = UnallocatedMap<ProfileKey, ProfilePointer>;
    using UnitListMap = UnallocatedMap<UnitListKey, UnitListPointer>;

    const api::session::Client& api_;
    const api::crypto::Blockchain& blockchain_;
    const Flag& running_;
    mutable AccountActivityMap accounts_;
    mutable AccountListMap account_lists_;
    mutable AccountSummaryMap account_summaries_;
    mutable AccountTreeMap account_trees_;
    mutable ActivitySummaryMap activity_summaries_;
    mutable ActivityThreadMap activity_threads_;
    mutable BlockchainAccountStatusMap blockchain_account_status_;
    mutable BlockchainSelectionMap blockchain_selection_;
    mutable BlockchainStatisticsPointer blockchain_statistics_;
    mutable ContactListMap contact_lists_;
    mutable ContactMap contacts_;
    mutable MessagableListMap messagable_lists_;
    mutable NymListPointer nym_list_;
    mutable PayableListMap payable_lists_;
    mutable ProfileMap profiles_;
    mutable SeedTreePointer seed_tree_;
    mutable UnitListMap unit_lists_;
    ui::UpdateManager update_manager_;

    auto account_activity(
        const Lock& lock,
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        const SimpleCallback& cb) const noexcept
        -> AccountActivityMap::mapped_type&;
    auto account_list(
        const Lock& lock,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) const noexcept
        -> AccountListMap::mapped_type&;
    auto account_summary(
        const Lock& lock,
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback& cb) const noexcept
        -> AccountSummaryMap::mapped_type&;
    auto account_tree(
        const Lock& lock,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) const noexcept
        -> AccountTreeMap::mapped_type&;
    auto activity_summary(
        const Lock& lock,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) const noexcept
        -> ActivitySummaryMap::mapped_type&;
    auto activity_thread(
        const Lock& lock,
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const SimpleCallback& cb) const noexcept
        -> ActivityThreadMap::mapped_type&;
    auto contact_list(
        const Lock& lock,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) const noexcept
        -> ContactListMap::mapped_type&;
    auto blockchain_selection(
        const Lock& lock,
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB) const noexcept
        -> BlockchainSelectionMap::mapped_type&;
    auto blockchain_statistics(const Lock& lock, const SimpleCallback updateCB)
        const noexcept -> BlockchainStatisticsPointer&;
    auto contact(
        const Lock& lock,
        const identifier::Generic& contactID,
        const SimpleCallback& cb) const noexcept -> ContactMap::mapped_type&;
    auto blockchain_account_status(
        const Lock& lock,
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain,
        const SimpleCallback& cb) const noexcept
        -> BlockchainAccountStatusMap::mapped_type&;
    auto is_blockchain_account(const identifier::Generic& id) const noexcept
        -> std::optional<opentxs::blockchain::Type>;
    auto messagable_list(
        const Lock& lock,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) const noexcept
        -> MessagableListMap::mapped_type&;
    auto nym_list(const Lock& lock, const SimpleCallback& cb) const noexcept
        -> opentxs::ui::internal::NymList&;
    auto payable_list(
        const Lock& lock,
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback& cb) const noexcept
        -> PayableListMap::mapped_type&;
    auto profile(
        const Lock& lock,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) const noexcept -> ProfileMap::mapped_type&;
    auto seed_tree(const Lock& lock, const SimpleCallback& cb) const noexcept
        -> opentxs::ui::internal::SeedTree&;
    auto unit_list(
        const Lock& lock,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) const noexcept -> UnitListMap::mapped_type&;

    auto ShutdownCallbacks() noexcept -> void;
    virtual auto ShutdownModels() noexcept -> void;
};
}  // namespace opentxs::api::session::imp
