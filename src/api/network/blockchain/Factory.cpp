// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/api/network/Factory.hpp"  // IWYU pragma: associated

#include <memory>

#include "api/network/blockchain/Blockchain.hpp"
#include "api/network/blockchain/Imp.hpp"
#include "opentxs/api/network/BlockchainHandle.hpp"

namespace opentxs::factory
{
auto BlockchainNetworkAPI(
    const api::session::Client& api,
    const api::session::Endpoints& endpoints,
    const opentxs::network::zeromq::Context& zmq) noexcept
    -> std::unique_ptr<api::network::Blockchain>
{
    using ReturnType = api::network::implementation::Blockchain;
    using Imp = api::network::implementation::BlockchainImp;

    return std::make_unique<ReturnType>(
        std::make_unique<Imp>(api, endpoints, zmq).release());
}
}  // namespace opentxs::factory
