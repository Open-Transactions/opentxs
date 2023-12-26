// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/Context.hpp"  // IWYU pragma: associated

#include <QQmlEngine>  // IWYU pragma: keep

#include "opentxs/UnitType.hpp"  // IWYU pragma: keep

namespace opentxs
{
auto RegisterQMLTypes() noexcept -> void
{
    qmlRegisterUncreatableMetaObject(
        opentxs::unittype::staticMetaObject,
        "org.opentransactions.unittype",
        VersionMajor(),
        VersionMinor(),
        "UnitType",
        "Access to opentxs::UnitType enum class");
}
}  // namespace opentxs
