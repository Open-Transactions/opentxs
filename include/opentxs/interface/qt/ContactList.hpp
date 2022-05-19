// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QString>

#include "opentxs/interface/qt/Model.hpp"

class QObject;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
namespace internal
{
struct ContactList;
}  // namespace internal

class ContactListQt;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::ui::ContactListQt final : public qt::Model
{
    Q_OBJECT

public:
    // NOLINTBEGIN(modernize-use-trailing-return-type)
    Q_INVOKABLE QString addContact(
        const QString& label,
        const QString& paymentCode = "",
        const QString& nymID = "") const noexcept;
    // NOLINTEND(modernize-use-trailing-return-type)

public:
    enum Roles {
        IDRole = Qt::UserRole + 0,       // QString
        NameRole = Qt::UserRole + 1,     // QString
        ImageRole = Qt::UserRole + 2,    // QPixmap
        SectionRole = Qt::UserRole + 3,  // QString
    };
    // This model is designed to be used in a list view
    enum Columns {
        NameColumn = 0,
    };

    ContactListQt(internal::ContactList& parent) noexcept;

    ~ContactListQt() final;

private:
    struct Imp;

    Imp* imp_;

    ContactListQt() = delete;
    ContactListQt(const ContactListQt&) = delete;
    ContactListQt(ContactListQt&&) = delete;
    auto operator=(const ContactListQt&) -> ContactListQt& = delete;
    auto operator=(ContactListQt&&) -> ContactListQt& = delete;
};
