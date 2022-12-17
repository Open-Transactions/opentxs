// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <exception>
#include <memory>

#include "internal/api/session/notary/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace notary
{
class Shared;
}  // namespace notary

class Notary;
}  // namespace session
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::session::notary
{
class Actor final : public opentxs::Actor<notary::Actor, Job>
{
public:
    auto Init(boost::shared_ptr<Actor> self) noexcept -> void
    {
        signal_startup(self);
    }

    Actor(
        std::shared_ptr<api::session::Notary> api,
        boost::shared_ptr<Shared> shared,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() final;

private:
    friend opentxs::Actor<notary::Actor, Job>;

    std::shared_ptr<api::session::Notary> api_p_;
    boost::shared_ptr<Shared> shared_p_;
    api::session::Notary& api_;
    Shared& shared_;
    Deque<identifier::UnitDefinition> queue_;

    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto process_queue_unitid(Message&& msg, allocator_type monotonic) noexcept
        -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::api::session::notary
