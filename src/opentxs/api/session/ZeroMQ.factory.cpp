// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/internal.factory.hpp"  // IWYU pragma: associated

#include "opentxs/api/session/ZeroMQPrivate.hpp"

namespace opentxs::factory
{
auto SessionZMQ(const api::Session& api, const Flag& running) noexcept
    -> api::session::internal::ZeroMQ*
{
    return std::make_unique<api::session::ZeroMQPrivate>(api, running)
        .release();
}
}  // namespace opentxs::factory
