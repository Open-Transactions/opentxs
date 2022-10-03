// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "opentxs/api/network/Network.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "api/network/Network.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/api/network/Factory.hpp"
#include "internal/api/network/OTDHT.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Blockchain.hpp"

namespace opentxs::factory
{
auto NetworkAPI(
    const api::Session& api,
    const api::network::Asio& asio,
    const network::zeromq::Context& zmq,
    const api::session::Endpoints& endpoints,
    std::unique_ptr<api::network::Blockchain> blockchain) noexcept
    -> std::unique_ptr<api::network::Network>
{
    using ReturnType = api::network::implementation::Network;

    return std::make_unique<ReturnType>(
        api, asio, zmq, endpoints, std::move(blockchain));
}
}  // namespace opentxs::factory

namespace opentxs::api::network::implementation
{
Network::Network(
    const api::Session& api,
    const network::Asio& asio,
    const opentxs::network::zeromq::Context& zmq,
    const api::session::Endpoints& endpoints,
    std::unique_ptr<api::network::Blockchain> blockchain) noexcept
    : asio_(asio)
    , zmq_(zmq)
    , blockchain_(std::move(blockchain))
    , otdht_(factory::OTDHT(api, zmq, endpoints, *blockchain_))
{
    OT_ASSERT(blockchain_);
    OT_ASSERT(otdht_);
}

auto Network::Start(
    std::shared_ptr<const api::Session> api,
    const api::crypto::Blockchain& crypto,
    const api::Legacy& legacy,
    const std::filesystem::path& dataFolder,
    const Options& args) noexcept -> void
{
    blockchain_->Internal().Init(api, crypto, legacy, dataFolder, args);
    otdht_->Internal().Start(api);
}

auto Network::Shutdown() noexcept -> void
{
    blockchain_->Internal().Shutdown();
}

Network::~Network() = default;
}  // namespace opentxs::api::network::implementation
