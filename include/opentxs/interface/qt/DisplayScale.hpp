// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QVariant>

class QObject;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace display
{
class Definition;
}  // namespace display

namespace ui
{
class DisplayScaleQt;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::ui::DisplayScaleQt final
    : public QAbstractListModel
{
    Q_OBJECT

public:
    int rowCount(const QModelIndex& parent = QModelIndex()) const final;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)
        const final;

    DisplayScaleQt(const display::Definition&) noexcept;
    DisplayScaleQt(const DisplayScaleQt&) noexcept;

    ~DisplayScaleQt() final = default;

private:
    const display::Definition& data_;

    DisplayScaleQt() = delete;
    DisplayScaleQt(DisplayScaleQt&&) = delete;
    DisplayScaleQt& operator=(const DisplayScaleQt&) = delete;
    DisplayScaleQt& operator=(DisplayScaleQt&&) = delete;
};
