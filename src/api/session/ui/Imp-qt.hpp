// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <cstddef>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>

#include "api/session/ui/Imp-base.hpp"
#include "api/session/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/ui/Blockchains.hpp"
#include "opentxs/ui/qt/AccountActivity.hpp"
#include "opentxs/ui/qt/AccountList.hpp"
#include "opentxs/ui/qt/AccountSummary.hpp"
#include "opentxs/ui/qt/ActivitySummary.hpp"
#include "opentxs/ui/qt/ActivityThread.hpp"
#include "opentxs/ui/qt/BlankModel.hpp"
#include "opentxs/ui/qt/BlockchainAccountStatus.hpp"
#include "opentxs/ui/qt/BlockchainSelection.hpp"
#include "opentxs/ui/qt/BlockchainStatistics.hpp"
#include "opentxs/ui/qt/Contact.hpp"
#include "opentxs/ui/qt/ContactList.hpp"
#include "opentxs/ui/qt/MessagableList.hpp"
#include "opentxs/ui/qt/PayableList.hpp"
#include "opentxs/ui/qt/Profile.hpp"
#include "opentxs/ui/qt/SeedValidator.hpp"
#include "opentxs/ui/qt/UnitList.hpp"

class QAbstractItemModel;

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
class Nym;
}  // namespace identifier

namespace ui
{
struct BlankModel;
}  // namespace ui

class Flag;
class Identifier;
}  // namespace opentxs

namespace opentxs::api::session::ui
{
class ImpQt final : public session::imp::UI::Imp
{
public:
    auto AccountActivityQt(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::AccountActivityQt* final;
    auto AccountListQt(const identifier::Nym& nym, const SimpleCallback cb)
        const noexcept -> opentxs::ui::AccountListQt* final;
    auto AccountSummaryQt(
        const identifier::Nym& nymID,
        const core::UnitType currency,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::AccountSummaryQt* final;
    auto ActivitySummaryQt(
        const identifier::Nym& nymID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::ActivitySummaryQt* final;
    auto ActivityThreadQt(
        const identifier::Nym& nymID,
        const Identifier& threadID,
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
    auto ContactQt(const Identifier& contactID, const SimpleCallback cb)
        const noexcept -> opentxs::ui::ContactQt* final;
    auto ContactListQt(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> opentxs::ui::ContactListQt* final;
    auto MessagableListQt(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> opentxs::ui::MessagableListQt* final;
    auto PayableListQt(
        const identifier::Nym& nymID,
        const core::UnitType currency,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::PayableListQt* final;
    auto ProfileQt(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> opentxs::ui::ProfileQt* final;
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

    ~ImpQt() final;

private:
    using AccountActivityQtPointer =
        std::unique_ptr<opentxs::ui::AccountActivityQt>;
    using AccountListQtPointer = std::unique_ptr<opentxs::ui::AccountListQt>;
    using AccountSummaryQtPointer =
        std::unique_ptr<opentxs::ui::AccountSummaryQt>;
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
    using PayableListQtPointer = std::unique_ptr<opentxs::ui::PayableListQt>;
    using ProfileQtPointer = std::unique_ptr<opentxs::ui::ProfileQt>;
    using UnitListQtPointer = std::unique_ptr<opentxs::ui::UnitListQt>;

    using AccountActivityQtMap =
        std::pmr::map<AccountActivityKey, AccountActivityQtPointer>;
    using AccountListQtMap =
        std::pmr::map<AccountListKey, AccountListQtPointer>;
    using AccountSummaryQtMap =
        std::pmr::map<AccountSummaryKey, AccountSummaryQtPointer>;
    using ActivitySummaryQtMap =
        std::pmr::map<ActivitySummaryKey, ActivitySummaryQtPointer>;
    using ActivityThreadQtMap =
        std::pmr::map<ActivityThreadKey, ActivityThreadQtPointer>;
    using BlockchainAccountStatusQtMap = std::pmr::
        map<BlockchainAccountStatusKey, BlockchainAccountStatusQtPointer>;
    using BlockchainSelectionQtMap =
        std::pmr::map<opentxs::ui::Blockchains, BlockchainSelectionQtPointer>;
    using ContactListQtMap =
        std::pmr::map<ContactListKey, ContactListQtPointer>;
    using ContactQtMap = std::pmr::map<ContactKey, ContactQtPointer>;
    using MessagableListQtMap =
        std::pmr::map<MessagableListKey, MessagableListQtPointer>;
    using PayableListQtMap =
        std::pmr::map<PayableListKey, PayableListQtPointer>;
    using ProfileQtMap = std::pmr::map<ProfileKey, ProfileQtPointer>;
    using SeedValidatorMap = std::pmr::map<
        opentxs::crypto::SeedStyle,
        std::pmr::map<opentxs::crypto::Language, opentxs::ui::SeedValidator>>;
    using UnitListQtMap = std::pmr::map<UnitListKey, UnitListQtPointer>;

    struct Blank {
        auto get(const std::size_t columns) noexcept
            -> opentxs::ui::BlankModel*;

    private:
        std::mutex lock_{};
        std::pmr::map<std::size_t, opentxs::ui::BlankModel> map_{};
    };

    mutable Blank blank_;
    mutable AccountActivityQtMap accounts_qt_;
    mutable AccountListQtMap account_lists_qt_;
    mutable AccountSummaryQtMap account_summaries_qt_;
    mutable ActivitySummaryQtMap activity_summaries_qt_;
    mutable ActivityThreadQtMap activity_threads_qt_;
    mutable BlockchainAccountStatusQtMap blockchain_account_status_qt_;
    mutable BlockchainSelectionQtMap blockchain_selection_qt_;
    mutable BlockchainStatisticsQtPointer blockchain_statistics_qt_;
    mutable ContactListQtMap contact_lists_qt_;
    mutable ContactQtMap contacts_qt_;
    mutable MessagableListQtMap messagable_lists_qt_;
    mutable PayableListQtMap payable_lists_qt_;
    mutable ProfileQtMap profiles_qt_;
    mutable SeedValidatorMap seed_validators_;
    mutable UnitListQtMap unit_lists_qt_;

    ImpQt() = delete;
    ImpQt(const ImpQt&) = delete;
    ImpQt(ImpQt&&) = delete;
    auto operator=(const ImpQt&) -> ImpQt& = delete;
    auto operator=(ImpQt&&) -> ImpQt& = delete;
};
}  // namespace opentxs::api::session::ui
