// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "opentxs/interface/qt/BlockchainStatistics.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QVariant>
#include <cstdint>
#include <memory>

#include "interface/ui/blockchainstatistics/BlockchainStatisticsItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto BlockchainStatisticsQtModel(
    ui::internal::BlockchainStatistics& parent) noexcept
    -> std::unique_ptr<ui::BlockchainStatisticsQt>
{
    using ReturnType = ui::BlockchainStatisticsQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct BlockchainStatisticsQt::Imp {
    internal::BlockchainStatistics& parent_;

    Imp(internal::BlockchainStatistics& parent)
        : parent_(parent)
    {
    }
};

BlockchainStatisticsQt::BlockchainStatisticsQt(
    internal::BlockchainStatistics& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 7);
        internal_->SetRoleData({
            {BlockchainStatisticsQt::Balance, "balance"},
            {BlockchainStatisticsQt::BlockQueue, "blockqueue"},
            {BlockchainStatisticsQt::Chain, "chain"},
            {BlockchainStatisticsQt::FilterHeight, "filterheight"},
            {BlockchainStatisticsQt::HeaderHeight, "headerheight"},
            {BlockchainStatisticsQt::Name, "name"},
            {BlockchainStatisticsQt::ActivePeerCount, "activepeers"},
            {BlockchainStatisticsQt::ConnectedPeerCount, "totalpeers"},
        });
    }
}

auto BlockchainStatisticsQt::headerData(int section, Qt::Orientation, int role)
    const noexcept -> QVariant
{
    if (Qt::DisplayRole != role) { return {}; }

    switch (section) {
        case NameColumn: {
            return "Blockchain";
        }
        case BalanceColumn: {
            return "Balance";
        }
        case HeaderColumn: {
            return "Block header height";
        }
        case FilterColumn: {
            return "Block filter height";
        }
        case ConnectedPeerColumn: {
            return "Connected peers";
        }
        case ActivePeerColumn: {
            return "Active peers";
        }
        case BlockQueueColumn: {
            return "Block download queue";
        }
        default: {

            return {};
        }
    }
}

BlockchainStatisticsQt::~BlockchainStatisticsQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto BlockchainStatisticsItem::qt_data(
    const int column,
    const int role,
    QVariant& out) const noexcept -> void
{
    switch (role) {
        case Qt::TextAlignmentRole: {
            switch (column) {
                case BlockchainStatisticsQt::NameColumn: {
                    out = Qt::AlignLeft;
                } break;
                default: {
                    out = Qt::AlignHCenter;
                } break;
            }
        } break;
        case Qt::DisplayRole: {
            switch (column) {
                case BlockchainStatisticsQt::NameColumn: {
                    qt_data(column, BlockchainStatisticsQt::Name, out);
                } break;
                case BlockchainStatisticsQt::BalanceColumn: {
                    qt_data(column, BlockchainStatisticsQt::Balance, out);
                } break;
                case BlockchainStatisticsQt::HeaderColumn: {
                    qt_data(column, BlockchainStatisticsQt::HeaderHeight, out);
                } break;
                case BlockchainStatisticsQt::FilterColumn: {
                    qt_data(column, BlockchainStatisticsQt::FilterHeight, out);
                } break;
                case BlockchainStatisticsQt::ConnectedPeerColumn: {
                    qt_data(
                        column,
                        BlockchainStatisticsQt::ConnectedPeerCount,
                        out);
                } break;
                case BlockchainStatisticsQt::ActivePeerColumn: {
                    qt_data(
                        column, BlockchainStatisticsQt::ActivePeerCount, out);
                } break;
                case BlockchainStatisticsQt::BlockQueueColumn: {
                    qt_data(column, BlockchainStatisticsQt::BlockQueue, out);
                } break;
                default: {
                }
            }
        } break;
        case BlockchainStatisticsQt::Balance: {
            out = Balance().c_str();
        } break;
        case BlockchainStatisticsQt::BlockQueue: {
            out = static_cast<int>(BlockDownloadQueue());
        } break;
        case BlockchainStatisticsQt::Chain: {
            out = static_cast<int>(static_cast<std::uint32_t>(Chain()));
        } break;
        case BlockchainStatisticsQt::FilterHeight: {
            out = static_cast<int>(Filters());
        } break;
        case BlockchainStatisticsQt::HeaderHeight: {
            out = static_cast<int>(Headers());
        } break;
        case BlockchainStatisticsQt::Name: {
            out = Name().c_str();
        } break;
        case BlockchainStatisticsQt::ActivePeerCount: {
            out = static_cast<int>(ActivePeers());
        } break;
        case BlockchainStatisticsQt::ConnectedPeerCount: {
            out = static_cast<int>(ConnectedPeers());
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
