// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/qt/NymList.hpp"  // IWYU pragma: associated

#include <QVariant>
#include <memory>

#include "interface/ui/nymlist/NymListItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto NymListQtModel(ui::internal::NymList& parent) noexcept
    -> std::unique_ptr<ui::NymListQt>
{
    using ReturnType = ui::NymListQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct NymListQt::Imp {
    internal::NymList& parent_;

    Imp(internal::NymList& parent)
        : parent_(parent)
    {
    }
};

NymListQt::NymListQt(internal::NymList& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 1);
        internal_->SetRoleData({
            {NymListQt::IDRole, "id"},
            {NymListQt::NameRole, "name"},
        });
    }
}

NymListQt::~NymListQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto NymListItem::qt_data(const int column, const int role, QVariant& out)
    const noexcept -> void
{
    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case NymListQt::NameColumn: {
                    qt_data(column, NymListQt::NameRole, out);
                } break;
                default: {
                }
            }
        } break;
        case NymListQt::IDRole: {
            out = NymID().c_str();
        } break;
        case NymListQt::NameRole: {
            out = Name().c_str();
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
