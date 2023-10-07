// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <chrono>
#include <memory>

#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/Timer.hpp"
#include "internal/util/storage/tree/Types.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
namespace imp
{
class Storage;
}  // namespace imp
}  // namespace session

class Session;
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::tree
{
class Actor final : public opentxs::Actor<tree::Actor, Job>
{
public:
    auto Init(std::shared_ptr<Actor> self) noexcept -> void
    {
        self_ = self;
        signal_startup(self);
    }

    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Actor(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<api::session::imp::Storage> parent,
        opentxs::network::zeromq::BatchID batchID,
        std::chrono::seconds interval,
        CString endpoint,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() final;

private:
    friend opentxs::Actor<tree::Actor, Job>;

    using GuardedSocket =
        libguarded::plain_guarded<network::zeromq::socket::Raw>;

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<api::session::imp::Storage> parent_p_;
    std::shared_ptr<Actor> self_;
    const api::Session& api_;
    api::session::imp::Storage& parent_;
    const std::chrono::seconds interval_;
    GuardedSocket push_;
    Timer timer_;

    static auto run_gc(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<api::session::imp::Storage> parent,
        std::shared_ptr<Actor> self,
        const GCParams& params) noexcept -> void;

    auto need_gc(std::chrono::microseconds elapsed) const noexcept -> bool;

    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto reset_gc_timer(std::chrono::microseconds wait) noexcept -> void;
    auto schedule_gc(std::chrono::microseconds elapsed) noexcept -> void;
    auto start_gc() noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::storage::tree
