// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <string_view>
#include <utility>

#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Hash;
}  // namespace block

namespace node
{
struct Endpoints;
}  // namespace node
}  // namespace blockchain

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::blockoracle
{
class Update final : public opentxs::Allocated
{
public:
    auto FinishJob() noexcept -> void;
    auto FinishWork() noexcept -> void;
    auto get_allocator() const noexcept -> allocator_type final;
    auto Queue(const block::Hash& id, ReadView bytes, bool persistent) noexcept
        -> void;
    auto StartJob() noexcept -> void;

    Update(
        const api::Session& api,
        const Endpoints& endpoints,
        const Log& log,
        std::string_view name,
        allocator_type alloc) noexcept;
    Update() = delete;
    Update(const Update&) = delete;
    Update(Update&&) = delete;
    auto operator=(const Update&) -> Update& = delete;
    auto operator=(Update&&) -> Update& = delete;

    ~Update() final;

private:
    using Cache = Deque<std::pair<sTime, network::zeromq::Message>>;

    const Log& log_;
    const std::string_view name_;
    bool actor_is_working_;
    std::size_t job_count_;
    Cache pending_;
    network::zeromq::socket::Raw to_actor_;

    static auto is_full(const network::zeromq::Message& msg) noexcept -> bool;

    auto ready_to_send() const noexcept -> bool;

    auto construct() noexcept -> Cache::value_type&;
    auto next_message() noexcept -> Cache::value_type&;
    auto send() noexcept -> void;
};
}  // namespace opentxs::blockchain::node::blockoracle
