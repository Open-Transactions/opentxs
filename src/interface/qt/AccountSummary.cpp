// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "opentxs/interface/qt/AccountSummary.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QVariant>
#include <QtGlobal>
#include <exception>
#include <memory>

#include "interface/ui/accountsummary/AccountSummaryItem.hpp"
#include "interface/ui/accountsummary/IssuerItem.hpp"
#include "internal/core/Amount.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto AccountSummaryQtModel(ui::internal::AccountSummary& parent) noexcept
    -> std::unique_ptr<ui::AccountSummaryQt>
{
    using ReturnType = ui::AccountSummaryQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct AccountSummaryQt::Imp {
    internal::AccountSummary& parent_;

    Imp(internal::AccountSummary& parent)
        : parent_(parent)
    {
    }
};

AccountSummaryQt::AccountSummaryQt(internal::AccountSummary& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 5);
        internal_->SetRoleData({
            {AccountSummaryQt::NotaryIDRole, "notary"},
            {AccountSummaryQt::AccountIDRole, "account"},
            {AccountSummaryQt::BalanceRole, "balance"},
        });
    }
}

AccountSummaryQt::~AccountSummaryQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto AccountSummaryItem::qt_data(
    const int column,
    const int role,
    QVariant& out) const noexcept -> void
{
    switch (role) {
        case AccountSummaryQt::NotaryIDRole: {
            // TODO
            out = {};
        } break;
        case AccountSummaryQt::AccountIDRole: {
            out = AccountID().c_str();
        } break;
        case AccountSummaryQt::BalanceRole: {
            try {
                out = qlonglong{Balance().Internal().ExtractInt64()};
            } catch (const std::exception& e) {
                LogError()(OT_PRETTY_CLASS())("Error getting balance.")(
                    e.what())
                    .Flush();
                out = {};
            }
        } break;
        case Qt::DisplayRole: {
            switch (column) {
                case AccountSummaryQt::AccountNameColumn: {
                    out = Name().c_str();
                } break;
                case AccountSummaryQt::BalanceColumn: {
                    out = DisplayBalance().c_str();
                } break;
                default: {
                }
            }
        } break;
        default: {
        }
    }
}

auto IssuerItem::qt_data(const int column, const int role, QVariant& out)
    const noexcept -> void
{
    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case AccountSummaryQt::IssuerNameColumn: {
                    out = Name().c_str();
                } break;
                default: {
                }
            }
        } break;
        case Qt::ToolTipRole: {
            out = Debug().c_str();
        } break;
        case Qt::CheckStateRole: {
            switch (column) {
                case AccountSummaryQt::ConnectionStateColumn: {
                    out = ConnectionState();
                } break;
                case AccountSummaryQt::TrustedColumn: {
                    out = Trusted();
                } break;
                default: {
                }
            }
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
