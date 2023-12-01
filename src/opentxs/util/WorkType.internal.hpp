// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/conversion.hpp>

#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Types.hpp"

namespace opentxs
{
constexpr auto OT_ZMQ_INTERNAL_SIGNAL = OTZMQWorkType{32768};
constexpr auto OT_ZMQ_HIGHEST_SIGNAL = OTZMQWorkType{65535};

// clang-format off
constexpr auto OT_ZMQ_STATE_MACHINE_SIGNAL =                    OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 0};
constexpr auto OT_ZMQ_PREPARE_SHUTDOWN =                        OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 1};
constexpr auto OT_ZMQ_ACKNOWLEDGE_SHUTDOWN =                    OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 2};
constexpr auto OT_ZMQ_SEND_SIGNAL =                             OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 3};
constexpr auto OT_ZMQ_RECEIVE_SIGNAL =                          OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 4};
constexpr auto OT_ZMQ_NEW_FILTER_SIGNAL =                       OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 5};
constexpr auto OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL =        OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 6};
constexpr auto OT_ZMQ_INIT_SIGNAL =                             OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 7};
constexpr auto OT_ZMQ_NEW_FULL_BLOCK_SIGNAL =                   OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 8};
constexpr auto OT_ZMQ_SYNC_DATA_SIGNAL =                        OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 9};
constexpr auto OT_ZMQ_HEARTBEAT_SIGNAL =                        OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 10};
constexpr auto OT_ZMQ_REGISTER_SIGNAL =                         OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 11};
constexpr auto OT_ZMQ_UNREGISTER_SIGNAL =                       OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 12};
constexpr auto OT_ZMQ_CONNECT_SIGNAL =                          OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 13};
constexpr auto OT_ZMQ_DISCONNECT_SIGNAL =                       OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 14};
constexpr auto OT_ZMQ_BIND_SIGNAL =                             OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 15};
constexpr auto OT_ZMQ_UNBIND_SIGNAL =                           OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 16};
constexpr auto OT_ZMQ_BLOCKCHAIN_NODE_READY =                   OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 17};
constexpr auto OT_ZMQ_SYNC_SERVER_BACKEND_READY =               OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 18};
constexpr auto OT_ZMQ_BLOCK_ORACLE_READY =                      OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 19};
constexpr auto OT_ZMQ_BLOCK_ORACLE_DOWNLOADER_READY =           OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 20};
constexpr auto OT_ZMQ_BLOCK_ORACLE_JOB_AVAILABLE =              OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 21};
constexpr auto OT_ZMQ_BLOCK_ORACLE_BLOCK_READY =                OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 22};
constexpr auto OT_ZMQ_FILTER_ORACLE_READY =                     OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 23};
constexpr auto OT_ZMQ_FILTER_ORACLE_INDEXER_READY =             OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 24};
constexpr auto OT_ZMQ_FILTER_ORACLE_FILTER_DOWNLOADER_READY =   OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 25};
constexpr auto OT_ZMQ_FILTER_ORACLE_HEADER_DOWNLOADER_READY =   OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 26};
constexpr auto OT_ZMQ_PEER_MANAGER_READY =                      OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 27};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_READY =                 OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 28};
constexpr auto OT_ZMQ_FEE_ORACLE_READY =                        OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 29};
constexpr auto OT_ZMQ_BALANCE_ORACLE_SUBMIT =                   OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 30};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_SHUTDOWN_READY =        OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 31};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_REORG_READY =           OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 32};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_FINISH_REORG =          OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 33};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_PERFORM_REORG =         OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 34};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_DO_RESCAN =             OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 35};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_RESCAN =                OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 36};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_REPROCESS =             OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 37};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_WATCHDOG_ACK =          OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 38};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_WATCHDOG =              OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 39};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_PROCESS =               OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 40};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_UPDATE =                OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 41};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_PREPARE_REORG =         OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 42};
constexpr auto OT_ZMQ_NEW_BLOCK_HEADER_SIGNAL =                 OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 43};
constexpr auto OT_ZMQ_REORG_SIGNAL =                            OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 44};
constexpr auto OT_ZMQ_HEADER_ORACLE_JOB_READY =                 OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 45};
constexpr auto OT_ZMQ_OTDHT_PEER_LIST =                         OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 46};
constexpr auto OT_ZMQ_BLOCKCHAIN_REPORT_STATUS =                OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 47};
constexpr auto OT_ZMQ_BLOCKCHAIN_SYNC_CHECKSUM_FAILURE =        OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 48};
constexpr auto OT_ZMQ_BLOCKCHAIN_BROADCAST_TX =                 OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 49};
// clang-format on
// NOTE update print function if new values are defined

template <typename Enum>
auto MakeWork(const Enum type) noexcept -> network::zeromq::Message
{
    static_assert(sizeof(Enum) == sizeof(OTZMQWorkType));
    auto value = static_cast<OTZMQWorkType>(type);
    boost::endian::native_to_little_inplace(value);

    return network::zeromq::tagged_message<OTZMQWorkType>(value, true);
}
}  // namespace opentxs
