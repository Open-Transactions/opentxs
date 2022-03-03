// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "opentxs/interface/qt/UnitList.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QVariant>
#include <memory>

#include "interface/ui/unitlist/UnitListItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto UnitListQtModel(ui::internal::UnitList& parent) noexcept
    -> std::unique_ptr<ui::UnitListQt>
{
    using ReturnType = ui::UnitListQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct UnitListQt::Imp {
    internal::UnitList& parent_;

    Imp(internal::UnitList& parent)
        : parent_(parent)
    {
    }
};

UnitListQt::UnitListQt(internal::UnitList& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 1);
        internal_->SetRoleData({
            {UnitListQt::UnitIDRole, "unit"},
        });
    }
}

UnitListQt::~UnitListQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto UnitListItem::qt_data(const int column, const int role, QVariant& out)
    const noexcept -> void
{
    switch (role) {
        case UnitListQt::UnitIDRole: {
            out = static_cast<unsigned int>(Unit());
        } break;
        case Qt::DisplayRole: {
            out = Name().c_str();
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
