// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <zmq.h>
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

#include "ottest/fixtures/zeromq/Helpers.hpp"

namespace zmq = ot::network::zeromq;

namespace ottest
{
using namespace std::literals::chrono_literals;

class OPENTXS_EXPORT RouterRouterF : public ::testing::Test
{
public:
    RouterRouterF(const RouterRouterF&) = delete;
    RouterRouterF(RouterRouterF&&) = delete;
    auto operator=(const RouterRouterF&) -> RouterRouterF& = delete;
    auto operator=(RouterRouterF&&) -> RouterRouterF& = delete;

protected:
    using Socket = std::unique_ptr<void, decltype(&::zmq_close)>;

    const zmq::Context& context_;
    const ot::UnallocatedCString endpoint_;
    const int linger_;
    Socket server_;
    Socket client_;
    ZMQQueue queue_;
    std::atomic_bool running_;
    std::thread thread_;

    RouterRouterF();
    ~RouterRouterF() override;

private:
    auto thread() noexcept -> void;
};
}  // namespace ottest
