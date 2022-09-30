// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <exception>
#include <memory>
#include <string_view>

#include "internal/util/Timer.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
namespace asio
{
class Shared;
}  // namespace asio
}  // namespace network

class Context;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::asio
{
class Actor final : public opentxs::Actor<Actor, OTZMQWorkType>
{
public:
    auto Init(boost::shared_ptr<Actor> self) noexcept -> void
    {
        signal_startup(self);
    }

    Actor(
        std::shared_ptr<const api::Context> context,
        boost::shared_ptr<Shared> shared,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() final;

private:
    friend opentxs::Actor<Actor, OTZMQWorkType>;

    std::shared_ptr<const api::Context> context_p_;
    boost::shared_ptr<Shared> shared_p_;
    const api::Context& context_;
    Shared& shared_;
    opentxs::network::zeromq::socket::Raw& router_;

    auto do_shutdown() noexcept -> void;
    auto do_startup() noexcept -> bool;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto pipeline_internal(const Work work, Message&& msg) noexcept -> void;
    auto pipeline_external(const Work work, Message&& msg) noexcept -> void;
    auto process_registration(Message&& msg) noexcept -> void;
    auto process_resolve(Message&& msg) noexcept -> void;
    auto process_sent(Message&& msg) noexcept -> void;
    auto work() noexcept -> bool;
};
}  // namespace opentxs::api::network::asio
