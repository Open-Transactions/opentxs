// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <string_view>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace p2p
{
namespace client
{
class Server;
}  // namespace client

class State;
}  // namespace p2p
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

class opentxs::network::p2p::client::Server
{
public:
    using Chain = opentxs::blockchain::Type;
    using Height = opentxs::blockchain::block::Height;

    const CString endpoint_;
    Time last_sent_;
    Time last_received_;
    bool connected_;
    bool active_;
    bool waiting_;
    bool new_local_handler_;
    CString publisher_;

    auto Chains() const noexcept -> Vector<Chain>;
    auto is_stalled() const noexcept -> bool;

    auto needs_query() noexcept -> bool;
    auto needs_retry() noexcept -> bool;
    auto ProcessState(const opentxs::network::p2p::State& state) noexcept
        -> Height;
    auto SetStalled() noexcept -> void;

    Server() noexcept;
    Server(const std::string_view endpoint) noexcept;

private:
    Map<Chain, opentxs::blockchain::block::Position> chains_;

    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;
};
