// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace otdht
{
class Block;
class Data;
}  // namespace otdht
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database
{
class Sync
{
public:
    using Height = block::Height;
    using Message = network::otdht::Data;

    virtual auto SyncTip() const noexcept -> block::Position = 0;

    virtual auto LoadSync(const Height height, Message& output) noexcept
        -> bool = 0;
    virtual auto ReorgSync(const Height height) noexcept -> bool = 0;
    virtual auto SetSyncTip(const block::Position& position) noexcept
        -> bool = 0;
    virtual auto StoreSync(
        const block::Position& tip,
        const network::otdht::SyncData& items) noexcept -> bool = 0;

    virtual ~Sync() = default;
};
}  // namespace opentxs::blockchain::database
