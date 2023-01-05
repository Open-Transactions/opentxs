// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "internal/api/network/ZAP.hpp"
#include "internal/network/zeromq/Handle.hpp"
#include "opentxs/network/zeromq/zap/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace internal
{
class Batch;
class Thread;
}  // namespace internal

namespace socket
{
class Raw;
}  // namespace socket

class Context;
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network

class Factory;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::imp
{
class ZAP final : public internal::ZAP
{
public:
    auto RegisterDomain(
        const std::string_view domain,
        const opentxs::network::zeromq::zap::ReceiveCallback& callback) const
        -> bool final;
    auto SetDefaultPolicy(
        const opentxs::network::zeromq::zap::Policy policy) const -> bool final;

    ZAP() = delete;
    ZAP(const ZAP&) = delete;
    ZAP(ZAP&&) = delete;
    auto operator=(const ZAP&) -> ZAP& = delete;
    auto operator=(ZAP&&) -> ZAP& = delete;

    ~ZAP() final;

private:
    friend opentxs::Factory;

    const opentxs::network::zeromq::Context& context_;
    opentxs::network::zeromq::internal::Handle handle_;
    opentxs::network::zeromq::internal::Batch& batch_;
    opentxs::network::zeromq::ListenCallback& callback_;
    opentxs::network::zeromq::socket::Raw& socket_;
    opentxs::network::zeromq::internal::Thread* thread_;

    auto process(opentxs::network::zeromq::Message&& msg) const noexcept
        -> void;

    ZAP(const opentxs::network::zeromq::Context& context);
};
}  // namespace opentxs::api::network::imp
