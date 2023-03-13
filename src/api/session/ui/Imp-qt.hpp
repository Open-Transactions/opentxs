// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>
#include <mutex>

#include "api/session/ui/Imp-base.hpp"
#include "api/session/ui/UI.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/interface/qt/AccountActivity.hpp"
#include "opentxs/interface/qt/AccountList.hpp"
#include "opentxs/interface/qt/AccountSummary.hpp"
#include "opentxs/interface/qt/AccountTree.hpp"
#include "opentxs/interface/qt/ActivitySummary.hpp"
#include "opentxs/interface/qt/ActivityThread.hpp"
#include "opentxs/interface/qt/BlankModel.hpp"
#include "opentxs/interface/qt/BlockchainAccountStatus.hpp"
#include "opentxs/interface/qt/BlockchainSelection.hpp"
#include "opentxs/interface/qt/BlockchainStatistics.hpp"
#include "opentxs/interface/qt/Contact.hpp"
#include "opentxs/interface/qt/ContactList.hpp"
#include "opentxs/interface/qt/IdentityManager.hpp"
#include "opentxs/interface/qt/MessagableList.hpp"
#include "opentxs/interface/qt/NymList.hpp"
#include "opentxs/interface/qt/PayableList.hpp"
#include "opentxs/interface/qt/Profile.hpp"
#include "opentxs/interface/qt/SeedList.hpp"
#include "opentxs/interface/qt/SeedTree.hpp"
#include "opentxs/interface/qt/SeedValidator.hpp"
#include "opentxs/interface/qt/UnitList.hpp"
#include "opentxs/interface/ui/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

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
class Generic;
class Nym;
}  // namespace identifier

class Flag;
}  // namespace opentxs

