// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/unordered/unordered_node_map.hpp>
#include <cs_plain_guarded.h>
#include <atomic>
#include <functional>
#include <span>
#include <string_view>
#include <utility>

#include "internal/network/zeromq/Handle.hpp"
#include "opentxs/api/network/Types.hpp"
#include "opentxs/api/network/ZAP.internal.hpp"
#include "opentxs/api/network/zap/Types.internal.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
class ZAPPrivate;  // IWYU pragma: keep
}  // namespace network
}  // namespace api

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
class Envelope;
class Frame;
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::api::network::ZAPPrivate final : public internal::ZAP
{
public:
    auto RegisterDomain(std::string_view domain, std::string_view handler)
        const noexcept -> bool final;
    auto SetDefaultPolicy(ZAPPolicy policy) const noexcept -> bool final;

    ZAPPrivate(const opentxs::network::zeromq::Context& context);
    ZAPPrivate() = delete;
    ZAPPrivate(const ZAPPrivate&) = delete;
    ZAPPrivate(ZAPPrivate&&) = delete;
    auto operator=(const ZAPPrivate&) -> ZAPPrivate& = delete;
    auto operator=(ZAPPrivate&&) -> ZAPPrivate& = delete;

    ~ZAPPrivate() final;

private:
    using Data = std::pair<CString, bool>;
    using DomainHandlers = boost::unordered_node_map<
        std::string_view,
        Data,
        std::hash<std::string_view>,
        std::equal_to<std::string_view>,
        alloc::PMR<std::pair<const std::string_view, Data>>>;
    using Guarded = libguarded::plain_guarded<DomainHandlers>;

    const opentxs::network::zeromq::Context& context_;
    mutable std::atomic<ZAPPolicy> policy_;
    mutable Guarded domains_;
    opentxs::network::zeromq::internal::Handle handle_;
    opentxs::network::zeromq::internal::Batch& batch_;
    opentxs::network::zeromq::ListenCallback& upstream_callback_;
    opentxs::network::zeromq::ListenCallback& downstream_callback_;
    opentxs::network::zeromq::socket::Raw& upstream_;
    opentxs::network::zeromq::socket::Raw& downstream_;
    opentxs::network::zeromq::internal::Thread* thread_;

    auto process_downstream(opentxs::network::zeromq::Message&& msg) noexcept
        -> void;
    auto process_upstream(opentxs::network::zeromq::Message&& msg) noexcept
        -> void;
    auto send_reply(
        opentxs::network::zeromq::Envelope&& envelope,
        std::span<opentxs::network::zeromq::Frame> payload,
        zap::Status answer) noexcept -> void;

    ZAPPrivate(
        const opentxs::network::zeromq::Context& context,
        opentxs::network::zeromq::BatchID batch);
};
