// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QString>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class AccountActivityQt;
class AccountListQt;
class AccountTreeQt;
class ActivityThreadQt;
class BlockchainAccountStatusQt;
class ContactListQt;
class IdentityManagerQt;
class NymListQt;
class ProfileQt;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class QAbstractListModel;
class QObject;

class OPENTXS_EXPORT opentxs::ui::IdentityManagerQt : public QObject
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

signals:
    void activeNymChanged(QString) const;
    void needNym() const;

public slots:
    void setActiveNym(QString) const;

public:
    class Imp;

    auto getAccountActivity(const QString& accountID) const noexcept
        -> AccountActivityQt*;
    auto getAccountList() const noexcept -> AccountListQt*;
    auto getAccountStatus(const QString& accountID) const noexcept
        -> BlockchainAccountStatusQt*;
    auto getAccountTree() const noexcept -> AccountTreeQt*;
    auto getActiveNym() const noexcept -> QString;
    auto getActivityThread(const QString& contactID) const noexcept
        -> ActivityThreadQt*;
    auto getContactList() const noexcept -> ContactListQt*;
    auto getNymList() const noexcept -> NymListQt*;
    auto getNymType() const noexcept -> QAbstractListModel*;
    auto getProfile() const noexcept -> ProfileQt*;

    auto setActiveNym(QString) noexcept -> void;

    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QObject* getAccountActivityQML(
        const QString& accountID) const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QObject* getAccountListQML() const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QObject* getAccountStatusQML(
        const QString& accountID) const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QObject* getAccountTreeQML() const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QObject* getActivityThreadQML(
        const QString& contactID) const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QObject* getContactListQML() const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QObject* getNymListQML() const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QObject* getNymTypeQML() const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QObject* getProfileQML() const noexcept;

    OPENTXS_NO_EXPORT IdentityManagerQt(Imp* imp) noexcept;
    OPENTXS_NO_EXPORT IdentityManagerQt(IdentityManagerQt&& rhs) noexcept;

    ~IdentityManagerQt() override;

private:
    Imp* imp_;

    IdentityManagerQt(const IdentityManagerQt&) = delete;
    IdentityManagerQt& operator=(const IdentityManagerQt&) = delete;
    IdentityManagerQt& operator=(IdentityManagerQt&&) = delete;
};
