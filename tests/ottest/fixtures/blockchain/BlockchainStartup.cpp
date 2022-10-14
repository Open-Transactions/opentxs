// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/BlockchainStartup.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <cstdlib>

#include "internal/api/session/Endpoints.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/util/Pimpl.hpp"
#include "util/Work.hpp"

namespace ottest
{
class BlockchainStartup::Imp
{
private:
    using Promise = std::promise<void>;
    const ot::blockchain::Type chain_;
    Promise block_oracle_promise_;
    Promise block_oracle_downloader_promise_;
    Promise fee_oracle_promise_;
    Promise filter_oracle_promise_;
    Promise filter_oracle_filter_downloader_promise_;
    Promise filter_oracle_header_downloader_promise_;
    Promise filter_oracle_indexer_promise_;
    Promise node_promise_;
    Promise peer_manager_promise_;
    Promise sync_server_promise_;
    Promise wallet_promise_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQSubscribeSocket socket_;

    auto cb(ot::network::zeromq::Message&& in) noexcept -> void
    {
        const auto body = in.Body();
        const auto type = body.at(0).as<ot::OTZMQWorkType>();
        const auto chain = body.at(1).as<ot::blockchain::Type>();

        if (chain != chain_) { return; }

        switch (type) {
            case ot::OT_ZMQ_BLOCKCHAIN_NODE_READY: {
                node_promise_.set_value();
            } break;
            case ot::OT_ZMQ_SYNC_SERVER_BACKEND_READY: {
                sync_server_promise_.set_value();
            } break;
            case ot::OT_ZMQ_BLOCK_ORACLE_READY: {
                block_oracle_promise_.set_value();
            } break;
            case ot::OT_ZMQ_BLOCK_ORACLE_DOWNLOADER_READY: {
                block_oracle_downloader_promise_.set_value();
            } break;
            case ot::OT_ZMQ_FILTER_ORACLE_READY: {
                filter_oracle_promise_.set_value();
            } break;
            case ot::OT_ZMQ_FILTER_ORACLE_INDEXER_READY: {
                filter_oracle_indexer_promise_.set_value();
            } break;
            case ot::OT_ZMQ_FILTER_ORACLE_FILTER_DOWNLOADER_READY: {
                filter_oracle_filter_downloader_promise_.set_value();
            } break;
            case ot::OT_ZMQ_FILTER_ORACLE_HEADER_DOWNLOADER_READY: {
                filter_oracle_header_downloader_promise_.set_value();
            } break;
            case ot::OT_ZMQ_PEER_MANAGER_READY: {
                peer_manager_promise_.set_value();
            } break;
            case ot::OT_ZMQ_BLOCKCHAIN_WALLET_READY: {
                wallet_promise_.set_value();
            } break;
            case ot::OT_ZMQ_FEE_ORACLE_READY: {
                fee_oracle_promise_.set_value();
            } break;
            default: {
                abort();
            }
        }
    }

public:
    Future block_oracle_;
    Future block_oracle_downloader_;
    Future fee_oracle_;
    Future filter_oracle_;
    Future filter_oracle_filter_downloader_;
    Future filter_oracle_header_downloader_;
    Future filter_oracle_indexer_;
    Future node_;
    Future peer_manager_;
    Future sync_server_;
    Future wallet_;

    Imp(const ot::api::Session& api, const ot::blockchain::Type chain) noexcept
        : chain_(chain)
        , block_oracle_promise_()
        , block_oracle_downloader_promise_()
        , fee_oracle_promise_()
        , filter_oracle_promise_()
        , filter_oracle_filter_downloader_promise_()
        , filter_oracle_header_downloader_promise_()
        , filter_oracle_indexer_promise_()
        , node_promise_()
        , peer_manager_promise_()
        , sync_server_promise_()
        , wallet_promise_()
        , cb_(ot::network::zeromq::ListenCallback::Factory(
              [this](auto&& in) { cb(std::move(in)); }))
        , socket_([&] {
            auto out = api.Network().ZeroMQ().Internal().SubscribeSocket(cb_);
            out->Start(ot::UnallocatedCString{
                api.Endpoints().Internal().BlockchainStartupPublish()});

            return out;
        }())
        , block_oracle_(block_oracle_promise_.get_future())
        , block_oracle_downloader_(
              block_oracle_downloader_promise_.get_future())
        , fee_oracle_(fee_oracle_promise_.get_future())
        , filter_oracle_(filter_oracle_promise_.get_future())
        , filter_oracle_filter_downloader_(
              filter_oracle_filter_downloader_promise_.get_future())
        , filter_oracle_header_downloader_(
              filter_oracle_header_downloader_promise_.get_future())
        , filter_oracle_indexer_(filter_oracle_indexer_promise_.get_future())
        , node_(node_promise_.get_future())
        , peer_manager_(peer_manager_promise_.get_future())
        , sync_server_(sync_server_promise_.get_future())
        , wallet_(wallet_promise_.get_future())
    {
    }
};
}  // namespace ottest

namespace ottest
{
BlockchainStartup::BlockchainStartup(
    const ot::api::Session& api,
    const ot::blockchain::Type chain) noexcept
    : imp_(std::make_unique<Imp>(api, chain))
{
}

auto BlockchainStartup::SyncServer() const noexcept -> Future
{
    return imp_->sync_server_;
}

BlockchainStartup::~BlockchainStartup() = default;
}  // namespace ottest
