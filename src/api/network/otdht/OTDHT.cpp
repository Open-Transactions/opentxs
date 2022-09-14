// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "api/network/otdht/OTDHT.hpp"       // IWYU pragma: associated
#include "internal/api/network/Factory.hpp"  // IWYU pragma: associated

#include "internal/api/network/Blockchain.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/network/otdht/Node.hpp"
#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Options.hpp"
#include "util/Work.hpp"

namespace opentxs::factory
{
auto OTDHT(
    const api::Session& api,
    const network::zeromq::Context& zmq,
    const api::session::Endpoints& endpoints,
    const api::network::Blockchain& blockchain) noexcept
    -> std::unique_ptr<api::network::OTDHT>
{
    using ReturnType = api::network::implementation::OTDHT;

    return std::make_unique<ReturnType>(api, zmq, endpoints, blockchain);
}
}  // namespace opentxs::factory

namespace opentxs::api::network::implementation
{
OTDHT::OTDHT(
    const api::Session& api,
    const opentxs::network::zeromq::Context& zmq,
    const api::session::Endpoints& endpoints,
    const api::network::Blockchain& blockchain) noexcept
    : api_(api)
    , blockchain_(blockchain)
    , to_node_([&] {
        auto out = zmq.Internal().RawSocket(
            opentxs::network::zeromq::socket::Type::Push);
        const auto rc =
            out.Connect(endpoints.Internal().OTDHTNodePull().data());

        OT_ASSERT(rc);

        return out;
    }())
{
}

auto OTDHT::AddPeer(std::string_view endpoint) const noexcept -> bool
{
    return blockchain_.Internal().AddSyncServer(endpoint);
}

auto OTDHT::DeletePeer(std::string_view endpoint) const noexcept -> bool
{
    return blockchain_.Internal().DeleteSyncServer(endpoint);
}

auto OTDHT::KnownPeers(alloc::Default alloc) const noexcept -> Endpoints
{
    return blockchain_.Internal().GetSyncServers(alloc);
}

auto OTDHT::Start(std::shared_ptr<const api::Session> api) noexcept -> void
{
    static const auto defaultServers = Vector<CString>{
        "tcp://metier1.opentransactions.org:8814",
        "tcp://metier2.opentransactions.org:8814",
        "tcp://ot01.matterfi.net:8814",
    };
    const auto& options = api_.GetOptions();
    const auto existing = [&] {
        auto out = Set<CString>{};

        // TODO allocator
        for (const auto& server : KnownPeers({})) {
            // TODO GetSyncServers should return pmr strings
            out.emplace(server.c_str());
        }

        for (const auto& server : defaultServers) {
            if (0 == out.count(server)) {
                if (false == options.TestMode()) { AddPeer(server); }

                out.emplace(server);
            }
        }

        return out;
    }();

    try {
        for (const auto& endpoint : options.RemoteBlockchainSyncServers()) {
            if (0 == existing.count(endpoint)) { AddPeer(endpoint); }
        }
    } catch (...) {
    }

    opentxs::network::otdht::Node{api_}.Init(api);
}

auto OTDHT::StartListener(
    std::string_view syncEndpoint,
    std::string_view publicSyncEndpoint,
    std::string_view updateEndpoint,
    std::string_view publicUpdateEndpoint) const noexcept -> bool
{
    return to_node_.lock()->SendDeferred(
        [&] {
            using Job = opentxs::network::otdht::NodeJob;
            auto out = MakeWork(Job::add_listener);
            out.AddFrame(syncEndpoint.data(), syncEndpoint.size());
            out.AddFrame(publicSyncEndpoint.data(), publicSyncEndpoint.size());
            out.AddFrame(updateEndpoint.data(), updateEndpoint.size());
            out.AddFrame(
                publicUpdateEndpoint.data(), publicUpdateEndpoint.size());

            return out;
        }(),
        __FILE__,
        __LINE__);
}

OTDHT::~OTDHT() = default;
}  // namespace opentxs::api::network::implementation
