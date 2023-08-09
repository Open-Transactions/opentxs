// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>

#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/AccountActivity.hpp"
#include "internal/interface/ui/AccountList.hpp"
#include "internal/interface/ui/AccountSummary.hpp"
#include "internal/interface/ui/AccountTree.hpp"
#include "internal/interface/ui/ActivitySummary.hpp"
#include "internal/interface/ui/BlockchainAccountStatus.hpp"
#include "internal/interface/ui/BlockchainSelection.hpp"
#include "internal/interface/ui/BlockchainStatistics.hpp"
#include "internal/interface/ui/Contact.hpp"
#include "internal/interface/ui/ContactActivity.hpp"
#include "internal/interface/ui/ContactList.hpp"
#include "internal/interface/ui/MessagableList.hpp"
#include "internal/interface/ui/NymList.hpp"
#include "internal/interface/ui/PayableList.hpp"
#include "internal/interface/ui/Profile.hpp"
#include "internal/interface/ui/SeedList.hpp"
#include "internal/interface/ui/SeedTree.hpp"
#include "internal/interface/ui/UnitList.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/interface/ui/Types.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier

namespace ui
{
class AccountActivityQt;
class AccountListQt;
class AccountSummaryQt;
class AccountTreeQt;
class ActivitySummaryQt;
class BlockchainAccountStatusQt;
class BlockchainSelectionQt;
class BlockchainStatisticsQt;
class ContactActivityQt;
class ContactActivityQtFilterable;
class ContactListQt;
class ContactQt;
class IdentityManagerQt;
class MessagableListQt;
class NymListQt;
class NymType;
class PayableListQt;
class ProfileQt;
class SeedListQt;
class SeedTreeQt;
class SeedValidator;
class UnitListQt;
}  // namespace ui
}  // namespace opentxs

class QAbstractItemModel;
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::imp
{
class UI final : public internal::UI
{
public:
    class Imp;

    auto AccountActivity(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::AccountActivity& final;
    auto AccountActivityQt(
        const identifier::Nym& nymID,
        const identifier::Account& accountID,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::AccountActivityQt* final;
    auto AccountList(const identifier::Nym& nym, const SimpleCallback updateCB)
        const noexcept -> const opentxs::ui::AccountList& final;
    auto AccountListQt(
        const identifier::Nym& nym,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::AccountListQt* final;
    auto AccountSummary(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::AccountSummary& final;
    auto AccountSummaryQt(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::AccountSummaryQt* final;
    auto AccountTree(const identifier::Nym& nym, const SimpleCallback updateCB)
        const noexcept -> const opentxs::ui::AccountTree& final;
    auto AccountTreeQt(
        const identifier::Nym& nym,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::AccountTreeQt* final;
    auto ActivateUICallback(const identifier::Generic& widget) const noexcept
        -> void final;
    auto ActivitySummary(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::ActivitySummary& final;
    auto ActivitySummaryQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::ActivitySummaryQt* final;
    auto BlankModel(const std::size_t columns) const noexcept
        -> QAbstractItemModel* final;
    auto BlockchainAccountStatus(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::BlockchainAccountStatus& final;
    auto BlockchainAccountStatusQt(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::BlockchainAccountStatusQt* final;
    auto BlockchainIssuerID(const opentxs::blockchain::Type chain)
        const noexcept -> const identifier::Nym& final;
    auto BlockchainNotaryID(const opentxs::blockchain::Type chain)
        const noexcept -> const identifier::Notary& final;
    auto BlockchainSelection(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::BlockchainSelection& final;
    auto BlockchainSelectionQt(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::BlockchainSelectionQt* final;
    auto BlockchainStatistics(const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::BlockchainStatistics& final;
    auto BlockchainStatisticsQt(const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::BlockchainStatisticsQt* final;
    auto BlockchainUnitID(const opentxs::blockchain::Type chain) const noexcept
        -> const identifier::UnitDefinition& final;
    auto ClearUICallbacks(const identifier::Generic& widget) const noexcept
        -> void final;
    auto Contact(
        const identifier::Generic& contactID,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::Contact& final;
    auto ContactQt(
        const identifier::Generic& contactID,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::ContactQt* final;
    auto ContactActivity(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::ContactActivity& final;
    auto ContactActivityQt(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::ContactActivityQt* final;
    auto ContactActivityQtFilterable(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::ContactActivityQtFilterable* final;
    auto ContactList(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::ContactList& final;
    auto ContactListQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::ContactListQt* final;
    auto IdentityManagerQt() const noexcept
        -> opentxs::ui::IdentityManagerQt* final;
    auto MessagableList(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::MessagableList& final;
    auto MessagableListQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::MessagableListQt* final;
    auto NymList(const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::NymList& final;
    auto NymListQt(const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::NymListQt* final;
    auto NymType() const noexcept -> opentxs::ui::NymType* final;
    auto PayableList(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::PayableList& final;
    auto PayableListQt(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::PayableListQt* final;
    auto Profile(const identifier::Nym& nymID, const SimpleCallback updateCB)
        const noexcept -> const opentxs::ui::Profile& final;
    auto ProfileQt(const identifier::Nym& nymID, const SimpleCallback updateCB)
        const noexcept -> opentxs::ui::ProfileQt* final;
    auto RegisterUICallback(
        const identifier::Generic& widget,
        const SimpleCallback& cb) const noexcept -> void final;
    auto SeedList(const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::SeedList& final;
    auto SeedListQt(const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::SeedListQt* final;
    auto SeedTree(const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::SeedTree& final;
    auto SeedTreeQt(const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::SeedTreeQt* final;
    auto SeedValidator(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang) const noexcept
        -> const opentxs::ui::SeedValidator* final;
    auto UnitList(const identifier::Nym& nym, const SimpleCallback updateCB)
        const noexcept -> const opentxs::ui::UnitList& final;
    auto UnitListQt(const identifier::Nym& nym, const SimpleCallback updateCB)
        const noexcept -> opentxs::ui::UnitListQt* final;

    auto Init() noexcept -> void final;
    auto Shutdown() noexcept -> void final;

    UI(std::unique_ptr<Imp> imp) noexcept;
    UI() = delete;
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    auto operator=(const UI&) -> UI& = delete;
    auto operator=(UI&&) -> UI& = delete;

    ~UI() final;

private:
    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::api::session::imp
