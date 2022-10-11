// Copyright (c) 2010-2022 The Open-Transactions developers
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
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string_view>

#include "internal/network/zeromq/socket/Publish.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/network/otdht/Block.hpp"
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace otdht
{
class Block;
class Data;
}  // namespace otdht
}  // namespace network

namespace storage
{
namespace lmdb
{
class Database;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::common
{
class Configuration
{
public:
    using Endpoints = Vector<CString>;

    auto AddSyncServer(std::string_view endpoint) const noexcept -> bool;
    auto DeleteSyncServer(std::string_view endpoint) const noexcept -> bool;
    auto GetSyncServers(alloc::Default alloc) const noexcept -> Endpoints;

    Configuration(
        const api::Session& api,
        storage::lmdb::Database& lmdb) noexcept;

private:
    const api::Session& api_;
    storage::lmdb::Database& lmdb_;
    const int config_table_;
    const OTZMQPublishSocket socket_;
};
}  // namespace opentxs::blockchain::database::common
