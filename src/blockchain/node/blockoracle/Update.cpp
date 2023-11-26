// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/blockoracle/Update.hpp"  // IWYU pragma: associated

#include <chrono>
#include <compare>
#include <limits>
#include <span>
#include <utility>

#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::blockoracle
{
Update::Update(
    const api::Session& api,
    const Endpoints& endpoints,
    const Log& log,
    std::string_view name,
    allocator_type alloc) noexcept
    : log_(log)
    , name_(name)
    , actor_is_working_()
    , job_count_(0)
    , pending_(alloc)
    , to_actor_([&] {
        using enum network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Context().Internal().RawSocket(Push);
        const auto rc = out.Connect(endpoints.block_oracle_pull_.c_str());

        assert_true(rc);

        return out;
    }())
{
}

auto Update::construct() noexcept -> Cache::value_type&
{
    using enum blockoracle::Job;

    return pending_.emplace_back(sClock::now(), MakeWork(block_ready));
}

auto Update::FinishJob() noexcept -> void
{
    assert_true(0_uz < job_count_);

    --job_count_;
    send();
}

auto Update::FinishWork() noexcept -> void
{
    actor_is_working_ = false;
    send();
}

auto Update::get_allocator() const noexcept -> allocator_type
{
    return pending_.get_allocator();
}

auto Update::is_full(const network::zeromq::Message& msg) noexcept -> bool
{
    constexpr auto limit = 1000_uz;

    return msg.Payload().size() > limit;
}

auto Update::next_message() noexcept -> Cache::value_type&
{
    if (pending_.empty()) {

        return construct();
    } else {

        return pending_.back();
    }
}

auto Update::Queue(const block::Hash& id, const BlockLocation& block) noexcept
    -> void
{
    auto& message = [&]() -> auto& {
        auto& [time, existing] = next_message();

        if (is_full(existing)) {

            return construct().second;
        } else {

            return existing;
        }
    }();
    message.AddFrame(id);

    if (false == serialize(block, message.AppendBytes())) {
        LogAbort()().Abort();
    }

    send();
}

auto Update::ready_to_send() const noexcept -> bool
{
    const auto& log = log_;

    if (pending_.empty()) {
        log()(name_)(": no pending message to send").Flush();

        return false;
    }

    if (actor_is_working_) {
        log()(name_)(
            ": waiting to send until actor is finished with previous message")
            .Flush();

        return false;
    }

    if (0_uz == job_count_) {
        log()(name_)(": all jobs are complete").Flush();

        return true;
    }

    constexpr auto limit = 5s;
    const auto& [time, _] = pending_.front();
    const auto elapsed = sClock::now() - time;

    return elapsed >= limit;
}

auto Update::send() noexcept -> void
{
    const auto& log = log_;

    if (ready_to_send()) {
        auto& [_, message] = pending_.front();
        log()(name_)(": notifying actor of ")(
            (message.Payload().size() - 1_uz) / 2_uz)(" downloaded blocks")
            .Flush();
        to_actor_.SendDeferred(std::move(message));
        actor_is_working_ = true;
        pending_.pop_front();
    }
}

auto Update::StartJob() noexcept -> void
{
    assert_true(std::numeric_limits<decltype(job_count_)>::max() > job_count_);

    ++job_count_;
}

Update::~Update() = default;
}  // namespace opentxs::blockchain::node::blockoracle
