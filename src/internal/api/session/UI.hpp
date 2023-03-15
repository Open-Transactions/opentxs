// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/session/UI.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier

namespace ui
{
class AccountActivity;
class AccountList;
class AccountSummary;
class AccountTree;
class ActivitySummary;
class ActivityThread;
class BlockchainAccountStatus;
class BlockchainSelection;
class BlockchainStatistics;
class Contact;
class ContactList;
class MessagableList;
class NymList;
class PayableList;
class Profile;
class SeedList;
class SeedTree;
class SeedValidator;
class UnitList;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::internal
{
class UI : virtual public opentxs::api::session::UI
{
public:
    virtual auto AccountActivity(
        const identifier::Nym& nymID,
        const identifier::Generic& accountID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::AccountActivity& = 0;
    virtual auto AccountList(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::AccountList& = 0;
    virtual auto AccountSummary(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::AccountSummary& = 0;
    virtual auto AccountTree(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::AccountTree& = 0;
    virtual auto ActivitySummary(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::ActivitySummary& = 0;
    virtual auto ActivityThread(
        const identifier::Nym& nymID,
        const identifier::Generic& threadID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::ActivityThread& = 0;
    virtual auto ActivateUICallback(
        const identifier::Generic& widget) const noexcept -> void = 0;
    virtual auto BlockchainAccountStatus(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::BlockchainAccountStatus& = 0;
    virtual auto BlockchainSelection(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::BlockchainSelection& = 0;
    virtual auto BlockchainStatistics(const SimpleCallback updateCB = {})
        const noexcept -> const opentxs::ui::BlockchainStatistics& = 0;
    virtual auto ClearUICallbacks(
        const identifier::Generic& widget) const noexcept -> void = 0;
    virtual auto Contact(
        const identifier::Generic& contactID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::Contact& = 0;
    virtual auto ContactList(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::ContactList& = 0;
    auto Internal() const noexcept -> const internal::UI& final
    {
        return *this;
    }
    virtual auto MessagableList(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::MessagableList& = 0;
    virtual auto NymList(const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::NymList& = 0;
    virtual auto PayableList(
        const identifier::Nym& nymID,
        const UnitType currency,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::PayableList& = 0;
    virtual auto Profile(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::Profile& = 0;
    virtual auto RegisterUICallback(
        const identifier::Generic& widget,
        const SimpleCallback& cb) const noexcept -> void = 0;
    virtual auto SeedList(const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::SeedList& = 0;
    virtual auto SeedTree(const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::SeedTree& = 0;
    virtual auto UnitList(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::UnitList& = 0;

    virtual auto Init() noexcept -> void = 0;
    auto Internal() noexcept -> internal::UI& final { return *this; }
    virtual auto Shutdown() noexcept -> void = 0;

    UI(const UI&) = delete;
    UI(UI&&) = delete;
    auto operator=(const UI&) -> UI& = delete;
    auto operator=(UI&&) -> UI& = delete;

    ~UI() override = default;

protected:
    UI() = default;
};
}  // namespace opentxs::api::session::internal
