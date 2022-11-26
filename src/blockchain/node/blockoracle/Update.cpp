// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "blockchain/node/blockoracle/Update.hpp"  // IWYU pragma: associated

#include <chrono>
#include <compare>
#include <limits>
#include <utility>

#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
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
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Push);
        const auto rc = out.Connect(endpoints.block_oracle_pull_.c_str());

        OT_ASSERT(rc);

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
    OT_ASSERT(0_uz < job_count_);

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
    constexpr auto limit = 100_uz;

    return msg.Body().size() > limit;
}

auto Update::next_message() noexcept -> Cache::value_type&
{
    if (pending_.empty()) {

        return construct();
    } else {

        return pending_.back();
    }
}

auto Update::Queue(
    const block::Hash& id,
    ReadView bytes,
    bool persistent) noexcept -> void
{
    auto& message = [&]() -> auto&
    {
        auto& [time, existing] = next_message();

        if (is_full(existing)) {

            return construct().second;
        } else {

            return existing;
        }
    }
    ();

    message.AddFrame(id);

    if (persistent) {
        const auto data = SerializedReadView{bytes};
        message.AddFrame(std::addressof(data), sizeof(data));
    } else {
        message.AddFrame(bytes.data(), bytes.size());
    }

    send();
}

auto Update::ready_to_send() const noexcept -> bool
{
    const auto& log = log_;

    if (pending_.empty()) {
        log(OT_PRETTY_CLASS())(name_)(": no pending message to send").Flush();

        return false;
    }

    if (actor_is_working_) {
        log(OT_PRETTY_CLASS())(name_)(
            ": waiting to send until actor is finished with previous message")
            .Flush();

        return false;
    }

    if (0_uz == job_count_) {
        log(OT_PRETTY_CLASS())(name_)(": all jobs are complete").Flush();

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
        log(OT_PRETTY_CLASS())(name_)(": notifying actor of ")(
            (message.Body().size() - 1_uz) / 2_uz)(" downloaded blocks")
            .Flush();
        to_actor_.SendDeferred(std::move(message), __FILE__, __LINE__);
        actor_is_working_ = true;
        pending_.pop_front();
    }
}

auto Update::StartJob() noexcept -> void
{
    OT_ASSERT(std::numeric_limits<decltype(job_count_)>::max() > job_count_);

    ++job_count_;
}

Update::~Update() = default;
}  // namespace opentxs::blockchain::node::blockoracle
