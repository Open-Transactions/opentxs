// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/interface/ui/UI.hpp"  // IWYU pragma: associated

#include <QQmlEngine>

namespace opentxs::ui
{
auto claim_ownership(QObject* object) noexcept -> void
{
    QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
}
}  // namespace opentxs::ui
