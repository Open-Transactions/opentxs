// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <QMetaObject>
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
struct Contact;
}  // namespace internal

class ContactQt;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)
class OPENTXS_EXPORT opentxs::ui::ContactQt final : public qt::Model
{
    Q_OBJECT
    Q_PROPERTY(QString displayName READ displayName NOTIFY displayNameChanged)
    Q_PROPERTY(QString contactID READ contactID CONSTANT)
    Q_PROPERTY(QString paymentCode READ paymentCode NOTIFY paymentCodeChanged)

signals:
    void displayNameChanged(QString) const;
    void paymentCodeChanged(QString) const;

public:
    // Tree layout
    auto displayName() const noexcept -> QString;
    auto contactID() const noexcept -> QString;
    auto paymentCode() const noexcept -> QString;

    ContactQt(internal::Contact& parent) noexcept;
    ContactQt() = delete;
    ContactQt(const ContactQt&) = delete;
    ContactQt(ContactQt&&) = delete;
    auto operator=(const ContactQt&) -> ContactQt& = delete;
    auto operator=(ContactQt&&) -> ContactQt& = delete;

    ~ContactQt() final;

private:
    struct Imp;

    Imp* imp_;
};
