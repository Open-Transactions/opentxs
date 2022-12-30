// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <atomic>
#include <cstddef>
#include <future>
#include <memory>

namespace ottest
{
namespace ot = opentxs;

class OPENTXS_EXPORT PeerListener
{
    std::promise<void> promise_;

public:
    std::future<void> done_;
    std::atomic<std::size_t> miner_1_peers_;
    std::atomic<std::size_t> sync_server_peers_;
    std::atomic<std::size_t> client_1_peers_;
    std::atomic<std::size_t> client_2_peers_;

    PeerListener(
        const bool waitForHandshake,
        const int clientCount,
        const ot::api::session::Client& miner,
        const ot::api::session::Client& syncServer,
        const ot::api::session::Client& client1,
        const ot::api::session::Client& client2);

    ~PeerListener();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace ottest
