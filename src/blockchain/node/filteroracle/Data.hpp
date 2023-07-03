// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>

#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace database
{
class Cfilter;
}  // namespace database

namespace node
{
struct Endpoints;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::filteroracle
{
class Data
{
public:
    sTime last_sync_progress_;
    Map<cfilter::Type, block::Position> last_broadcast_;
    network::zeromq::socket::Raw to_blockchain_api_;
    network::zeromq::socket::Raw filter_notifier_internal_;
    network::zeromq::socket::Raw reindex_blocks_;

    auto DB() const noexcept -> const database::Cfilter&;

    auto DB() noexcept -> database::Cfilter&;
    auto Init() noexcept -> void;

    Data(
        const api::Session& api,
        const node::Endpoints& endpoints,
        database::Cfilter& db) noexcept;
    Data() = delete;
    Data(const Data&) = delete;
    Data(Data&&) = delete;
    auto operator=(const Data&) -> Data& = delete;
    auto operator=(Data&&) -> Data& = delete;

    ~Data();

private:
    database::Cfilter& db_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;
};
}  // namespace opentxs::blockchain::node::filteroracle
