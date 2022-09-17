// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_shared_guarded.h>
#include <atomic>
#include <memory>
#include <shared_mutex>

#include "internal/network/blockchain/OTDHT.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/P0330.hpp"
#include "network/blockchain/otdht/Actor.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"
#include "util/JobCounter.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace database
{
class Sync;
}  // namespace database

namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::otdht
{
class Server final : public OTDHT::Actor
{
public:
    Server(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const opentxs::blockchain::node::Manager> node,
        network::zeromq::BatchID batchID,
        allocator_type alloc) noexcept;
    Server() = delete;
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;

    ~Server() final;

private:
    static constexpr auto queue_limit_ = 1000_uz;

    struct Shared {
        opentxs::blockchain::database::Sync& db_;
        opentxs::blockchain::block::Position sync_tip_;
        Deque<opentxs::blockchain::block::Position> queue_;
        bool caught_up_;

        auto Best() const noexcept
            -> const opentxs::blockchain::block::Position&;

        Shared(
            const opentxs::blockchain::node::Manager& node,
            allocator_type alloc) noexcept;
        Shared() = delete;
        Shared(const Shared&) = delete;
        Shared(Shared&&) = delete;
        auto operator=(const Shared&) -> Shared& = delete;
        auto operator=(Shared&&) -> Shared& = delete;
    };

    using Guarded = libguarded::shared_guarded<Shared, std::shared_mutex>;

    const opentxs::blockchain::block::Position checkpoint_;
    Guarded shared_;
    JobCounter counter_;
    Outstanding running_;

    auto local_position() const noexcept
        -> opentxs::blockchain::block::Position final;

    auto background() noexcept -> void;
    auto check_caught_up(Shared& shared) noexcept -> void;
    auto do_work() noexcept -> bool final;
    auto drain_queue(Shared& shared) noexcept -> void;
    auto fill_queue(Shared& shared) noexcept -> void;
    auto process_report(Message&& msg) noexcept -> void final;
    auto process_sync_request(Message&& msg) noexcept -> void final;
    auto report() noexcept -> void;
    auto report(const opentxs::blockchain::block::Position& tip) noexcept
        -> void;
    auto update_tip(
        Shared& shared,
        bool db,
        opentxs::blockchain::block::Position tip) noexcept -> void;
};
}  // namespace opentxs::network::blockchain::otdht