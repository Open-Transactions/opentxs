// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace network
{
class Asio;
class Blockchain;
class Dht;
}  // namespace network
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

namespace opentxs::api::network
{
class OPENTXS_EXPORT Network
{
public:
    struct Imp;

    auto Asio() const noexcept -> const network::Asio&;
    auto Blockchain() const noexcept -> const network::Blockchain&;
    auto DHT() const noexcept -> const network::Dht&;
    auto ZeroMQ() const noexcept -> const opentxs::network::zeromq::Context&;

    OPENTXS_NO_EXPORT auto Shutdown() noexcept -> void;

    OPENTXS_NO_EXPORT Network(Imp*) noexcept;

    OPENTXS_NO_EXPORT ~Network();

private:
    Imp* imp_;

    Network() = delete;
    Network(const Network&) = delete;
    Network(Network&&) = delete;
    auto operator=(const Network&) -> Network& = delete;
    auto operator=(Network&&) -> Network& = delete;
};
}  // namespace opentxs::api::network
