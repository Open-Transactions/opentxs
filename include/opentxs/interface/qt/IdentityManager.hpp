// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QObject>
#include <QString>

#include "opentxs/Export.hpp"  // IWYU pragma: keep

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class AccountActivityQt;
class AccountListQt;
class AccountTreeQt;
class BlockchainAccountStatusQt;
class ContactActivityQt;
class ContactListQt;
class NymListQt;
class ProfileQt;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class QAbstractListModel;

namespace opentxs::ui
{
class OPENTXS_EXPORT IdentityManagerQt : public QObject
{
    Q_OBJECT
    Q_PROPERTY(
        QObject* accountList READ getAccountListQML NOTIFY activeNymChanged)
    Q_PROPERTY(QString activeNym READ getActiveNym WRITE setActiveNym NOTIFY
                   activeNymChanged USER true)
    Q_PROPERTY(
        QObject* contactList READ getContactListQML NOTIFY activeNymChanged)
    Q_PROPERTY(QObject* nymList READ getNymListQML CONSTANT)
    Q_PROPERTY(QObject* nymTypet READ getNymTypeQML CONSTANT)
    Q_PROPERTY(QObject* profile READ getProfileQML NOTIFY activeNymChanged)

Q_SIGNALS:
    void activeNymChanged(QString) const;
    void needNym() const;

public Q_SLOTS:
    void setActiveNym(QString) const;

public:
    // NOLINTBEGIN(modernize-use-trailing-return-type)
    Q_INVOKABLE QObject* getAccountActivityQML(
        const QString& accountID) const noexcept;
    Q_INVOKABLE QObject* getAccountListQML() const noexcept;
    Q_INVOKABLE QObject* getAccountStatusQML(
        const QString& accountID) const noexcept;
    Q_INVOKABLE QObject* getAccountTreeQML() const noexcept;
    Q_INVOKABLE QObject* getContactActivityQML(
        const QString& contactID) const noexcept;
    Q_INVOKABLE QObject* getContactListQML() const noexcept;
    Q_INVOKABLE QObject* getNymListQML() const noexcept;
    Q_INVOKABLE QObject* getNymTypeQML() const noexcept;
    Q_INVOKABLE QObject* getProfileQML() const noexcept;
    // NOLINTEND(modernize-use-trailing-return-type)

public:
    class Imp;

    auto getAccountActivity(const QString& accountID) const noexcept
        -> AccountActivityQt*;
    auto getAccountList() const noexcept -> AccountListQt*;
    auto getAccountStatus(const QString& accountID) const noexcept
        -> BlockchainAccountStatusQt*;
    auto getAccountTree() const noexcept -> AccountTreeQt*;
    auto getActiveNym() const noexcept -> QString;
    auto getContactActivity(const QString& contactID) const noexcept
        -> ContactActivityQt*;
    auto getContactList() const noexcept -> ContactListQt*;
    auto getNymList() const noexcept -> NymListQt*;
    auto getNymType() const noexcept -> QAbstractListModel*;
    auto getProfile() const noexcept -> ProfileQt*;

    auto setActiveNym(QString) noexcept -> void;

    OPENTXS_NO_EXPORT IdentityManagerQt(Imp* imp) noexcept;
    IdentityManagerQt(const IdentityManagerQt&) = delete;
    OPENTXS_NO_EXPORT IdentityManagerQt(IdentityManagerQt&& rhs) noexcept;
    auto operator=(const IdentityManagerQt&) -> IdentityManagerQt& = delete;
    auto operator=(IdentityManagerQt&&) -> IdentityManagerQt& = delete;

    OPENTXS_NO_EXPORT ~IdentityManagerQt() override;

private:
    Imp* imp_;
};
}  // namespace opentxs::ui
