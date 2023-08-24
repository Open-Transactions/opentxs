// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/interface/qt/AccountList.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QString>
#include <QVariant>
#include <memory>

#include "interface/ui/accountlist/AccountListItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "util/Polarity.hpp"  // IWYU pragma: keep

namespace opentxs::factory
{
auto AccountListQtModel(ui::internal::AccountList& parent) noexcept
    -> std::unique_ptr<ui::AccountListQt>
{
    using ReturnType = ui::AccountListQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct AccountListQt::Imp {
    internal::AccountList& parent_;

    Imp(internal::AccountList& parent)
        : parent_(parent)
    {
    }
};

AccountListQt::AccountListQt(internal::AccountList& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 4);
        internal_->SetRoleData({
            {AccountListQt::NameRole, "name"},
            {AccountListQt::NotaryIDRole, "notaryid"},
            {AccountListQt::NotaryNameRole, "notaryname"},
            {AccountListQt::UnitRole, "unit"},
            {AccountListQt::UnitNameRole, "unitname"},
            {AccountListQt::AccountIDRole, "account"},
            {AccountListQt::BalanceRole, "balance"},
            {AccountListQt::PolarityRole, "polarity"},
            {AccountListQt::AccountTypeRole, "accounttype"},
            {AccountListQt::ContractIdRole, "contractid"},
        });
    }
}

AccountListQt::~AccountListQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto AccountListItem::qt_data(const int column, const int role, QVariant& out)
    const noexcept -> void
{
    using Parent = AccountListQt;

    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case Parent::NotaryNameColumn: {
                    qt_data(column, Parent::NotaryNameRole, out);
                } break;
                case Parent::DisplayUnitColumn: {
                    qt_data(column, Parent::UnitNameRole, out);
                } break;
                case Parent::AccountNameColumn: {
                    qt_data(column, Parent::NameRole, out);
                } break;
                case Parent::DisplayBalanceColumn: {
                    qt_data(column, Parent::BalanceRole, out);
                } break;
                default: {
                }
            }
        } break;
        case Parent::NameRole: {
            out = Name().c_str();
        } break;
        case Parent::NotaryIDRole: {
            out = NotaryID().c_str();
        } break;
        case Parent::NotaryNameRole: {
            out = NotaryName().c_str();
        } break;
        case Parent::UnitRole: {
            out = static_cast<int>(Unit());
        } break;
        case Parent::UnitNameRole: {
            out = DisplayUnit().c_str();
        } break;
        case Parent::AccountIDRole: {
            out = AccountID().c_str();
        } break;
        case Parent::BalanceRole: {
            out = DisplayBalance().c_str();
        } break;
        case Parent::PolarityRole: {
            out = polarity(Balance());
        } break;
        case Parent::AccountTypeRole: {
            out = static_cast<int>(Type());
        } break;
        case Parent::ContractIdRole: {
            out = ContractID().c_str();
        } break;
        case Parent::UnitDescriptionRole: {
            const auto text = print(Unit());
            out = QString::fromUtf8(text.data(), size_to_int(text.size()));
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
