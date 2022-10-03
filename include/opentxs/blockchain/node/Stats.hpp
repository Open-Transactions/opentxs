// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <cstddef>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
class Position;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
class OPENTXS_EXPORT Stats
{
public:
    class Imp;

    auto BlockHeaderTip(Type chain) const noexcept -> block::Position;
    auto BlockTip(Type chain) const noexcept -> block::Position;
    auto CfilterTip(Type chain) const noexcept -> block::Position;
    auto PeerCount(Type chain) const noexcept -> std::size_t;
    auto SyncTip(Type chain) const noexcept -> block::Position;

    Stats() noexcept;
    OPENTXS_NO_EXPORT Stats(Imp* imp) noexcept;
    Stats(const Stats&) noexcept;
    Stats(Stats&&) noexcept;
    auto operator=(const Stats&) noexcept -> Stats&;
    auto operator=(Stats&&) noexcept -> Stats&;

    ~Stats();

private:
    Imp* imp_;
};
}  // namespace opentxs::blockchain::node
