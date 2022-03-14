// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/node/wallet/subchain/statemachine/Progress.hpp"

#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <optional>

#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
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
namespace socket
{
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Progress::Imp final : public Actor<Imp, SubchainJobs>
{
public:
    auto VerifyState(const State state) const noexcept -> void;

    auto Init(boost::shared_ptr<Imp> me) noexcept -> void
    {
        signal_startup(me);
    }
    auto ProcessReorg(const block::Position& parent) noexcept -> void;
    auto Shutdown() noexcept -> void { signal_shutdown(); }

    Imp(const boost::shared_ptr<const SubchainStateData>& parent,
        const network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    Imp& operator=(const Imp&) = delete;
    Imp& operator=(Imp&&) = delete;

    ~Imp() final = default;

private:
    friend Actor<Imp, SubchainJobs>;

    network::zeromq::socket::Raw& to_parent_;
    network::zeromq::socket::Raw& to_rescan_;
    boost::shared_ptr<const SubchainStateData> parent_p_;
    const SubchainStateData& parent_;
    std::atomic<State> state_;
    std::optional<block::Position> last_reported_;

    auto notify(const block::Position& pos) const noexcept -> void;

    auto do_shutdown() noexcept -> void;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_update(Message&& msg) noexcept -> void;
    auto startup() noexcept -> void {}
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_normal(Message&& msg) noexcept -> void;
    auto transition_state_reorg(Message&& msg) noexcept -> void;
    [[noreturn]] auto work() noexcept -> bool;
};
}  // namespace opentxs::blockchain::node::wallet
