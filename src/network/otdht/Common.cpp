// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/otdht/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <string_view>

#include "internal/network/otdht/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/network/otdht/MessageType.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::network::otdht
{
using namespace std::literals;

auto print(Job job) noexcept -> std::string_view
{
    try {
        using namespace std::literals;
        using enum Job;
        static constexpr auto map =
            frozen::make_unordered_map<Job, std::string_view>({
                {Shutdown, "Shutdown"sv},
                {BlockHeader, "BlockHeader"sv},
                {Reorg, "Reorg"sv},
                {SyncServerUpdated, "SyncServerUpdated"sv},
                {SyncAck, "SyncAck"sv},
                {SyncReply, "SyncReply"sv},
                {SyncPush, "SyncPush"sv},
                {Response, "Response"sv},
                {PublishContract, "PublishContract"sv},
                {QueryContract, "QueryContract"sv},
                {PushTransaction, "PushTransaction"sv},
                {Register, "Register"sv},
                {Request, "Request"sv},
                {Processed, "Processed"sv},
                {ReorgInternal, "ReorgInternal"sv},
                {NewHeaderTip, "NewHeaderTip"sv},
                {Init, "Init"sv},
                {NewCFilterTip, "NewCFilterTip"sv},
                {StateMachine, "StateMachine"sv},
            });

        return map.at(job);
    } catch (...) {
        LogError()(__FUNCTION__)("invalid Job: ")(
            static_cast<OTZMQWorkType>(job))
            .Flush();

        OT_FAIL;
    }
}

auto print(MessageType value) noexcept -> std::string_view
{
    using enum MessageType;
    static constexpr auto map =
        frozen::make_unordered_map<MessageType, std::string_view>({
            {sync_request, "sync request"sv},
            {sync_ack, "sync acknowledgment"sv},
            {sync_reply, "sync reply"sv},
            {new_block_header, "sync push"sv},
            {query, "sync query"sv},
            {publish_contract, "publish contract"sv},
            {publish_ack, "publish acknowledgment"sv},
            {contract_query, "contract query"sv},
            {contract, "contract reply"sv},
        });

    try {

        return map.at(value);
    } catch (...) {

        return "error"sv;
    }
}
}  // namespace opentxs::network::otdht
