// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QMetaObject>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>

#include "opentxs/Export.hpp"

class QObject;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace display
{
class Definition;
}  // namespace display

namespace ui
{
class DisplayScaleQt;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::ui::DisplayScaleQt final
    : public QAbstractListModel
{
    Q_OBJECT

public:
    auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int final;
    auto data(const QModelIndex& index, int role = Qt::DisplayRole) const
        -> QVariant final;

    DisplayScaleQt(const display::Definition&) noexcept;
    DisplayScaleQt(const DisplayScaleQt&) noexcept;
    DisplayScaleQt() = delete;
    DisplayScaleQt(DisplayScaleQt&&) = delete;
    auto operator=(const DisplayScaleQt&) -> DisplayScaleQt& = delete;
    auto operator=(DisplayScaleQt&&) -> DisplayScaleQt& = delete;

    ~DisplayScaleQt() final = default;

private:
    const display::Definition& data_;
};
