// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QObject>

#include "opentxs/Export.hpp"
#include "opentxs/interface/qt/Model.hpp"
#include "opentxs/interface/qt/QML.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
namespace internal
{
struct SeedList;
}  // namespace internal
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT SeedListQt final : public qt::Model
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        SeedIDRole = Qt::UserRole + 0,    // QString
        SeedNameRole = Qt::UserRole + 1,  // QString
        SeedTypeRole = Qt::UserRole + 2,  // int (crypto::SeedStyle)
    };
    Q_ENUM(Roles)
    enum Columns {
        NameColumn = 0,
    };
    Q_ENUM(Columns)

    OPENTXS_NO_EXPORT SeedListQt(internal::SeedList& parent) noexcept;
    SeedListQt(const SeedListQt&) = delete;
    SeedListQt(SeedListQt&&) = delete;
    auto operator=(const SeedListQt&) -> SeedListQt& = delete;
    auto operator=(SeedListQt&&) -> SeedListQt& = delete;

    OPENTXS_NO_EXPORT ~SeedListQt() final;

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::ui