class QAbstractItemModel;
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::ui
{
class ImpQt final : public session::imp::UI::Imp
{
public:
    auto AccountActivityQt(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::AccountActivityQt* final;
    auto AccountListQt(const identifier::Nym& nym, const SimpleCallback cb)
        const noexcept -> opentxs::ui::AccountListQt* final;
    auto AccountSummaryQt(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::AccountSummaryQt* final;
    auto AccountTreeQt(const identifier::Nym& nym, const SimpleCallback cb)
        const noexcept -> opentxs::ui::AccountTreeQt* final;
    auto ActivitySummaryQt(
        const identifier::Nym& nymID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::ActivitySummaryQt* final;
    auto ActivityThreadQt(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::ActivityThreadQt* final;
    auto BlankModel(const std::size_t columns) const noexcept
        -> QAbstractItemModel* final;
    auto BlockchainAccountStatusQt(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::BlockchainAccountStatusQt* final;
    auto BlockchainSelectionQt(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::BlockchainSelectionQt* final;
    auto BlockchainStatisticsQt(const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::BlockchainStatisticsQt* final;
    auto ContactQt(
        const identifier::Generic& contactID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::ContactQt* final;
    auto ContactListQt(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> opentxs::ui::ContactListQt* final;
    auto IdentityManagerQt() const noexcept
        -> opentxs::ui::IdentityManagerQt* final
    {
        return &identity_manager_;
    }
    auto MessagableListQt(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> opentxs::ui::MessagableListQt* final;
    auto NymListQt(const SimpleCallback cb) const noexcept
        -> opentxs::ui::NymListQt* final;
    auto PayableListQt(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::PayableListQt* final;
    auto ProfileQt(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> opentxs::ui::ProfileQt* final;
    auto SeedListQt(const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::SeedListQt* final;
    auto SeedTreeQt(const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::SeedTreeQt* final;
    auto SeedValidator(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang) const noexcept
        -> const opentxs::ui::SeedValidator* final;
    auto UnitListQt(const identifier::Nym& nym, const SimpleCallback cb)
        const noexcept -> opentxs::ui::UnitListQt* final;

    auto ShutdownModels() noexcept -> void final;

    ImpQt(
        const api::session::Client& api,
        const api::crypto::Blockchain& blockchain,
        const Flag& running) noexcept;
    ImpQt() = delete;
    ImpQt(const ImpQt&) = delete;
    ImpQt(ImpQt&&) = delete;
    auto operator=(const ImpQt&) -> ImpQt& = delete;
    auto operator=(ImpQt&&) -> ImpQt& = delete;

    ~ImpQt() final;

private:
    using AccountActivityQtPointer =
        std::unique_ptr<opentxs::ui::AccountActivityQt>;
    using AccountListQtPointer = std::unique_ptr<opentxs::ui::AccountListQt>;
    using AccountSummaryQtPointer =
        std::unique_ptr<opentxs::ui::AccountSummaryQt>;
    using AccountTreeQtPointer = std::unique_ptr<opentxs::ui::AccountTreeQt>;
    using ActivitySummaryQtPointer =
        std::unique_ptr<opentxs::ui::ActivitySummaryQt>;
    using ActivityThreadQtPointer =
        std::unique_ptr<opentxs::ui::ActivityThreadQt>;
    using BlockchainAccountStatusQtPointer =
        std::unique_ptr<opentxs::ui::BlockchainAccountStatusQt>;
    using BlockchainSelectionQtPointer =
        std::unique_ptr<opentxs::ui::BlockchainSelectionQt>;
    using BlockchainStatisticsQtPointer =
        std::unique_ptr<opentxs::ui::BlockchainStatisticsQt>;
    using ContactListQtPointer = std::unique_ptr<opentxs::ui::ContactListQt>;
    using ContactQtPointer = std::unique_ptr<opentxs::ui::ContactQt>;
    using MessagableListQtPointer =
        std::unique_ptr<opentxs::ui::MessagableListQt>;
    using NymListQtPointer = std::unique_ptr<opentxs::ui::NymListQt>;
    using PayableListQtPointer = std::unique_ptr<opentxs::ui::PayableListQt>;
    using ProfileQtPointer = std::unique_ptr<opentxs::ui::ProfileQt>;
    using SeedListQtPointer = std::unique_ptr<opentxs::ui::SeedListQt>;
    using SeedTreeQtPointer = std::unique_ptr<opentxs::ui::SeedTreeQt>;
    using UnitListQtPointer = std::unique_ptr<opentxs::ui::UnitListQt>;

    using AccountActivityQtMap =
        UnallocatedMap<AccountActivityKey, AccountActivityQtPointer>;
    using AccountListQtMap =
        UnallocatedMap<AccountListKey, AccountListQtPointer>;
    using AccountSummaryQtMap =
        UnallocatedMap<AccountSummaryKey, AccountSummaryQtPointer>;
    using AccountTreeQtMap =
        UnallocatedMap<AccountTreeKey, AccountTreeQtPointer>;
    using ActivitySummaryQtMap =
        UnallocatedMap<ActivitySummaryKey, ActivitySummaryQtPointer>;
    using ActivityThreadQtMap =
        UnallocatedMap<ActivityThreadKey, ActivityThreadQtPointer>;
    using BlockchainAccountStatusQtMap = UnallocatedMap<
        BlockchainAccountStatusKey,
        BlockchainAccountStatusQtPointer>;
    using BlockchainSelectionQtMap =
        UnallocatedMap<opentxs::ui::Blockchains, BlockchainSelectionQtPointer>;
    using ContactListQtMap =
        UnallocatedMap<ContactListKey, ContactListQtPointer>;
    using ContactQtMap = UnallocatedMap<ContactKey, ContactQtPointer>;
    using MessagableListQtMap =
        UnallocatedMap<MessagableListKey, MessagableListQtPointer>;
    using PayableListQtMap =
        UnallocatedMap<PayableListKey, PayableListQtPointer>;
    using ProfileQtMap = UnallocatedMap<ProfileKey, ProfileQtPointer>;
    using SeedValidatorMap = UnallocatedMap<
        opentxs::crypto::SeedStyle,
        UnallocatedMap<opentxs::crypto::Language, opentxs::ui::SeedValidator>>;
    using UnitListQtMap = UnallocatedMap<UnitListKey, UnitListQtPointer>;

    struct Blank {
        auto get(const std::size_t columns) noexcept
            -> opentxs::ui::BlankModel*;

    private:
        std::mutex lock_{};
        UnallocatedMap<std::size_t, opentxs::ui::BlankModel> map_{};
    };

    mutable Blank blank_;
    mutable opentxs::ui::IdentityManagerQt identity_manager_;
    mutable AccountActivityQtMap accounts_qt_;
    mutable AccountListQtMap account_lists_qt_;
    mutable AccountSummaryQtMap account_summaries_qt_;
    mutable AccountTreeQtMap account_trees_qt_;
    mutable ActivitySummaryQtMap activity_summaries_qt_;
    mutable ActivityThreadQtMap activity_threads_qt_;
    mutable BlockchainAccountStatusQtMap blockchain_account_status_qt_;
    mutable BlockchainSelectionQtMap blockchain_selection_qt_;
    mutable BlockchainStatisticsQtPointer blockchain_statistics_qt_;
    mutable ContactListQtMap contact_lists_qt_;
    mutable ContactQtMap contacts_qt_;
    mutable MessagableListQtMap messagable_lists_qt_;
    mutable NymListQtPointer nym_list_qt_;
    mutable PayableListQtMap payable_lists_qt_;
    mutable ProfileQtMap profiles_qt_;
    mutable SeedListQtPointer seed_list_qt_;
    mutable SeedTreeQtPointer seed_tree_qt_;
    mutable SeedValidatorMap seed_validators_;
    mutable UnitListQtMap unit_lists_qt_;
};
}  // namespace opentxs::api::session::ui
