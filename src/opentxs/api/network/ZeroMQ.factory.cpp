// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/network/internal.factory.hpp"  // IWYU pragma: associated

#include "opentxs/api/network/ZeroMQPrivate.hpp"

namespace opentxs::factory
{
auto NetworkZMQ(
    const network::zeromq::Context& context,
    const api::network::ZAP& zap) noexcept -> api::network::internal::ZeroMQ*
{
    return std::make_unique<api::network::ZeroMQPrivate>(context, zap)
        .release();
}
}  // namespace opentxs::factory
