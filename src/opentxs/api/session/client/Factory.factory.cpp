// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/internal.factory.hpp"  // IWYU pragma: associated

#include <exception>

#include "opentxs/api/session/client/FactoryPrivate.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto SessionFactoryAPI(
    const api::session::internal::Client& api,
    const api::Factory& parent) noexcept
    -> std::unique_ptr<api::session::internal::Factory>
{
    using ReturnType = api::session::client::FactoryPrivate;

    try {

        return std::make_unique<ReturnType>(api, parent);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory
