// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/flat_map_types.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/table.hpp>

#include "opentxs/network/blockchain/Types.internal.hpp"  // IWYU pragma: associated

#include <boost/endian/conversion.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <span>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/params/ChainData.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/Type.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/identifier/Account.hpp"            // IWYU pragma: keep
#include "opentxs/network/blockchain/Protocol.hpp"   // IWYU pragma: keep
#include "opentxs/network/blockchain/Subchain.hpp"   // IWYU pragma: keep
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::network::blockchain
{
using namespace std::literals;

auto decode(const zeromq::Message& in) noexcept -> opentxs::blockchain::Type
{
    using enum opentxs::blockchain::Type;
    using namespace opentxs::blockchain::params;
    static const auto map = [] {
        using Key = opentxs::blockchain::params::ChainData::ZMQParams;
        using Value = opentxs::blockchain::Type;
        auto out = boost::unordered_flat_map<Key, Value>{};

        for (const auto chain : chains()) {
            const auto& data = get(chain);
            out.try_emplace(data.ZMQ(), chain);
        }

        out.reserve(out.size());

        return out;
    }();

    try {
        const auto payload = in.Payload();

        if (auto count = payload.size(); 5_uz > count) {
            const auto error =
                UnallocatedCString{
                    "expected at least 5 frames in payload but only have "}
                    .append(std::to_string(count));

            throw std::runtime_error{error};
        }

        const auto key = std::make_pair(
            payload[1].as<opentxs::blockchain::crypto::Bip44Type>(),
            payload[2].as<Subchain>());

        if (const auto i = map.find(key); map.end() != i) {

            return i->second;
        } else {
            const auto error =
                UnallocatedCString{
                    "unable to decode combination of bip44 type "}
                    .append(
                        std::to_string(static_cast<std::uint32_t>(key.first)))
                    .append(" and subchain ")
                    .append(
                        std::to_string(static_cast<std::uint8_t>(key.second)));

            throw std::runtime_error{error};
        }
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return UnknownBlockchain;
    }
}

auto encode(opentxs::blockchain::Type chain, zeromq::Message& out) noexcept
    -> void
{
    auto [bip44, subchain] = opentxs::blockchain::params::get(chain).ZMQ();
    static_assert(sizeof(subchain) == sizeof(std::uint8_t));
    boost::endian::native_to_little_inplace(bip44);
    out.AddFrame(bip44);
    out.AddFrame(subchain);
}

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

auto print(PeerJob in) noexcept -> std::string_view
{
    using enum PeerJob;
    static constexpr auto map =
        frozen::make_unordered_map<PeerJob, std::string_view>({
            {shutdown, "shutdown"sv},
            {blockheader, "blockheader"sv},
            {reorg, "reorg"sv},
            {mempool, "mempool"sv},
            {registration, "registration"sv},
            {connect, "connect"sv},
            {disconnect, "disconnect"sv},
            {sendresult, "sendresult"sv},
            {p2p, "p2p"sv},
            {gossip_address, "gossip_address"sv},
            {jobtimeout, "jobtimeout"sv},
            {needpeers, "needpeers"sv},
            {statetimeout, "statetimeout"sv},
            {activitytimeout, "activitytimeout"sv},
            {needping, "needping"sv},
            {body, "body"sv},
            {header, "header"sv},
            {broadcasttx, "broadcasttx"sv},
            {jobavailablegetheaders, "jobavailablegetheaders"sv},
            {jobavailableblock, "jobavailableblock"sv},
            {block, "block"sv},
            {init, "init"sv},
            {statemachine, "statemachine"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid PeerJob: ")(
            static_cast<OTZMQWorkType>(in))
            .Abort();
    }
}
}  // namespace opentxs::network::blockchain
