// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/block/Hash.hpp"

#pragma once

#include <string_view>

#include "blockchain/DownloadTask.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/util/WorkType.hpp"
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
namespace bitcoin
{
namespace block
{
class Block;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Hash;
}  // namespace block

namespace cfilter
{
class Hash;
class Header;
}  // namespace cfilter

namespace node
{
class HeaderOracle;
}  // namespace node

class GCS;
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
enum class ManagerJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    sync_reply = value(WorkType::P2PBlockchainSyncReply),
    sync_new_block = value(WorkType::P2PBlockchainNewBlock),
    submit_block = OT_ZMQ_INTERNAL_SIGNAL + 2,
    heartbeat = OT_ZMQ_INTERNAL_SIGNAL + 3,
    send_to_address = OT_ZMQ_INTERNAL_SIGNAL + 4,
    send_to_paymentcode = OT_ZMQ_INTERNAL_SIGNAL + 5,
    start_wallet = OT_ZMQ_INTERNAL_SIGNAL + 6,
    filter_update = OT_ZMQ_NEW_FILTER_SIGNAL,
    state_machine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

enum class PeerManagerJobs : OTZMQWorkType {
    BroadcastTransaction = OT_ZMQ_INTERNAL_SIGNAL + 2,
    JobAvailableCfheaders = OT_ZMQ_INTERNAL_SIGNAL + 4,
    JobAvailableCfilters = OT_ZMQ_INTERNAL_SIGNAL + 5,
    Heartbeat = OT_ZMQ_HEARTBEAT_SIGNAL,
};

enum class SyncServerJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    heartbeat = OT_ZMQ_HEARTBEAT_SIGNAL,
    filter = OT_ZMQ_NEW_FILTER_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

enum class StatsJobs : OTZMQWorkType {
    shutdown = value(WorkType::Shutdown),
    block_header = value(WorkType::BlockchainNewHeader),
    reorg = value(WorkType::BlockchainReorg),
    cfilter = value(WorkType::BlockchainNewFilter),
    peer = value(WorkType::BlockchainPeerAdded),
    sync_server = value(WorkType::BlockchainSyncServerProgress),
    block = value(WorkType::BlockchainBlockOracleProgress),
    init = OT_ZMQ_INIT_SIGNAL,
    statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
};

using CfheaderJob =
    download::Batch<cfilter::Hash, cfilter::Header, cfilter::Type>;
using CfilterJob = download::Batch<GCS, cfilter::Header, cfilter::Type>;

auto print(StatsJobs) noexcept -> std::string_view;
constexpr auto value(PeerManagerJobs job) noexcept
{
    return static_cast<OTZMQWorkType>(job);
}
}  // namespace opentxs::blockchain::node
