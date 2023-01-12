// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "internal/blockchain/node/wallet/FeeOracle.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <memory>
#include <numeric>  // IWYU pragma: keep
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/feeoracle/Actor.hpp"
#include "blockchain/node/wallet/feeoracle/Shared.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::wallet
{
auto print(FeeOracleJobs job) noexcept -> std::string_view
{
    try {
        using Job = FeeOracleJobs;
        static const auto map = Map<Job, CString>{
            {Job::shutdown, "shutdown"},
            {Job::update_estimate, "update_estimate"},
            {Job::init, "init"},
            {Job::statemachine, "statemachine"},
        };

        return map.at(job);
    } catch (...) {
        LogAbort()(__FUNCTION__)("invalid FeeOracleJobs: ")(
            static_cast<OTZMQWorkType>(job))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
FeeOracle::FeeOracle(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node) noexcept
    : shared_(boost::make_shared<Shared>())
{
    OT_ASSERT(api);
    OT_ASSERT(node);
    OT_ASSERT(shared_);

    const auto& asio = api->Network().ZeroMQ().Internal();
    const auto batchID = asio.PreallocateBatch();
    // TODO the version of libc++ present in android ndk 23.0.7599858
    // has a broken std::allocate_shared function so we're using
    // boost::shared_ptr instead of std::shared_ptr

    auto actor = boost::allocate_shared<Actor>(
        alloc::PMR<Actor>{asio.Alloc(batchID)},
        std::move(api),
        std::move(node),
        shared_,
        batchID);
    actor->Init(actor);
}

auto FeeOracle::EstimatedFee() const noexcept -> std::optional<Amount>
{
    return *(shared_->data_.lock_shared());
}

FeeOracle::~FeeOracle() = default;
}  // namespace opentxs::blockchain::node::wallet
