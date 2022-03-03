// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <tuple>
#include <utility>

#include "core/Shutdown.hpp"
#include "opentxs/api/session/Endpoints.hpp"

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
class Context;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
class ZMQ
{
public:
    virtual ~ZMQ() = default;

protected:
    const opentxs::network::zeromq::Context& zmq_context_;
    const int instance_;

private:
    std::unique_ptr<api::session::Endpoints> endpoints_p_;

protected:
    const api::session::Endpoints& endpoints_;
    opentxs::internal::ShutdownSender shutdown_sender_;

    ZMQ(const opentxs::network::zeromq::Context& zmq, const int instance)
    noexcept;

private:
    ZMQ() = delete;
    ZMQ(const ZMQ&) = delete;
    ZMQ(ZMQ&&) = delete;
    auto operator=(const ZMQ&) -> ZMQ& = delete;
    auto operator=(ZMQ&&) -> ZMQ& = delete;
};
}  // namespace opentxs::api::session
