// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/internal.factory.hpp"  // IWYU pragma: associated

#include "api/session/ui/Imp-qt.hpp"
#include "api/session/ui/UI.hpp"

namespace opentxs::factory
{
auto UI(
    const api::session::Client& api,
    const api::crypto::Blockchain& blockchain,
    const Flag& running) noexcept -> std::unique_ptr<api::session::UI>
{
    using ReturnType = api::session::imp::UI;

    return std::make_unique<ReturnType>(
        std::make_unique<api::session::ui::ImpQt>(api, blockchain, running));
}
}  // namespace opentxs::factory
