// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#if OT_BLOCKCHAIN

#include <mutex>

#include "opentxs/blockchain/node/HeaderOracle.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace network
{
namespace p2p
{
class Data;
}  // namespace p2p
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class HeaderOracle : virtual public node::HeaderOracle
{
public:
    using CheckpointBlockHash = block::pHash;
    using PreviousBlockHash = block::pHash;
    using CheckpointFilterHash = block::pHash;
    using CheckpointData = std::tuple<
        block::Height,
        CheckpointBlockHash,
        PreviousBlockHash,
        CheckpointFilterHash>;

    using node::HeaderOracle::CalculateReorg;
    virtual auto CalculateReorg(const Lock& lock, const block::Position& tip)
        const noexcept(false) -> Positions = 0;
    virtual auto GetMutex() const noexcept -> std::mutex& = 0;
    using node::HeaderOracle::GetPosition;
    virtual auto GetPosition(const Lock& lock, const block::Height height)
        const noexcept -> block::Position = 0;

    virtual auto GetDefaultCheckpoint() const noexcept -> CheckpointData = 0;
    virtual auto Init() noexcept -> void = 0;
    virtual auto LoadBitcoinHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::bitcoin::Header> = 0;
    virtual auto ProcessSyncData(
        block::Hash& prior,
        UnallocatedVector<block::pHash>& hashes,
        const network::p2p::Data& data) noexcept -> std::size_t = 0;

    ~HeaderOracle() override = default;
};
}  // namespace opentxs::blockchain::node::internal
#endif  // OT_BLOCKCHAIN
