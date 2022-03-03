// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "opentxs/interface/qt/SeedTree.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QVariant>
#include <cstddef>
#include <memory>

#include "interface/ui/seedtree/SeedTreeItem.hpp"
#include "interface/ui/seedtree/SeedTreeNym.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto SeedTreeQtModel(ui::internal::SeedTree& parent) noexcept
    -> std::unique_ptr<ui::SeedTreeQt>
{
    using ReturnType = ui::SeedTreeQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct SeedTreeQt::Imp {
    internal::SeedTree& parent_;

    Imp(internal::SeedTree& parent)
        : parent_(parent)
    {
    }
};

SeedTreeQt::SeedTreeQt(internal::SeedTree& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 1);
        internal_->SetRoleData({
            {SeedTreeQt::SeedIDRole, "seedid"},
            {SeedTreeQt::SeedNameRole, "seedname"},
            {SeedTreeQt::SeedTypeRole, "seedtype"},
            {SeedTreeQt::NymIDRole, "nymid"},
            {SeedTreeQt::NymNameRole, "nymname"},
            {SeedTreeQt::NymIndexRole, "nymindex"},
        });
    }

    imp_->parent_.SetCallbacks(
        {[this](const auto& id) {
             emit defaultNymChanged(QString::fromStdString(id.str()));
         },
         [this](const auto& id) {
             emit defaultSeedChanged(QString::fromStdString(id.str()));
         }});
}

auto SeedTreeQt::check() -> void
{
    if (const auto seed = defaultSeed(); seed.isEmpty()) {
        emit needSeed();
    } else if (const auto nym = defaultNym(); nym.isEmpty()) {
        emit needNym();
    } else {
        emit ready();
    }
}

auto SeedTreeQt::defaultNym() const noexcept -> QString
{
    return QString::fromStdString(imp_->parent_.DefaultNym()->str());
}

auto SeedTreeQt::defaultSeed() const noexcept -> QString
{
    return QString::fromStdString(imp_->parent_.DefaultSeed()->str());
}

SeedTreeQt::~SeedTreeQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto SeedTreeItem::qt_data(const int column, const int role, QVariant& out)
    const noexcept -> void
{
    using Parent = SeedTreeQt;

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
            out = SeedID().c_str();
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
auto SeedTreeNym::qt_data(const int column, const int role, QVariant& out)
    const noexcept -> void
{
    using Parent = SeedTreeQt;

    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case Parent::NameColumn: {
                    qt_data(column, Parent::NymNameRole, out);
                } break;
                default: {
                }
            }
        } break;
        case Parent::NymNameRole: {
            out = Name().c_str();
        } break;
        case Parent::NymIDRole: {
            out = NymID().c_str();
        } break;
        case Parent::NymIndexRole: {
            static_assert(sizeof(unsigned long long) >= sizeof(std::size_t));

            out = static_cast<unsigned long long>(Index());
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
