// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
namespace internal
{
class Network;
}

class Asio;
class Blockchain;
class OTDHT;
}  // namespace network
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

namespace opentxs::api::network
{
/**
 The top-level network API. Used for accessing the Asio API, the Blockchain
 network API, the DHT network API, and the ZeroMQ network API.
 */
class OPENTXS_EXPORT Network
{
public:
    virtual auto Asio() const noexcept -> const network::Asio& = 0;
    virtual auto Blockchain() const noexcept -> const network::Blockchain& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Network& = 0;
    virtual auto OTDHT() const noexcept -> const network::OTDHT& = 0;
    virtual auto ZeroMQ() const noexcept
        -> const opentxs::network::zeromq::Context& = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Network& = 0;

    Network(const Network&) = delete;
    Network(Network&&) = delete;
    auto operator=(const Network&) -> Network& = delete;
    auto operator=(Network&&) -> Network& = delete;

    OPENTXS_NO_EXPORT virtual ~Network() = default;

protected:
    Network() = default;
};
}  // namespace opentxs::api::network
