// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "interface/ui/blockchainaccountstatus/BlockchainAccountStatus.hpp"  // IWYU pragma: associated
#include "interface/ui/blockchainaccountstatus/BlockchainSubaccount.hpp"  // IWYU pragma: associated
#include "interface/ui/blockchainaccountstatus/BlockchainSubaccountSource.hpp"  // IWYU pragma: associated
#include "interface/ui/blockchainaccountstatus/BlockchainSubchain.hpp"  // IWYU pragma: associated
#include "opentxs/interface/qt/BlockchainAccountStatus.hpp"  // IWYU pragma: associated

#include <QVariant>
#include <memory>

namespace opentxs::factory
{
auto BlockchainAccountStatusQtModel(
    ui::internal::BlockchainAccountStatus& parent) noexcept
    -> std::unique_ptr<ui::BlockchainAccountStatusQt>
{
    using ReturnType = ui::BlockchainAccountStatusQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct BlockchainAccountStatusQt::Imp {
    internal::BlockchainAccountStatus& parent_;

    Imp(internal::BlockchainAccountStatus& parent)
        : parent_(parent)
    {
    }
};

BlockchainAccountStatusQt::BlockchainAccountStatusQt(
    internal::BlockchainAccountStatus& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 1);
        internal_->SetRoleData({
            {BlockchainAccountStatusQt::NameRole, "name"},
            {BlockchainAccountStatusQt::SourceIDRole, "sourceid"},
            {BlockchainAccountStatusQt::SubaccountIDRole, "subaccountid"},
            {BlockchainAccountStatusQt::SubaccountTypeRole, "subaccounttype"},
            {BlockchainAccountStatusQt::SubchainTypeRole, "subchaintype"},
            {BlockchainAccountStatusQt::ProgressRole, "progress"},
        });
    }
}

auto BlockchainAccountStatusQt::getNym() const noexcept -> QString
{
    return imp_->parent_.Owner().str().c_str();
}

auto BlockchainAccountStatusQt::getChain() const noexcept -> int
{
    return static_cast<int>(imp_->parent_.Chain());
}

BlockchainAccountStatusQt::~BlockchainAccountStatusQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto BlockchainSubaccount::qt_data(
    const int column,
    const int role,
    QVariant& out) const noexcept -> void
{
    using Parent = BlockchainAccountStatusQt;

    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case Parent::NameColumn: {
                    qt_data(column, Parent::NameRole, out);
                } break;
                default: {
                }
            }
        } break;
        case Parent::NameRole: {
            out = Name().c_str();
        } break;
        case Parent::SubaccountIDRole: {
            out = SubaccountID().str().c_str();
        } break;
        default: {
        }
    };
}

auto BlockchainSubaccountSource::qt_data(
    const int column,
    const int role,
    QVariant& out) const noexcept -> void
{
    using Parent = BlockchainAccountStatusQt;

    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case Parent::NameColumn: {
                    qt_data(column, Parent::NameRole, out);
                } break;
                default: {
                }
            }
        } break;
        case Parent::NameRole: {
            out = Name().c_str();
        } break;
        case Parent::SourceIDRole: {
            out = SourceID().str().c_str();
        } break;
        case Parent::SubaccountTypeRole: {
            out = static_cast<int>(Type());
        } break;
        default: {
        }
    };
}

auto BlockchainSubchain::qt_data(
    const int column,
    const int role,
    QVariant& out) const noexcept -> void
{
    using Parent = BlockchainAccountStatusQt;

    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case Parent::NameColumn: {
                    qt_data(column, Parent::NameRole, out);
                } break;
                default: {
                }
            }
        } break;
        case Parent::NameRole: {
            out = Name().c_str();
        } break;
        case Parent::SubchainTypeRole: {
            out = static_cast<int>(Type());
        } break;
        case Parent::ProgressRole: {
            out = Progress().c_str();
        } break;
        default: {
        }
    };
}
}  // namespace opentxs::ui::implementation
