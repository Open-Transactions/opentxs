// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QAbstractItemModel>
#include <QMetaObject>
#include <QObject>
#include <QVariant>

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace display
{
class Definition;
}  // namespace display
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT DisplayScaleQt final : public QAbstractListModel
{
    Q_OBJECT

public:
    auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int final;
    auto data(const QModelIndex& index, int role = Qt::DisplayRole) const
        -> QVariant final;

    OPENTXS_NO_EXPORT DisplayScaleQt(const display::Definition&) noexcept;
    DisplayScaleQt(const DisplayScaleQt&) noexcept;
    DisplayScaleQt() = delete;
    DisplayScaleQt(DisplayScaleQt&&) = delete;
    auto operator=(const DisplayScaleQt&) -> DisplayScaleQt& = delete;
    auto operator=(DisplayScaleQt&&) -> DisplayScaleQt& = delete;

    OPENTXS_NO_EXPORT ~DisplayScaleQt() final = default;

private:
    const display::Definition& data_;
};
}  // namespace opentxs::ui
