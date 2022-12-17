// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/AsyncConst.hpp"
#include "opentxs/blockchain/Types.hpp"
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
namespace block
{
class Position;
}  // namespace block

namespace database
{
class Cfilter;
}  // namespace database

namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::filteroracle
{
class Data
{
public:
    AsyncConst<database::Cfilter*> db_;
    sTime last_sync_progress_;
    Map<cfilter::Type, block::Position> last_broadcast_;
    network::zeromq::socket::Raw to_blockchain_api_;
    network::zeromq::socket::Raw filter_notifier_internal_;
    network::zeromq::socket::Raw reindex_blocks_;

    Data(const api::Session& api, const node::Manager& node) noexcept;
    Data() = delete;
    Data(const Data&) = delete;
    Data(Data&&) = delete;
    auto operator=(const Data&) -> Data& = delete;
    auto operator=(Data&&) -> Data& = delete;

    ~Data();
};
}  // namespace opentxs::blockchain::node::filteroracle
