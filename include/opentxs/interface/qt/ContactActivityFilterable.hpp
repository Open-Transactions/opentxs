// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QSortFilterProxyModel>
#include <QString>
#include <QValidator>  // IWYU pragma: keep

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
class QModelIndex;

namespace opentxs
{
namespace ui
{
class ContactActivityQt;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT ContactActivityQtFilterable final
    : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(bool canMessage READ canMessage NOTIFY canMessageUpdate)
    Q_PROPERTY(QString displayName READ displayName NOTIFY displayNameUpdate)
    Q_PROPERTY(QString draft READ draft WRITE setDraft NOTIFY draftUpdate)
    Q_PROPERTY(QObject* draftValidator READ draftValidator CONSTANT)
    Q_PROPERTY(QString participants READ participants CONSTANT)
    Q_PROPERTY(QString threadID READ threadID CONSTANT)

Q_SIGNALS:
    void canMessageUpdate(bool) const;
    void displayNameUpdate() const;
    void draftUpdate() const;

public Q_SLOTS:
    void setDraft(QString);
    void blacklistType(int);
    void whitelistType(int);

public:
    // NOLINTBEGIN(modernize-use-trailing-return-type)
    Q_INVOKABLE bool pay(
        const QString& amount,
        const QString& sourceAccount,
        const QString& memo = "") const noexcept;
    Q_INVOKABLE QString paymentCode(const int currency) const noexcept;
    Q_INVOKABLE bool sendDraft() const noexcept;
    Q_INVOKABLE bool sendFaucetRequest(const int currency) const noexcept;
    // NOLINTEND(modernize-use-trailing-return-type)

public:
    // NOTE roles and columns same as ContactActivityQt

    auto canMessage() const noexcept -> bool;
    auto displayName() const noexcept -> QString;
    auto draft() const noexcept -> QString;
    auto draftValidator() const noexcept -> QValidator*;
    auto participants() const noexcept -> QString;
    auto threadID() const noexcept -> QString;

    OPENTXS_NO_EXPORT ContactActivityQtFilterable(
        ContactActivityQt& parent) noexcept;
    ContactActivityQtFilterable() = delete;
    ContactActivityQtFilterable(const ContactActivityQtFilterable&) = delete;
    ContactActivityQtFilterable(ContactActivityQtFilterable&&) = delete;
    auto operator=(const ContactActivityQtFilterable&)
        -> ContactActivityQtFilterable& = delete;
    auto operator=(ContactActivityQtFilterable&&)
        -> ContactActivityQtFilterable& = delete;

    OPENTXS_NO_EXPORT ~ContactActivityQtFilterable() final;

private:
    struct Imp;

    auto filterAcceptsRow(int source_row, const QModelIndex& source_parent)
        const -> bool final;
    auto lessThan(
        const QModelIndex& source_left,
        const QModelIndex& source_right) const -> bool final;

    Imp* imp_;
};
}  // namespace opentxs::ui
