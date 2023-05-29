// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/api/session/Endpoints.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session
{
class ZMQ
{
public:
    ZMQ() = delete;
    ZMQ(const ZMQ&) = delete;
    ZMQ(ZMQ&&) = delete;
    auto operator=(const ZMQ&) -> ZMQ& = delete;
    auto operator=(ZMQ&&) -> ZMQ& = delete;

    virtual ~ZMQ() = default;

protected:
    const opentxs::network::zeromq::Context& zmq_context_;
    const int instance_;

private:
    std::unique_ptr<api::session::Endpoints> endpoints_p_;

protected:
    const api::session::Endpoints& endpoints_;

    ZMQ(const api::Crypto& crypto,
        const opentxs::network::zeromq::Context& zmq,
        const int instance) noexcept;
};
}  // namespace opentxs::api::session
