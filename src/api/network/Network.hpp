// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>

#include "api/network/Dht.hpp"
#include "internal/api/network/Factory.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Dht.hpp"
#include "opentxs/api/network/Network.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace network
{
class Asio;
class Dht;
}  // namespace network

namespace session
{
class Endpoints;
}  // namespace session

class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network
{
struct Network::Imp {
    const network::Asio& asio_;
    const opentxs::network::zeromq::Context& zmq_;
    std::unique_ptr<Dht> dht_;
    network::Blockchain blockchain_;

    Imp(const api::Session& api,
        const network::Asio& asio,
        const opentxs::network::zeromq::Context& zmq,
        const api::session::Endpoints& endpoints,
        api::network::Blockchain::Imp* blockchain,
        const bool dhtDefault,
        std::int64_t& nymPublishInterval,
        std::int64_t& nymRefreshInterval,
        std::int64_t& serverPublishInterval,
        std::int64_t& serverRefreshInterval,
        std::int64_t& unitPublishInterval,
        std::int64_t& unitRefreshInterval) noexcept
        : asio_(asio)
        , zmq_(zmq)
        , dht_(factory::DhtAPI(
              api,
              zmq_,
              endpoints,
              dhtDefault,
              nymPublishInterval,
              nymRefreshInterval,
              serverPublishInterval,
              serverRefreshInterval,
              unitPublishInterval,
              unitRefreshInterval))
        , blockchain_(blockchain)
    {
    }

private:
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::api::network
