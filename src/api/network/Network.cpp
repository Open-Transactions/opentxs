// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "opentxs/api/network/Network.hpp"  // IWYU pragma: associated

#include <memory>

#include "api/network/Network.hpp"
#include "api/session/base/Scheduler.hpp"
#include "internal/api/network/Factory.hpp"

namespace opentxs::factory
{
auto NetworkAPI(
    const api::Session& api,
    const api::network::Asio& asio,
    const network::zeromq::Context& zmq,
    const api::session::Endpoints& endpoints,
    api::network::Blockchain::Imp* blockchain,
    api::session::Scheduler& config,
    bool dhtDefault) noexcept -> api::network::Network::Imp*
{
    using ReturnType = api::network::Network;

    return std::make_unique<ReturnType::Imp>(
               api,
               asio,
               zmq,
               endpoints,
               blockchain,
               dhtDefault,
               config.nym_publish_interval_,
               config.nym_refresh_interval_,
               config.server_publish_interval_,
               config.server_refresh_interval_,
               config.unit_publish_interval_,
               config.unit_refresh_interval_)
        .release();
}
}  // namespace opentxs::factory

namespace opentxs::api::network
{
Network::Network(Imp* imp) noexcept
    : imp_(imp)
{
}

auto Network::Asio() const noexcept -> const network::Asio&
{
    return imp_->asio_;
}

auto Network::Blockchain() const noexcept -> const network::Blockchain&
{
    return imp_->blockchain_;
}

auto Network::DHT() const noexcept -> const network::Dht&
{
    return *imp_->dht_;
}

auto Network::Shutdown() noexcept -> void { imp_->dht_.reset(); }

auto Network::ZeroMQ() const noexcept
    -> const opentxs::network::zeromq::Context&
{
    return imp_->zmq_;
}

Network::~Network()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::api::network
