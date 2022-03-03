// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QString>
#include <QValidator>
#include <QVariant>

#include "opentxs/interface/qt/Model.hpp"

class QObject;
class QValidator;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
namespace internal
{
struct ActivityThread;
}  // namespace internal

class ActivityThreadQt;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::ui::ActivityThreadQt final : public qt::Model
{
    Q_OBJECT
    Q_PROPERTY(bool canMessage READ canMessage NOTIFY canMessageUpdate)
    Q_PROPERTY(QString displayName READ displayName NOTIFY displayNameUpdate)
    Q_PROPERTY(QString draft READ draft WRITE setDraft NOTIFY draftUpdate)
    Q_PROPERTY(QObject* draftValidator READ draftValidator CONSTANT)
    Q_PROPERTY(QString participants READ participants CONSTANT)
    Q_PROPERTY(QString threadID READ threadID CONSTANT)

signals:
    void canMessageUpdate(bool) const;
    void displayNameUpdate() const;
    void draftUpdate() const;

public slots:
    void setDraft(QString);

public:
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE bool pay(
        const QString& amount,
        const QString& sourceAccount,
        const QString& memo = "") const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QString paymentCode(const int currency) const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE bool sendDraft() const noexcept;

public:
    enum Roles {
        AmountRole = Qt::UserRole + 0,    // QString
        LoadingRole = Qt::UserRole + 1,   // bool
        MemoRole = Qt::UserRole + 2,      // QString
        PendingRole = Qt::UserRole + 3,   // bool
        PolarityRole = Qt::UserRole + 4,  // int, -1, 0, or 1
        TextRole = Qt::UserRole + 5,      // QString
        TimeRole = Qt::UserRole + 6,      // QDateTime
        TypeRole = Qt::UserRole + 7,      // int, opentxs::StorageBox
        OutgoingRole = Qt::UserRole + 8,  // bool
        FromRole = Qt::UserRole + 9,      // QString
    };
    enum Columns {
        TimeColumn = 0,
        FromColumn = 1,
        TextColumn = 2,
        AmountColumn = 3,
        MemoColumn = 4,
        LoadingColumn = 5,
        PendingColumn = 6,
    };

    auto canMessage() const noexcept -> bool;
    auto displayName() const noexcept -> QString;
    auto draft() const noexcept -> QString;
    auto draftValidator() const noexcept -> QValidator*;
    auto headerData(
        int section,
        Qt::Orientation orientation,
        int role = Qt::DisplayRole) const noexcept -> QVariant final;
    auto participants() const noexcept -> QString;
    auto threadID() const noexcept -> QString;

    ActivityThreadQt(internal::ActivityThread& parent) noexcept;

    ~ActivityThreadQt() final;

private:
    struct Imp;

    Imp* imp_;

    ActivityThreadQt() = delete;
    ActivityThreadQt(const ActivityThreadQt&) = delete;
    ActivityThreadQt(ActivityThreadQt&&) = delete;
    ActivityThreadQt& operator=(const ActivityThreadQt&) = delete;
    ActivityThreadQt& operator=(ActivityThreadQt&&) = delete;
};
