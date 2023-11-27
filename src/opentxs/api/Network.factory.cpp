// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/internal.factory.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "opentxs/api/NetworkPrivate.hpp"

namespace opentxs::factory
{
auto NetworkAPI(
    const api::internal::Session& api,
    const api::network::Asio& asio,
    const network::zeromq::Context& zmq,
    const api::network::ZAP& zap,
    const api::session::Endpoints& endpoints,
    std::unique_ptr<api::network::Blockchain> blockchain) noexcept
    -> api::internal::Network*
{
    using ReturnType = api::NetworkPrivate;

    return std::make_unique<ReturnType>(
               api, asio, zmq, zap, endpoints, std::move(blockchain))
        .release();
}
}  // namespace opentxs::factory
