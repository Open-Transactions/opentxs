// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/Types.hpp"

#include "ottest/fixtures/blockchain/PeerListener.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <mutex>
#include <span>
#include <stdexcept>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/util/Mutex.hpp"

namespace ottest
{
struct PeerListener::Imp {
    PeerListener& parent_;
    const std::size_t client_count_;
    mutable std::mutex lock_;
    ot::OTZMQListenCallback miner_1_cb_;
    ot::OTZMQListenCallback ss_cb_;
    ot::OTZMQListenCallback client_1_cb_;
    ot::OTZMQListenCallback client_2_cb_;
    ot::OTZMQSubscribeSocket m_socket_;
    ot::OTZMQSubscribeSocket ss_socket_;
    ot::OTZMQSubscribeSocket c1_socket_;
    ot::OTZMQSubscribeSocket c2_socket_;

    auto cb(
        ot::network::zeromq::Message&& msg,
        std::atomic<std::size_t>& counter) noexcept -> void
    {
        counter.store([&] {
            const auto payload = msg.Payload();

            switch (payload[0].as<ot::WorkType>()) {
                case ot::WorkType::BlockchainPeerAdded: {
                    opentxs::assert_true(3 < payload.size());

                    return payload[3].as<std::size_t>();
                }
                case ot::WorkType::BlockchainPeerConnected: {
                    opentxs::assert_true(2 < payload.size());

                    return payload[2].as<std::size_t>();
                }
                default: {
                    opentxs::LogAbort()().Abort();
                }
            }
        }());
        auto lock = ot::Lock{lock_};
        const auto target = client_count_ + 1;

        if (target != parent_.miner_1_peers_) { return; }

        if (0 == parent_.sync_server_peers_) { return; }

        if ((0 < client_count_) && (0 == parent_.client_1_peers_)) { return; }

        if ((1 < client_count_) && (0 == parent_.client_2_peers_)) { return; }

        try {
            parent_.promise_.set_value();
        } catch (...) {
        }
    }

    Imp(PeerListener& parent,
        const bool waitForHandshake,
        const int clientCount,
        const ot::api::session::Client& miner,
        const ot::api::session::Client& syncServer,
        const ot::api::session::Client& client1,
        const ot::api::session::Client& client2)
        : parent_(parent)
        , client_count_(clientCount)
        , lock_()
        , miner_1_cb_(
              ot::network::zeromq::ListenCallback::Factory([this](auto&& msg) {
                  cb(std::move(msg), parent_.miner_1_peers_);
              }))
        , ss_cb_(
              ot::network::zeromq::ListenCallback::Factory([this](auto&& msg) {
                  cb(std::move(msg), parent_.sync_server_peers_);
              }))
        , client_1_cb_(
              ot::network::zeromq::ListenCallback::Factory([this](auto&& msg) {
                  cb(std::move(msg), parent_.client_1_peers_);
              }))
        , client_2_cb_(
              ot::network::zeromq::ListenCallback::Factory([this](auto&& msg) {
                  cb(std::move(msg), parent_.client_2_peers_);
              }))
        , m_socket_(
              miner.Network().ZeroMQ().Context().Internal().SubscribeSocket(
                  miner_1_cb_))
        , ss_socket_(syncServer.Network()
                         .ZeroMQ()
                         .Context()
                         .Internal()
                         .SubscribeSocket(ss_cb_))
        , c1_socket_(
              client1.Network().ZeroMQ().Context().Internal().SubscribeSocket(
                  client_1_cb_))
        , c2_socket_(
              client2.Network().ZeroMQ().Context().Internal().SubscribeSocket(
                  client_2_cb_))
    {
        if (false == m_socket_->Start(
                         (waitForHandshake
                              ? miner.Endpoints().BlockchainPeer()
                              : miner.Endpoints().BlockchainPeerConnection())
                             .data())) {
            throw std::runtime_error("Error connecting to miner socket");
        }

        if (false ==
            ss_socket_->Start(
                (waitForHandshake
                     ? syncServer.Endpoints().BlockchainPeer()
                     : syncServer.Endpoints().BlockchainPeerConnection())
                    .data())) {
            throw std::runtime_error("Error connecting to sync server socket");
        }

        if (false == c1_socket_->Start(
                         (waitForHandshake
                              ? client1.Endpoints().BlockchainPeer()
                              : client1.Endpoints().BlockchainPeerConnection())
                             .data())) {
            throw std::runtime_error("Error connecting to client1 socket");
        }

        if (false == c2_socket_->Start(
                         (waitForHandshake
                              ? client2.Endpoints().BlockchainPeer()
                              : client2.Endpoints().BlockchainPeerConnection())
                             .data())) {
            throw std::runtime_error("Error connecting to client2 socket");
        }
    }

    ~Imp()
    {
        c2_socket_->Close();
        c1_socket_->Close();
        ss_socket_->Close();
        m_socket_->Close();
    }
};
}  // namespace ottest

namespace ottest
{
PeerListener::PeerListener(
    const bool waitForHandshake,
    const int clientCount,
    const ot::api::session::Client& miner,
    const ot::api::session::Client& syncServer,
    const ot::api::session::Client& client1,
    const ot::api::session::Client& client2)
    : promise_()
    , done_(promise_.get_future())
    , miner_1_peers_(0)
    , sync_server_peers_(0)
    , client_1_peers_(0)
    , client_2_peers_(0)
    , imp_(std::make_unique<Imp>(
          *this,
          waitForHandshake,
          clientCount,
          miner,
          syncServer,
          client1,
          client2))
{
}

PeerListener::~PeerListener() = default;
}  // namespace ottest
