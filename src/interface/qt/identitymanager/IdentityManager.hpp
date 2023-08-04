// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_ordered_guarded.h>
#include <QString>
#include <shared_mutex>

#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/interface/qt/IdentityManager.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace ui
{
class AccountActivityQt;
class AccountListQt;
class AccountTreeQt;
class BlockchainAccountStatusQt;
class ContactActivityQt;
class ContactActivityQtFilterable;
class ContactListQt;
class NymListQt;
class ProfileQt;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class QAbstractListModel;

class opentxs::ui::IdentityManagerQt::Imp
{
public:
    auto getAccountActivity(const QString& accountID) const noexcept
        -> AccountActivityQt*;
    auto getAccountList() const noexcept -> AccountListQt*;
    auto getAccountStatus(const QString& accountID) const noexcept
        -> BlockchainAccountStatusQt*;
    auto getAccountTree() const noexcept -> AccountTreeQt*;
    auto getActiveNym() const noexcept -> QString;
    auto getContactActivity(const QString& contactID) const noexcept
        -> ContactActivityQt*;
    auto getContactActivityFilterable(const QString& contactID) const noexcept
        -> ContactActivityQtFilterable*;
    auto getContactList() const noexcept -> ContactListQt*;
    auto getNymList() const noexcept -> NymListQt*;
    auto getNymType() const noexcept -> QAbstractListModel*;
    auto getProfile() const noexcept -> ProfileQt*;

    auto init(IdentityManagerQt* parent) noexcept -> void;
    auto setActiveNym(QString) noexcept -> void;

    Imp(const api::session::Client& api) noexcept;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

private:
    const api::session::Client& api_;
    IdentityManagerQt* parent_;
    libguarded::ordered_guarded<identifier::Nym, std::shared_mutex> active_nym_;
};
