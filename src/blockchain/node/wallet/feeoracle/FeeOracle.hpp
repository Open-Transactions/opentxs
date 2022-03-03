// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <cs_deferred_guarded.h>
#include <future>
#include <optional>
#include <shared_mutex>
#include <tuple>
#include <utility>

#include "core/Worker.hpp"
#include "internal/blockchain/node/wallet/FeeOracle.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Allocated.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::blockchain::node::wallet::FeeOracle::Imp final
    : public opentxs::implementation::Allocated,
      public Worker<Imp, api::Session>
{
public:
    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        update_estimate = OT_ZMQ_INTERNAL_SIGNAL + 0,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    const blockchain::Type chain_;

    // Returns satoshis per 1000 bytes
    auto EstimatedFee() const noexcept -> std::optional<Amount>;
    auto Shutdown() noexcept -> void;

    Imp(const api::Session& api,
        const blockchain::Type chain,
        allocator_type&& alloc,
        CString endpoint) noexcept;

    ~Imp() final;

private:
    friend Worker<Imp, api::Session>;
    using Data = Vector<std::pair<Time, Amount>>;
    using Estimate =
        libguarded::deferred_guarded<std::optional<Amount>, std::shared_mutex>;

    Timer timer_;
    ForwardList<blockchain::node::wallet::FeeSource> sources_;
    Data data_;
    Estimate output_;

    auto pipeline(network::zeromq::Message&&) noexcept -> void;
    auto process_update(network::zeromq::Message&&) noexcept -> void;
    auto reset_timer() noexcept -> void;
    auto state_machine() noexcept -> bool;
    auto shutdown(std::promise<void>&) noexcept -> void;
};
