// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/filteroracle/Data.hpp"  // IWYU pragma: associated

#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::filteroracle
{
Data::Data(
    const api::Session& api,
    const node::Endpoints& endpoints,
    database::Cfilter& db) noexcept
    : last_sync_progress_()
    , last_broadcast_()  // TODO allocator
    , to_blockchain_api_([&] {
        using Type = opentxs::network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto endpoint = UnallocatedCString{
            api.Endpoints().Internal().Internal().BlockchainMessageRouter()};
        const auto rc = out.Connect(endpoint.c_str());

        assert_true(rc);

        return out;
    }())
    , filter_notifier_internal_([&] {
        using Socket = network::zeromq::socket::Type;
        auto socket =
            api.Network().ZeroMQ().Internal().RawSocket(Socket::Publish);
        auto rc = socket.Bind(endpoints.new_filter_publish_.c_str());

        assert_true(rc);

        return socket;
    }())
    , reindex_blocks_([&] {
        using Socket = network::zeromq::socket::Type;
        auto socket =
            api.Network().ZeroMQ().Internal().RawSocket(Socket::Publish);
        auto rc = socket.Bind(endpoints.filter_oracle_reindex_publish_.c_str());

        assert_true(rc);

        return socket;
    }())
    , db_(db)
    , init_promise_()
    , init_(init_promise_.get_future())
{
}

auto Data::DB() const noexcept -> const database::Cfilter&
{
    init_.get();

    return db_;
}

auto Data::DB() noexcept -> database::Cfilter&
{
    init_.get();

    return db_;
}

auto Data::Init() noexcept -> void { init_promise_.set_value(); }

Data::~Data() = default;
}  // namespace opentxs::blockchain::node::filteroracle
