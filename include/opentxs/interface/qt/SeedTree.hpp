// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <QMetaObject>
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
struct SeedTree;
}  // namespace internal

class SeedTreeQt;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class OPENTXS_EXPORT opentxs::ui::SeedTreeQt final : public qt::Model
{
    Q_OBJECT
    Q_PROPERTY(QString defaultNym READ defaultNym NOTIFY defaultNymChanged)
    Q_PROPERTY(QString defaultSeed READ defaultSeed NOTIFY defaultSeedChanged)

signals:
    void defaultNymChanged(QString) const;
    void defaultSeedChanged(QString) const;
    void needNym() const;
    void needSeed() const;
    void ready();

public slots:
    void check();

public:
    enum Roles {
        SeedIDRole = Qt::UserRole + 0,    // QString
        SeedNameRole = Qt::UserRole + 1,  // QString
        SeedTypeRole = Qt::UserRole + 2,  // int (crypto::SeedStyle)
        NymIDRole = Qt::UserRole + 3,     // QString
        NymNameRole = Qt::UserRole + 4,   // QString
        NymIndexRole = Qt::UserRole + 5,  // unsigned long long
    };
    enum Columns {
        NameColumn = 0,
    };

    auto defaultNym() const noexcept -> QString;
    auto defaultSeed() const noexcept -> QString;

    SeedTreeQt(internal::SeedTree& parent) noexcept;
    SeedTreeQt(const SeedTreeQt&) = delete;
    SeedTreeQt(SeedTreeQt&&) = delete;
    auto operator=(const SeedTreeQt&) -> SeedTreeQt& = delete;
    auto operator=(SeedTreeQt&&) -> SeedTreeQt& = delete;

    ~SeedTreeQt() final;

private:
    struct Imp;

    Imp* imp_;
};
