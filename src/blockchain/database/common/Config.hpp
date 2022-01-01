// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/thread/thread.hpp>
#include <array>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <vector>

#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/network/p2p/Data.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/Bytes.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace p2p
{
class Block;
class Data;
}  // namespace p2p
}  // namespace network

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs

namespace opentxs::blockchain::database::common
{
class Configuration
{
public:
    using Endpoints = std::pmr::vector<std::string>;

    auto AddSyncServer(const std::string& endpoint) const noexcept -> bool;
    auto DeleteSyncServer(const std::string& endpoint) const noexcept -> bool;
    auto GetSyncServers() const noexcept -> Endpoints;

    Configuration(const api::Session& api, storage::lmdb::LMDB& lmdb) noexcept;

private:
    const api::Session& api_;
    storage::lmdb::LMDB& lmdb_;
    const int config_table_;
    const OTZMQPublishSocket socket_;
};
}  // namespace opentxs::blockchain::database::common
