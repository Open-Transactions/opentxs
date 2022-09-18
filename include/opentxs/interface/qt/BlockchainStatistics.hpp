// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVariant>

#include "opentxs/interface/qt/Model.hpp"

class QModelIndex;
class QObject;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
namespace internal
{
struct BlockchainStatistics;
}  // namespace internal

class BlockchainStatisticsQt;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::ui::BlockchainStatisticsQt final
    : public qt::Model
{
    Q_OBJECT
public:
    // Seven columns when used in a table view
    //
    // All data is available in a list view via the user roles defined below:
    enum Roles {
        Balance = Qt::UserRole + 0,     // QString
        BlockQueue = Qt::UserRole + 1,  // int, std::size_t
        Chain = Qt::UserRole + 2,       // int, opentxs::blockchain::Type
        FilterHeight =
            Qt::UserRole + 3,  // int, opentxs::blockchain::block::Height
        HeaderHeight =
            Qt::UserRole + 4,     // int, opentxs::blockchain::block::Height
        Name = Qt::UserRole + 5,  // QString
        ActivePeerCount = Qt::UserRole + 6,     // int, std::size_t
        ConnectedPeerCount = Qt::UserRole + 7,  // int, std::size_t
    };
    enum Columns {
        NameColumn = 0,
        BalanceColumn = 1,
        HeaderColumn = 2,
        FilterColumn = 3,
        ConnectedPeerColumn = 4,
        ActivePeerColumn = 5,
        BlockQueueColumn = 6,
    };

    auto headerData(
        int section,
        Qt::Orientation orientation,
        int role = Qt::DisplayRole) const noexcept -> QVariant final;

    BlockchainStatisticsQt(internal::BlockchainStatistics& parent) noexcept;
    BlockchainStatisticsQt(const BlockchainStatisticsQt&) = delete;
    BlockchainStatisticsQt(BlockchainStatisticsQt&&) = delete;
    auto operator=(const BlockchainStatisticsQt&)
        -> BlockchainStatisticsQt& = delete;
    auto operator=(BlockchainStatisticsQt&&)
        -> BlockchainStatisticsQt& = delete;

    ~BlockchainStatisticsQt() final;

private:
    struct Imp;

    Imp* imp_;
};
