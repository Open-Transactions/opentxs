// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QObject>
#include <QVariant>

#include "opentxs/Export.hpp"
#include "opentxs/interface/qt/Model.hpp"
#include "opentxs/interface/qt/QML.hpp"

class QModelIndex;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
namespace internal
{
struct BlockchainSelection;
}  // namespace internal
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT BlockchainSelectionQt final : public qt::Model
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int enabledCount READ enabledCount NOTIFY enabledChanged)

Q_SIGNALS:
    void chainEnabled(int chain);
    void chainDisabled(int chain);
    void enabledChanged(int enabledCount);

public:
    // NOLINTBEGIN(modernize-use-trailing-return-type)
    /// chain is an opentxs::blockchain::Type, retrievable as TypeRole
    Q_INVOKABLE bool disableChain(const int chain) noexcept;
    /// chain is an opentxs::blockchain::Type, retrievable as TypeRole
    Q_INVOKABLE bool enableChain(const int chain) noexcept;
    // NOLINTEND(modernize-use-trailing-return-type)

public:
    enum Roles {
        NameRole = Qt::UserRole + 0,   // QString
        TypeRole = Qt::UserRole + 1,   // int, opentxs::blockchain::Type
        IsEnabled = Qt::UserRole + 2,  // bool
        IsTestnet = Qt::UserRole + 3,  // bool
    };
    Q_ENUM(Roles)
    enum Columns {
        NameColumn = 0,
    };
    Q_ENUM(Columns)

    auto enabledCount() const noexcept -> int;
    auto flags(const QModelIndex& index) const -> Qt::ItemFlags final;

    auto setData(const QModelIndex& index, const QVariant& value, int role)
        -> bool final;

    OPENTXS_NO_EXPORT BlockchainSelectionQt(
        internal::BlockchainSelection& parent) noexcept;
    BlockchainSelectionQt(const BlockchainSelectionQt&) = delete;
    BlockchainSelectionQt(BlockchainSelectionQt&&) = delete;
    auto operator=(const BlockchainSelectionQt&)
        -> BlockchainSelectionQt& = delete;
    auto operator=(BlockchainSelectionQt&&) -> BlockchainSelectionQt& = delete;

    OPENTXS_NO_EXPORT ~BlockchainSelectionQt() final;

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::ui
