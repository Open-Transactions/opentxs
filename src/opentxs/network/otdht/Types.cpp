// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/otdht/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <string_view>
#include <utility>

#include "opentxs/network/otdht/MessageType.hpp"  // IWYU pragma: keep

namespace opentxs::network::otdht
{
using namespace std::literals;

auto print(MessageType in) noexcept -> std::string_view
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

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown otdht::MessageType"sv;
    }
}
}  // namespace opentxs::network::otdht
