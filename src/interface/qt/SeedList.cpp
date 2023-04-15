// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/qt/SeedList.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QVariant>
#include <memory>

#include "interface/ui/seedlist/SeedListItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/core/identifier/Generic.hpp"

namespace opentxs::factory
{
auto SeedListQtModel(ui::internal::SeedList& parent) noexcept
    -> std::unique_ptr<ui::SeedListQt>
{
    using ReturnType = ui::SeedListQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct SeedListQt::Imp {
    internal::SeedList& parent_;

    Imp(internal::SeedList& parent)
        : parent_(parent)
    {
    }
};

SeedListQt::SeedListQt(internal::SeedList& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 1);
        internal_->SetRoleData({
            {SeedListQt::SeedIDRole, "seedid"},
            {SeedListQt::SeedNameRole, "seedname"},
            {SeedListQt::SeedTypeRole, "seedtype"},
        });
    }
}

SeedListQt::~SeedListQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto SeedListItem::qt_data(const int column, const int role, QVariant& out)
    const noexcept -> void
{
    using Parent = SeedListQt;

    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case Parent::NameColumn: {
                    qt_data(column, Parent::SeedNameRole, out);
                } break;
                default: {
                }
            }
        } break;
        case Parent::SeedIDRole: {
            out = SeedID().asBase58(api_.Crypto()).c_str();
        } break;
        case Parent::SeedNameRole: {
            out = Name().c_str();
        } break;
        case Parent::SeedTypeRole: {
            out = static_cast<int>(Type());
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
