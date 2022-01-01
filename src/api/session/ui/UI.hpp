// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <cstddef>
#include <memory>

#include "internal/api/session/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountList.hpp"
#include "opentxs/ui/AccountSummary.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/BlockchainAccountStatus.hpp"
#include "opentxs/ui/BlockchainSelection.hpp"
#include "opentxs/ui/BlockchainStatistics.hpp"
#include "opentxs/ui/Blockchains.hpp"
#include "opentxs/ui/Contact.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/MessagableList.hpp"
#include "opentxs/ui/PayableList.hpp"
#include "opentxs/ui/Profile.hpp"
#include "opentxs/ui/UnitList.hpp"

class QAbstractItemModel;

namespace opentxs
{
namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

namespace ui
{
class AccountActivity;
class AccountActivityQt;
class AccountList;
class AccountListQt;
class AccountSummary;
class AccountSummaryQt;
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
class MessagableList;
class MessagableListQt;
class PayableList;
class PayableListQt;
class Profile;
class ProfileQt;
class SeedValidator;
class UnitList;
class UnitListQt;
}  // namespace ui

class Identifier;
}  // namespace opentxs

namespace opentxs::api::session::imp
{
class UI final : public internal::UI
{
public:
    class Imp;

    auto AccountActivity(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::AccountActivity& final;
    auto AccountActivityQt(
        const identifier::Nym& nymID,
        const Identifier& accountID,
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
        const core::UnitType currency,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::AccountSummary& final;
    auto AccountSummaryQt(
        const identifier::Nym& nymID,
        const core::UnitType currency,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::AccountSummaryQt* final;
    auto ActivateUICallback(const Identifier& widget) const noexcept
        -> void final;
    auto ActivitySummary(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::ActivitySummary& final;
    auto ActivitySummaryQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::ActivitySummaryQt* final;
    auto ActivityThread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::ActivityThread& final;
    auto ActivityThreadQt(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::ActivityThreadQt* final;
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
        const noexcept -> const identifier::Server& final;
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
    auto ClearUICallbacks(const Identifier& widget) const noexcept
        -> void final;
    auto Contact(const Identifier& contactID, const SimpleCallback updateCB)
        const noexcept -> const opentxs::ui::Contact& final;
    auto ContactQt(const Identifier& contactID, const SimpleCallback updateCB)
        const noexcept -> opentxs::ui::ContactQt* final;
    auto ContactList(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::ContactList& final;
    auto ContactListQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::ContactListQt* final;
    auto MessagableList(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::MessagableList& final;
    auto MessagableListQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::MessagableListQt* final;
    auto PayableList(
        const identifier::Nym& nymID,
        const core::UnitType currency,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::PayableList& final;
    auto PayableListQt(
        const identifier::Nym& nymID,
        const core::UnitType currency,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::PayableListQt* final;
    auto Profile(const identifier::Nym& nymID, const SimpleCallback updateCB)
        const noexcept -> const opentxs::ui::Profile& final;
    auto ProfileQt(const identifier::Nym& nymID, const SimpleCallback updateCB)
        const noexcept -> opentxs::ui::ProfileQt* final;
    auto RegisterUICallback(const Identifier& widget, const SimpleCallback& cb)
        const noexcept -> void final;
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

    ~UI() final;

private:
    std::unique_ptr<Imp> imp_;

    UI() = delete;
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    auto operator=(const UI&) -> UI& = delete;
    auto operator=(UI&&) -> UI& = delete;
};
}  // namespace opentxs::api::session::imp
