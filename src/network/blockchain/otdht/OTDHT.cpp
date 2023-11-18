// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/network/blockchain/OTDHT.hpp"  // IWYU pragma: associated

#include <string_view>

#include "internal/network/blockchain/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "network/blockchain/otdht/Actor.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::network::blockchain
{
using namespace std::literals;

auto print(DHTJob job) noexcept -> std::string_view
{
    try {
        using Job = DHTJob;
        static const auto map = Map<Job, std::string_view>{
            {Job::shutdown, "shutdown"sv},
            {Job::sync_request, "sync_request"sv},
            {Job::sync_ack, "sync_ack"sv},
            {Job::sync_reply, "sync_reply"sv},
            {Job::sync_push, "sync_push"sv},
            {Job::response, "response"sv},
            {Job::push_tx, "push_tx"sv},
            {Job::job_processed, "job_processed"sv},
            {Job::checksum_failure, "checksum_failure"sv},
            {Job::report, "report"sv},
            {Job::peer_list, "peer_list"sv},
            {Job::registration, "registration"sv},
            {Job::init, "init"sv},
            {Job::cfilter, "cfilter"sv},
            {Job::statemachine, "statemachine"sv},
        };

        return map.at(job);
    } catch (...) {
        LogAbort()(__FUNCTION__)("invalid DHTJob: ")(
            static_cast<OTZMQWorkType>(job))
            .Abort();
    }
}
}  // namespace opentxs::network::blockchain

namespace opentxs::network::blockchain
{
OTDHT::OTDHT(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<const opentxs::blockchain::node::Manager> node) noexcept
{
    assert_false(nullptr == api);
    assert_false(nullptr == node);

    const auto& zmq = api->Network().ZeroMQ().Internal();
    const auto batchID = zmq.PreallocateBatch();
    Actor::Factory(api, node, batchID);
}

auto OTDHT::Init() noexcept -> void
{
    // NOTE this function intentionally left blank
}

OTDHT::~OTDHT() = default;
}  // namespace opentxs::network::blockchain
