// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/node/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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

namespace node
{
class Manager;
}  // namespace node

class GCS;
}  // namespace blockchain

namespace network
{
namespace otdht
{
class Data;
}  // namespace otdht
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class FilterOracle : virtual public node::FilterOracle
{
public:
    virtual auto GetFilterJob() const noexcept -> CfilterJob = 0;
    virtual auto GetHeaderJob() const noexcept -> CfheaderJob = 0;
    auto Internal() const noexcept -> const internal::FilterOracle& final
    {
        return *this;
    }
    virtual auto ProcessBlock(
        const bitcoin::block::Block& block,
        alloc::Default monotonic) const noexcept -> bool = 0;
    virtual auto ProcessSyncData(
        const block::Hash& prior,
        const Vector<block::Hash>& hashes,
        const network::otdht::Data& data,
        alloc::Default monotonic) const noexcept -> void = 0;
    virtual auto Tip(const cfilter::Type type) const noexcept
        -> block::Position = 0;

    virtual auto Heartbeat() noexcept -> void = 0;
    virtual auto Init(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node) noexcept -> void = 0;
    auto Internal() noexcept -> internal::FilterOracle& final { return *this; }
    virtual auto Start() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> void = 0;

    ~FilterOracle() override = default;
};
}  // namespace opentxs::blockchain::node::internal
