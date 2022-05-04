// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <exception>
#include <optional>

#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
class Hash;
}  // namespace block

namespace node
{
namespace wallet
{
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

class Log;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet::statemachine
{
class Job : virtual public wallet::Job, public Actor<SubchainJobs>
{
    boost::shared_ptr<const SubchainStateData> parent_p_;

public:
    auto ChangeState(const State state, StateSequence reorg) noexcept
        -> bool final;
    auto Init(boost::shared_ptr<Job> me) noexcept -> void
    {
        signal_startup(me);
    }
    auto Shutdown() noexcept -> void { signal_shutdown(); }

    ~Job() override;

protected:
    auto do_startup() noexcept -> void override;
    auto do_shutdown() noexcept -> void override;
    auto pipeline(const Work work, Message&& msg) noexcept -> void override;
    auto work() noexcept -> bool override;

    const SubchainStateData& parent_;

    auto last_reorg() const noexcept -> std::optional<StateSequence>;

    Job(const Log& logger,
        const boost::shared_ptr<const SubchainStateData>& parent,
        const network::zeromq::BatchID batch,
        const JobType type,
        allocator_type alloc,
        const network::zeromq::EndpointArgs& subscribe = {},
        const network::zeromq::EndpointArgs& pull = {},
        const network::zeromq::EndpointArgs& dealer = {},
        const Vector<network::zeromq::SocketData>& extra = {},
        Set<Work>&& neverDrop = {}) noexcept;

private:
    using HandledReorgs = Set<StateSequence>;

    const JobType job_type_;
    network::zeromq::socket::Raw& to_parent_;
    std::atomic<State> pending_state_;
    std::atomic<State> state_;
    HandledReorgs reorgs_;
    Timer watchdog_;

    auto process_block(Message&& in) noexcept -> void;
    auto process_filter(Message&& in) noexcept -> void;
    auto process_prepare_reorg(Message&& in) noexcept -> void;
    auto process_process(Message&& in) noexcept -> void;
    auto process_watchdog() noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_normal() noexcept -> bool;
    auto transition_state_reorg(StateSequence id) noexcept -> bool;
    auto transition_state_shutdown() noexcept -> bool;

    virtual auto process_block(block::Hash&& block) noexcept -> void;
    virtual auto process_key(Message&& in) noexcept -> void;
    virtual auto process_filter(Message&& in, block::Position&& tip) noexcept
        -> void;
    virtual auto process_mempool(Message&& in) noexcept -> void;
    virtual auto process_process(block::Position&& position) noexcept -> void;
    virtual auto process_reprocess(Message&& msg) noexcept -> void;
    virtual auto process_startup(Message&& in) noexcept -> void;
    virtual auto process_update(Message&& msg) noexcept -> void;
};
}  // namespace opentxs::blockchain::node::wallet::statemachine
