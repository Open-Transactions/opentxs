// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QString>

#include "opentxs/interface/qt/Model.hpp"

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
struct BlockchainAccountStatus;
}  // namespace internal

class BlockchainAccountStatusQt;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::ui::BlockchainAccountStatusQt final
    : public qt::Model
{
    Q_OBJECT
    Q_PROPERTY(QString nym READ getNym CONSTANT)
    Q_PROPERTY(int chain READ getChain CONSTANT)

public:
    enum Roles {
        NameRole = Qt::UserRole + 0,            // QString
        SourceIDRole = Qt::UserRole + 1,        // QString
        SubaccountIDRole = Qt::UserRole + 2,    // QString
        SubaccountTypeRole = Qt::UserRole + 3,  // int
        SubchainTypeRole = Qt::UserRole + 4,    // int
        ProgressRole = Qt::UserRole + 5,        // QString
    };
    // This model is designed to be used in a tree view
    enum Columns {
        NameColumn = 0,
    };

    auto getNym() const noexcept -> QString;
    auto getChain() const noexcept -> int;

    BlockchainAccountStatusQt(
        internal::BlockchainAccountStatus& parent) noexcept;

    ~BlockchainAccountStatusQt() final;

private:
    struct Imp;

    Imp* imp_;

    BlockchainAccountStatusQt() = delete;
    BlockchainAccountStatusQt(const BlockchainAccountStatusQt&) = delete;
    BlockchainAccountStatusQt(BlockchainAccountStatusQt&&) = delete;
    BlockchainAccountStatusQt& operator=(const BlockchainAccountStatusQt&) =
        delete;
    BlockchainAccountStatusQt& operator=(BlockchainAccountStatusQt&&) = delete;
};
