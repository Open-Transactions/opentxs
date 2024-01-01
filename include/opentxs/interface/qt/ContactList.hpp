// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtQmlIntegration>

#include "opentxs/Export.hpp"
#include "opentxs/interface/qt/Model.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
namespace internal
{
struct ContactList;
}  // namespace internal
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT ContactListQt final : public qt::Model
{
    Q_OBJECT
    QML_ELEMENT

public:
    // NOLINTBEGIN(modernize-use-trailing-return-type)
    Q_INVOKABLE QString addContact(
        const QString& label,
        const QString& paymentCode = "",
        const QString& nymID = "") const noexcept;
    Q_INVOKABLE bool setContactName(
        const QString& contactID,
        const QString& name) const noexcept;
    // NOLINTEND(modernize-use-trailing-return-type)

public:
    enum Roles {
        IDRole = Qt::UserRole + 0,       // QString
        NameRole = Qt::UserRole + 1,     // QString
        ImageRole = Qt::UserRole + 2,    // QPixmap
        SectionRole = Qt::UserRole + 3,  // QString
    };
    Q_ENUM(Roles)
    // This model is designed to be used in a list view
    enum Columns {
        NameColumn = 0,
    };
    Q_ENUM(Columns)

    OPENTXS_NO_EXPORT ContactListQt(internal::ContactList& parent) noexcept;
    ContactListQt() = delete;
    ContactListQt(const ContactListQt&) = delete;
    ContactListQt(ContactListQt&&) = delete;
    auto operator=(const ContactListQt&) -> ContactListQt& = delete;
    auto operator=(ContactListQt&&) -> ContactListQt& = delete;

    OPENTXS_NO_EXPORT ~ContactListQt() final;

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::ui
