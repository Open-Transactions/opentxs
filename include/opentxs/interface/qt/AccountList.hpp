// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QObject>
#include <QString>

#include "opentxs/Export.hpp"
#include "opentxs/interface/qt/Model.hpp"

class QObject;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
namespace internal
{
struct AccountList;
}  // namespace internal

class AccountListQt;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::ui::AccountListQt final : public qt::Model
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 0,         // QString
        NotaryIDRole = Qt::UserRole + 1,     // QString
        NotaryNameRole = Qt::UserRole + 2,   // QString
        UnitRole = Qt::UserRole + 3,         // int (identity::wot::claim::)
        UnitNameRole = Qt::UserRole + 4,     // QString
        AccountIDRole = Qt::UserRole + 5,    // QString
        BalanceRole = Qt::UserRole + 6,      // QString
        PolarityRole = Qt::UserRole + 7,     // int (-1, 0, or 1)
        AccountTypeRole = Qt::UserRole + 8,  // int (opentxs::AccountType)
        ContractIdRole = Qt::UserRole + 9,   // QString
    };
    enum Columns {
        NotaryNameColumn = 0,
        DisplayUnitColumn = 1,
        AccountNameColumn = 2,
        DisplayBalanceColumn = 3,
    };

    AccountListQt(internal::AccountList& parent) noexcept;
    AccountListQt(const AccountListQt&) = delete;
    AccountListQt(AccountListQt&&) = delete;
    auto operator=(const AccountListQt&) -> AccountListQt& = delete;
    auto operator=(AccountListQt&&) -> AccountListQt& = delete;

    ~AccountListQt() final;

private:
    struct Imp;

    Imp* imp_;
};
