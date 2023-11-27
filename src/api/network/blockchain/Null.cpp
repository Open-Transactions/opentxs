// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/network/internal.factory.hpp"  // IWYU pragma: associated

#include "opentxs/util/Writer.hpp"

namespace opentxs::factory
{
auto BlockchainNetworkAPI(
    const api::session::Client&,
    const api::session::Endpoints&,
    const opentxs::network::zeromq::Context&) noexcept
    -> std::unique_ptr<api::network::Blockchain>
{
    return BlockchainNetworkAPINull();
}
}  // namespace opentxs::factory
