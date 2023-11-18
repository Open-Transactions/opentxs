// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QMetaObject>
#include <QObject>

#include "opentxs/Export.hpp"
#include "opentxs/interface/qt/Model.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
namespace internal
{
struct UnitList;
}  // namespace internal
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT UnitListQt final : public qt::Model
{
    Q_OBJECT
public:
    enum Columns {
        UnitNameColumn = 0,
    };
    enum Roles {
        UnitIDRole = Qt::UserRole,
    };

    OPENTXS_NO_EXPORT UnitListQt(internal::UnitList& parent) noexcept;
    UnitListQt(const UnitListQt&) = delete;
    UnitListQt(UnitListQt&&) = delete;
    auto operator=(const UnitListQt&) -> UnitListQt& = delete;
    auto operator=(UnitListQt&&) -> UnitListQt& = delete;

    OPENTXS_NO_EXPORT ~UnitListQt() final;

private:
    struct Imp;

    Imp* imp_;
};
}  // namespace opentxs::ui
