// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QIdentityProxyModel>

#include "opentxs/Export.hpp"

namespace opentxs::ui
{
struct OPENTXS_EXPORT BlankModel final : public QAbstractItemModel {
    Q_OBJECT

public:
    auto columnCount(const QModelIndex& = QModelIndex()) const noexcept
        -> int final
    {
        return static_cast<int>(columns_);
    }
    auto data(const QModelIndex&, int = Qt::DisplayRole) const noexcept
        -> QVariant final
    {
        return {};
    }
    auto index(int, int, const QModelIndex& = QModelIndex()) const noexcept
        -> QModelIndex final
    {
        return {};
    }
    auto parent(const QModelIndex&) const noexcept -> QModelIndex final
    {
        return {};
    }
    auto rowCount(const QModelIndex& = QModelIndex()) const noexcept
        -> int final
    {
        return 0;
    }

    BlankModel(const std::size_t columns)
        : columns_(columns)
    {
        assert(columns_ <= std::numeric_limits<int>::max());
    }
    BlankModel() = delete;

private:
    const std::size_t columns_;
};
}  // namespace opentxs::ui
