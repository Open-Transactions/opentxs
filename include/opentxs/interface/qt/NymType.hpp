// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QAbstractItemModel>
#include <QAbstractListModel>  // IWYU pragma: keep
#include <QMetaObject>
#include <QObject>
#include <QVariant>

#include "opentxs/Export.hpp"
#include "opentxs/interface/qt/QML.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class NymTypePrivate;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT NymType final : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        NymTypeValue = Qt::UserRole + 0,  // int (identity::Type)
        NymTypeName = Qt::UserRole + 1,   // QString
    };
    Q_ENUM(Roles)

    auto data(const QModelIndex& index, int role = Qt::DisplayRole)
        const noexcept -> QVariant final;
    auto headerData(
        int section,
        Qt::Orientation orientation,
        int role = Qt::DisplayRole) const noexcept -> QVariant final;
    auto rowCount(const QModelIndex& parent = {}) const noexcept -> int final;

    OPENTXS_NO_EXPORT NymType([[maybe_unused]] int) noexcept;
    NymType(const NymType&) = delete;
    NymType(NymType&&) = delete;
    auto operator=(const NymType&) -> NymType& = delete;
    auto operator=(NymType&&) -> NymType& = delete;

    OPENTXS_NO_EXPORT ~NymType() final;

private:
    NymTypePrivate* imp_;
};
}  // namespace opentxs::ui
