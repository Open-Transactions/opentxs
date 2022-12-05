// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/context/Context.hpp"  // IWYU pragma: associated

#include <QObject>
#include <memory>

namespace opentxs::api::imp
{
auto Context::get_qt() const noexcept -> std::unique_ptr<QObject>&
{
    static auto qt = std::make_unique<QObject>();

    return qt;
}

auto Context::shutdown_qt() noexcept -> void { get_qt().reset(); }

auto Context::QtRootObject() const noexcept -> QObject*
{
    auto& qt = get_qt();

    if (auto* parent = args_.QtRootObject(); qt && (nullptr != parent)) {
        if (qt->thread() != parent->thread()) {
            qt->moveToThread(parent->thread());
        }
    }

    return qt.get();
}
}  // namespace opentxs::api::imp
