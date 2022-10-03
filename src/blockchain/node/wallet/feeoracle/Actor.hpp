// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_deferred_guarded.h>
#include <exception>
#include <future>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <tuple>
#include <utility>

#include "blockchain/node/wallet/feeoracle/Shared.hpp"
#include "internal/blockchain/node/wallet/FeeOracle.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Actor.hpp"
#include "util/Allocated.hpp"
#include "util/Work.hpp"

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
namespace wallet
{
class FeeSource;
}  // namespace wallet

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

class Amount;
class Timer;
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
    auto do_startup() noexcept -> bool;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_update(network::zeromq::Message&&) noexcept -> void;
    auto work() noexcept -> bool;
};
