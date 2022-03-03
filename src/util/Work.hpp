// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs
{
constexpr auto OT_ZMQ_INTERNAL_SIGNAL = OTZMQWorkType{32768};
constexpr auto OT_ZMQ_HIGHEST_SIGNAL = OTZMQWorkType{65535};

// clang-format off
constexpr auto OT_ZMQ_STATE_MACHINE_SIGNAL =                  OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 0};
constexpr auto OT_ZMQ_SEND_SIGNAL =                           OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 1};
constexpr auto OT_ZMQ_RECEIVE_SIGNAL =                        OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 2};
constexpr auto OT_ZMQ_NEW_FILTER_SIGNAL =                     OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 3};
constexpr auto OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL =      OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 4};
constexpr auto OT_ZMQ_INIT_SIGNAL =                           OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 5};
constexpr auto OT_ZMQ_NEW_FULL_BLOCK_SIGNAL =                 OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 6};
constexpr auto OT_ZMQ_SYNC_DATA_SIGNAL =                      OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 7};
constexpr auto OT_ZMQ_HEARTBEAT_SIGNAL =                      OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 8};
constexpr auto OT_ZMQ_REGISTER_SIGNAL =                       OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 9};
constexpr auto OT_ZMQ_UNREGISTER_SIGNAL =                     OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 10};
constexpr auto OT_ZMQ_CONNECT_SIGNAL =                        OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 11};
constexpr auto OT_ZMQ_DISCONNECT_SIGNAL =                     OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 12};
constexpr auto OT_ZMQ_BIND_SIGNAL =                           OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 13};
constexpr auto OT_ZMQ_UNBIND_SIGNAL =                         OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 14};
constexpr auto OT_ZMQ_BLOCKCHAIN_NODE_READY =                 OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 15};
constexpr auto OT_ZMQ_SYNC_SERVER_BACKEND_READY =             OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 16};
constexpr auto OT_ZMQ_BLOCK_ORACLE_READY =                    OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 17};
constexpr auto OT_ZMQ_BLOCK_ORACLE_DOWNLOADER_READY =         OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 18};
constexpr auto OT_ZMQ_FILTER_ORACLE_READY =                   OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 19};
constexpr auto OT_ZMQ_FILTER_ORACLE_INDEXER_READY =           OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 20};
constexpr auto OT_ZMQ_FILTER_ORACLE_FILTER_DOWNLOADER_READY = OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 21};
constexpr auto OT_ZMQ_FILTER_ORACLE_HEADER_DOWNLOADER_READY = OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 22};
constexpr auto OT_ZMQ_PEER_MANAGER_READY =                    OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 23};
constexpr auto OT_ZMQ_BLOCKCHAIN_WALLET_READY =               OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 24};
constexpr auto OT_ZMQ_FEE_ORACLE_READY =                      OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 25};
// clang-format on

template <typename Enum>
auto MakeWork(const Enum type) noexcept -> network::zeromq::Message
{
    return network::zeromq::tagged_message<Enum>(type);
}
}  // namespace opentxs
