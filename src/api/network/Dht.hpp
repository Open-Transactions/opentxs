// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <utility>

#include "internal/api/network/Dht.hpp"
#include "network/DhtConfig.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/network/Dht.hpp"
#include "opentxs/contract/Notary.hpp"
#include "opentxs/network/OpenDHT.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"

namespace opentxs
{
namespace api
{
namespace network
{
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

namespace proto
{
class Nym;
class ServerContract;
class UnitDefinition;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::api::network::imp
{
class Dht final : virtual public internal::Dht
{
public:
    auto GetPublicNym(const std::string& key) const noexcept -> void final;
    auto GetServerContract(const std::string& key) const noexcept -> void final;
    auto GetUnitDefinition(const std::string& key) const noexcept -> void final;
    auto Insert(const std::string& key, const std::string& value) const noexcept
        -> void final;
    auto Insert(const proto::Nym& nym) const noexcept -> void final;
    auto Insert(const proto::ServerContract& contract) const noexcept
        -> void final;
    auto Insert(const proto::UnitDefinition& contract) const noexcept
        -> void final;
    auto OpenDHT() const noexcept -> const opentxs::network::OpenDHT& final;
    auto RegisterCallbacks(const CallbackMap& callbacks) const noexcept
        -> void final;

    Dht(const api::Session& api,
        const opentxs::network::zeromq::Context& zeromq,
        const api::session::Endpoints& endpoints,
        opentxs::network::DhtConfig&& config) noexcept;

    ~Dht() final = default;

private:
    const api::Session& api_;
    const opentxs::network::DhtConfig config_;
    mutable std::shared_mutex lock_;
    mutable CallbackMap callback_map_;
    std::unique_ptr<opentxs::network::OpenDHT> node_;
    OTZMQReplyCallback request_nym_callback_;
    OTZMQReplySocket request_nym_socket_;
    OTZMQReplyCallback request_server_callback_;
    OTZMQReplySocket request_server_socket_;
    OTZMQReplyCallback request_unit_callback_;
    OTZMQReplySocket request_unit_socket_;

    static auto ProcessPublicNym(
        const api::Session& api,
        const std::string key,
        const DhtResults& values,
        NotifyCB notifyCB) noexcept -> bool;
    static auto ProcessServerContract(
        const api::Session& api,
        const std::string key,
        const DhtResults& values,
        NotifyCB notifyCB) noexcept -> bool;
    static auto ProcessUnitDefinition(
        const api::Session& api,
        const std::string key,
        const DhtResults& values,
        NotifyCB notifyCB) noexcept -> bool;

    auto process_request(
        const opentxs::network::zeromq::Message& incoming,
        void (Dht::*get)(const std::string&) const) const noexcept
        -> opentxs::network::zeromq::Message;

    Dht() = delete;
    Dht(const Dht&) = delete;
    Dht(Dht&&) = delete;
    auto operator=(const Dht&) -> Dht& = delete;
    auto operator=(Dht&&) -> Dht& = delete;
};
}  // namespace opentxs::api::network::imp
