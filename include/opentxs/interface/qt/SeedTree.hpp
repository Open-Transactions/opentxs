// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QObject>
#include <QString>

#include "opentxs/Export.hpp"
#include "opentxs/interface/qt/Model.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
namespace internal
{
struct SeedTree;
}  // namespace internal

}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT SeedTreeQt final : public qt::Model
{
    Q_OBJECT
    Q_PROPERTY(QString defaultNym READ defaultNym NOTIFY defaultNymChanged)
    Q_PROPERTY(QString defaultSeed READ defaultSeed NOTIFY defaultSeedChanged)

Q_SIGNALS:
    void defaultNymChanged(QString) const;
    void defaultSeedChanged(QString) const;
    void needNym() const;
    void needSeed() const;
    void ready();

public Q_SLOTS:
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

    OPENTXS_NO_EXPORT SeedTreeQt(internal::SeedTree& parent) noexcept;
    SeedTreeQt(const SeedTreeQt&) = delete;
    SeedTreeQt(SeedTreeQt&&) = delete;
    auto operator=(const SeedTreeQt&) -> SeedTreeQt& = delete;
    auto operator=(SeedTreeQt&&) -> SeedTreeQt& = delete;

    OPENTXS_NO_EXPORT ~SeedTreeQt() final;

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::ui
