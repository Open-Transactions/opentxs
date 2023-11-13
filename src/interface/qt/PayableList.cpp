// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/qt/PayableList.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QVariant>
#include <memory>
#include <utility>

#include "interface/ui/contactlist/ContactListItem.hpp"
#include "interface/ui/payablelist/PayableListItem.hpp"
#include "internal/interface/ui/UI.hpp"

namespace opentxs::factory
{
auto PayableListQtModel(ui::internal::PayableList& parent) noexcept
    -> std::unique_ptr<ui::PayableListQt>
{
    using ReturnType = ui::PayableListQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct PayableListQt::Imp {
    internal::PayableList& parent_;

    Imp(internal::PayableList& parent)
        : parent_(parent)
    {
    }
};

PayableListQt::PayableListQt(internal::PayableList& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 2);
        internal_->SetRoleData({
            {PayableListQt::ContactIDRole, "id"},
            {PayableListQt::SectionRole, "section"},
        });
    }
}

PayableListQt::~PayableListQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto PayableListItem::qt_data(const int column, const int role, QVariant& out)
    const noexcept -> void
{
    if (0 == column) {
        ContactListItem::qt_data(column, role, out);

        return;
    }

    switch (role) {
        case Qt::DisplayRole: {
            out = PaymentCode().c_str();
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
