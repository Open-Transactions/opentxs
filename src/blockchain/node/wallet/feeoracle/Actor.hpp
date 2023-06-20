// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <memory>
#include <utility>

#include "blockchain/node/wallet/feeoracle/Shared.hpp"
#include "internal/blockchain/node/wallet/FeeOracle.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Actor.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::blockchain::node::wallet::FeeOracle::Actor final
    : public opentxs::Actor<FeeOracle::Actor, FeeOracleJobs>
{
public:
    auto Init(boost::shared_ptr<Actor> me) noexcept -> void
    {
        signal_startup(me);
    }

    Actor(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        boost::shared_ptr<Shared> shared,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;

    ~Actor() final;

private:
    friend opentxs::Actor<FeeOracle::Actor, FeeOracleJobs>;

    using Data = Vector<std::pair<Time, Amount>>;

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const node::Manager> node_p_;
    boost::shared_ptr<Shared> shared_p_;
    const api::Session& api_;
    const node::Manager& node_;
    const blockchain::Type chain_;
    Timer timer_;
    Data data_;
    Shared::Estimate& output_;

    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto pipeline(
        const Work work,
        Message&& msg,
        allocator_type monotonic) noexcept -> void;
    auto process_update(
        network::zeromq::Message&&,
        allocator_type monotonic) noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
