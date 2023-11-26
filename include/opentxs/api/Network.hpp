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
namespace internal
{
class Network;
}  // namespace internal

namespace network
{
class Asio;
class Blockchain;
class OTDHT;
class ZeroMQ;
}  // namespace network

class Network;  // IWYU pragma: keep
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

/**
 The top-level network API. Used for accessing the Asio API, the Blockchain
 network API, the DHT network API, and the ZeroMQ network API.
 */
class OPENTXS_EXPORT opentxs::api::Network
{
public:
    auto Asio() const noexcept -> const network::Asio&;
    auto Blockchain() const noexcept -> const network::Blockchain&;
    OPENTXS_NO_EXPORT auto Internal() const noexcept
        -> const internal::Network&;
    auto OTDHT() const noexcept -> const network::OTDHT&;
    auto ZeroMQ() const noexcept -> const network::ZeroMQ&;

    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Network&;

    OPENTXS_NO_EXPORT Network(internal::Network* imp) noexcept;
    Network() = delete;
    Network(const Network&) = delete;
    Network(Network&&) = delete;
    auto operator=(const Network&) -> Network& = delete;
    auto operator=(Network&&) -> Network& = delete;

    OPENTXS_NO_EXPORT virtual ~Network();

protected:
    friend internal::Network;

    internal::Network* imp_;
};
