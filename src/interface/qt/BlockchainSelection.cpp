// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "opentxs/interface/qt/BlockchainSelection.hpp"  // IWYU pragma: associated

#include <QAbstractItemModel>
#include <QObject>
#include <QVariant>
#include <QtGlobal>
#include <cstdint>
#include <memory>

#include "interface/ui/blockchainselection/BlockchainSelectionItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto BlockchainSelectionQtModel(
    ui::internal::BlockchainSelection& parent) noexcept
    -> std::unique_ptr<ui::BlockchainSelectionQt>
{
    using ReturnType = ui::BlockchainSelectionQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct BlockchainSelectionQt::Imp {
    internal::BlockchainSelection& parent_;

    Imp(internal::BlockchainSelection& parent)
        : parent_(parent)
    {
    }
};

BlockchainSelectionQt::BlockchainSelectionQt(
    internal::BlockchainSelection& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 1);
        internal_->SetRoleData({
            {BlockchainSelectionQt::NameRole, "name"},
            {BlockchainSelectionQt::TypeRole, "type"},
            {BlockchainSelectionQt::IsEnabled, "enabled"},
            {BlockchainSelectionQt::IsTestnet, "testnet"},
        });
    }

    imp_->parent_.Set([this](auto chain, auto enabled, auto total) -> void {
        const auto type = static_cast<int>(static_cast<std::uint32_t>(chain));

        if (enabled) {
            emit chainEnabled(type);
        } else {
            emit chainDisabled(type);
        }

        emit enabledChanged(static_cast<int>(total));
    });
}

auto BlockchainSelectionQt::disableChain(const int chain) noexcept -> bool
{
    return imp_->parent_.Disable(static_cast<blockchain::Type>(chain));
}

auto BlockchainSelectionQt::enableChain(const int chain) noexcept -> bool
{
    return imp_->parent_.Enable(static_cast<blockchain::Type>(chain));
}

auto BlockchainSelectionQt::enabledCount() const noexcept -> int
{
    return static_cast<int>(imp_->parent_.EnabledCount());
}

auto BlockchainSelectionQt::flags(const QModelIndex& index) const
    -> Qt::ItemFlags
{
    return Model::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}

auto BlockchainSelectionQt::setData(
    const QModelIndex& index,
    const QVariant& value,
    int role) -> bool
{
    if (false == index.isValid()) { return false; }

    if (role == Qt::CheckStateRole) {
        const auto chain = data(index, TypeRole).toInt();

        if (static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked) {

            return enableChain(chain);
        } else {

            return disableChain(chain);
        }
    }

    return false;
}

BlockchainSelectionQt::~BlockchainSelectionQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto BlockchainSelectionItem::qt_data(
    const int column,
    const int role,
    QVariant& out) const noexcept -> void
{
    using Parent = BlockchainSelectionQt;

    if (0 != column) { return; }

    switch (role) {
        case Qt::DisplayRole: {
            qt_data(column, Parent::NameRole, out);
        } break;
        case Qt::CheckStateRole: {
            out = IsEnabled() ? Qt::Checked : Qt::Unchecked;
        } break;
        case BlockchainSelectionQt::NameRole: {
            out = Name().c_str();
        } break;
        case BlockchainSelectionQt::TypeRole: {
            out = static_cast<int>(static_cast<std::uint32_t>(Type()));
        } break;
        case BlockchainSelectionQt::IsEnabled: {
            out = IsEnabled();
        } break;
        case BlockchainSelectionQt::IsTestnet: {
            out = IsTestnet();
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
