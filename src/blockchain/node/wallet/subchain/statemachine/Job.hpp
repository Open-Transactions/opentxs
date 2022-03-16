// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>

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
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
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
class Message;
}  // namespace zeromq
}  // namespace network

class Log;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet::statemachine
{
class Job : virtual public wallet::Job, public Actor<Job, SubchainJobs>
{
public:
    auto ChangeState(const State state) noexcept -> bool final;
    auto Init(boost::shared_ptr<Job> me) noexcept -> void
    {
        signal_startup(me);
    }
    auto Shutdown() noexcept -> void { signal_shutdown(); }

    ~Job() override;

protected:
    const SubchainStateData& parent_;

    Job(const Log& logger,
        const SubchainStateData& parent,
        const network::zeromq::BatchID batch,
        CString&& name,
        allocator_type alloc,
        const network::zeromq::EndpointArgs& subscribe = {},
        const network::zeromq::EndpointArgs& pull = {},
        const network::zeromq::EndpointArgs& dealer = {},
        const Vector<network::zeromq::SocketData>& extra = {},
        Set<Work>&& neverDrop = {}) noexcept;

private:
    friend Actor<Job, SubchainJobs>;

    const CString name_;
    std::atomic<State> state_;

    auto do_shutdown() noexcept -> void;
    virtual auto do_startup() noexcept -> void = 0;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_block(Message&& in) noexcept -> void;
    virtual auto process_block(block::pHash&& block) noexcept -> void;
    virtual auto process_key(Message&& in) noexcept -> void;
    auto process_filter(Message&& in) noexcept -> void;
    virtual auto process_filter(block::Position&& tip) noexcept -> void;
    virtual auto process_mempool(Message&& in) noexcept -> void;
    virtual auto process_startup(Message&& in) noexcept -> void;
    virtual auto process_update(Message&& msg) noexcept -> void;
    auto process_process(Message&& in) noexcept -> void;
    virtual auto process_process(block::Position&& position) noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_normal() noexcept -> void;
    auto transition_state_reorg() noexcept -> void;
    auto transition_state_shutdown() noexcept -> void;
    virtual auto work() noexcept -> bool = 0;
};
}  // namespace opentxs::blockchain::node::wallet::statemachine
