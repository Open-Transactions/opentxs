// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QObject>

#include "opentxs/Export.hpp"
#include "opentxs/interface/qt/Model.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
namespace internal
{
struct AccountSummary;
}  // namespace internal
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT AccountSummaryQt final : public qt::Model
{
    Q_OBJECT

public:
    // Tree layout
    enum Roles {
        NotaryIDRole = Qt::UserRole + 0,
        AccountIDRole = Qt::UserRole + 1,
        BalanceRole = Qt::UserRole + 2,
    };
    enum Columns {
        IssuerNameColumn = 0,
        ConnectionStateColumn = 1,
        TrustedColumn = 2,
        AccountNameColumn = 3,
        BalanceColumn = 4,
    };

    OPENTXS_NO_EXPORT AccountSummaryQt(
        internal::AccountSummary& parent) noexcept;
    AccountSummaryQt(const AccountSummaryQt&) = delete;
    AccountSummaryQt(AccountSummaryQt&&) = delete;
    auto operator=(const AccountSummaryQt&) -> AccountSummaryQt& = delete;
    auto operator=(AccountSummaryQt&&) -> AccountSummaryQt& = delete;

    OPENTXS_NO_EXPORT ~AccountSummaryQt() final;

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::ui
