// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/node/filteroracle/Data.hpp"  // IWYU pragma: associated

#include "blockchain/node/filteroracle/CfheaderDownloader.hpp"
#include "blockchain/node/filteroracle/CfilterDownloader.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node::filteroracle
{
Data::Data(const api::Session& api, const node::Manager& node) noexcept
    : db_()
    , last_sync_progress_()
    , last_broadcast_()  // TODO allocator
    , to_blockchain_api_([&] {
        using Type = opentxs::network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto endpoint = UnallocatedCString{
            api.Endpoints().Internal().Internal().BlockchainMessageRouter()};
        const auto rc = out.Connect(endpoint.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , filter_notifier_internal_([&] {
        using Socket = network::zeromq::socket::Type;
        auto socket =
            api.Network().ZeroMQ().Internal().RawSocket(Socket::Publish);
        auto rc = socket.Bind(
            node.Internal().Endpoints().new_filter_publish_.c_str());

        OT_ASSERT(rc);

        return socket;
    }())
    , reindex_blocks_([&] {
        using Socket = network::zeromq::socket::Type;
        auto socket =
            api.Network().ZeroMQ().Internal().RawSocket(Socket::Publish);
        auto rc = socket.Bind(
            node.Internal().Endpoints().filter_oracle_reindex_publish_.c_str());

        OT_ASSERT(rc);

        return socket;
    }())
    , filter_downloader_()
    , header_downloader_()
{
}

Data::~Data() = default;
}  // namespace opentxs::blockchain::node::filteroracle
