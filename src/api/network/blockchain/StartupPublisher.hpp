// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/Handle.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Endpoints;
}  // namespace session
}  // namespace api

namespace network
{
namespace zeromq
{
namespace internal
{
class Batch;
class Context;
class Thread;
}  // namespace internal

namespace socket
{
class Raw;
}  // namespace socket

class Context;
class ListenCallback;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::blockchain
{
class StartupPublisher
{
public:
    StartupPublisher(
        const api::session::Endpoints& endpoints,
        const opentxs::network::zeromq::Context& zmq) noexcept;
    StartupPublisher(const StartupPublisher&) = delete;
    StartupPublisher(StartupPublisher&&) = delete;
    auto operator=(const StartupPublisher&) -> StartupPublisher& = delete;
    auto operator=(StartupPublisher&&) -> StartupPublisher& = delete;

    ~StartupPublisher();

private:
    using Callback = opentxs::network::zeromq::ListenCallback;

    const opentxs::network::zeromq::internal::Context& zmq_;
    opentxs::network::zeromq::internal::Handle handle_;
    opentxs::network::zeromq::internal::Batch& batch_;
    const Callback& cb_;
    opentxs::network::zeromq::socket::Raw& publish_;
    opentxs::network::zeromq::socket::Raw& pull_;
    opentxs::network::zeromq::internal::Thread* thread_;
};
}  // namespace opentxs::api::network::blockchain
