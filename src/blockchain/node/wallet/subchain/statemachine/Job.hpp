// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <exception>
#include <optional>

#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/ReorgSlave.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Hash;
class Position;
}  // namespace block

namespace node
{
namespace internal
{
struct HeaderOraclePrivate;
}  // namespace internal

namespace wallet
{
class SubchainStateData;
}  // namespace wallet

class HeaderOracle;
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet::statemachine
{
class Job : virtual public wallet::Job, public opentxs::Actor<Job, SubchainJobs>
{
    boost::shared_ptr<const SubchainStateData> parent_p_;

public:
    using wallet::Job::Init;

    auto Init() noexcept -> void final {}
    auto Init(boost::shared_ptr<Job> me) noexcept -> void
    {
        signal_startup(me);
    }

    ~Job() override;

protected:
    using State = JobState;

    const SubchainStateData& parent_;
    mutable ReorgSlave reorg_;

    auto add_last_reorg(Message& out) const noexcept -> void;
    auto last_reorg() const noexcept -> std::optional<StateSequence>;
    auto state() const noexcept -> State { return state_.load(); }

    virtual auto do_reorg(
        const node::HeaderOracle& oracle,
        const node::internal::HeaderOraclePrivate& data,
        Reorg::Params& params) noexcept -> bool;
    virtual auto work() noexcept -> bool;

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
    friend opentxs::Actor<Job, SubchainJobs>;

    using HandledReorgs = Set<StateSequence>;

    const JobType job_type_;
    network::zeromq::socket::Raw& to_parent_;
    std::atomic<State> pending_state_;
    std::atomic<State> state_;
    HandledReorgs reorgs_;
    Timer watchdog_;

    auto do_shutdown() noexcept -> void;
    auto do_startup() noexcept -> bool;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_block(Message&& in) noexcept -> void;
    auto process_filter(Message&& in) noexcept -> void;
    auto process_prepare_reorg(Message&& in) noexcept -> void;
    auto process_process(Message&& in) noexcept -> void;
    auto process_update(Message&& msg) noexcept -> void;
    auto process_watchdog() noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_pre_shutdown(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_normal() noexcept -> void;
    auto transition_state_pre_shutdown() noexcept -> void;
    auto transition_state_reorg(StateSequence id) noexcept -> void;

    virtual auto do_process_update(Message&& msg) noexcept -> void;
    virtual auto do_startup_internal() noexcept -> void = 0;
    virtual auto forward_to_next(Message&& msg) noexcept -> void = 0;
    virtual auto process_block(block::Hash&& block) noexcept -> void;
    virtual auto process_do_rescan(Message&& in) noexcept -> void = 0;
    virtual auto process_filter(Message&& in, block::Position&& tip) noexcept
        -> void;
    virtual auto process_key(Message&& in) noexcept -> void;
    virtual auto process_mempool(Message&& in) noexcept -> void;
    virtual auto process_process(block::Position&& position) noexcept -> void;
    virtual auto process_reprocess(Message&& msg) noexcept -> void;
    virtual auto process_start_scan(Message&& in) noexcept -> void;
};
}  // namespace opentxs::blockchain::node::wallet::statemachine
