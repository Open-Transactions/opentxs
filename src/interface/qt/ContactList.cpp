// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/qt/ContactList.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QVariant>
#include <memory>

#include "interface/ui/contactlist/ContactListItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto ContactListQtModel(ui::internal::ContactList& parent) noexcept
    -> std::unique_ptr<ui::ContactListQt>
{
    using ReturnType = ui::ContactListQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct ContactListQt::Imp {
    internal::ContactList& parent_;

    Imp(internal::ContactList& parent)
        : parent_(parent)
    {
    }
};

ContactListQt::ContactListQt(internal::ContactList& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 1);
        internal_->SetRoleData({
            {ContactListQt::IDRole, "id"},
            {ContactListQt::NameRole, "name"},
            {ContactListQt::ImageRole, "image"},
            {ContactListQt::SectionRole, "section"},
        });
    }
}

auto ContactListQt::addContact(
    const QString& label,
    const QString& paymentCode,
    const QString& nymID) const noexcept -> QString
{
    return imp_->parent_
        .AddContact(
            label.toStdString(), paymentCode.toStdString(), nymID.toStdString())
        .c_str();
}

ContactListQt::~ContactListQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto ContactListItem::qt_data(const int column, const int role, QVariant& out)
    const noexcept -> void
{
    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case ContactListQt::NameColumn: {
                    qt_data(column, ContactListQt::NameRole, out);
                } break;
                default: {
                }
            }
        } break;
        case Qt::DecorationRole: {
            // TODO render ImageURI into a QPixmap
            out = {};
        } break;
        case Qt::TextAlignmentRole: {
            switch (column) {
                case ContactListQt::NameColumn: {
                    out = Qt::AlignLeft;
                } break;
                default: {
                }
            }
        } break;
        case ContactListQt::IDRole: {
            out = ContactID().c_str();
        } break;
        case ContactListQt::NameRole: {
            out = DisplayName().c_str();
        } break;
        case ContactListQt::ImageRole: {
            out = ImageURI().c_str();
        } break;
        case ContactListQt::SectionRole: {
            out = Section().c_str();
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
