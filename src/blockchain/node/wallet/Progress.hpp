// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <vector>

#include "blockchain/node/wallet/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"

namespace opentxs
{
namespace blockchain
{
namespace node
{
namespace wallet
{
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::node::wallet
{
class Progress
{
public:
    auto Dirty() const noexcept -> std::optional<block::Position>;
    auto Get() const noexcept -> block::Position;

    auto Init() noexcept -> void;
    auto Reorg(const block::Position& parent) noexcept -> void;
    auto UpdateProcess(const ProgressBatch& processed) noexcept -> void;
    auto UpdateScan(
        const std::optional<block::Position>& highestClean,
        const std::pmr::vector<block::Position>& dirtyBlocks) noexcept -> void;

    Progress(const SubchainStateData& parent) noexcept;

    ~Progress();

private:
    struct Imp;

    Imp* imp_;

    Progress() = delete;
    Progress(const Progress&) = delete;
    Progress(Progress&&) = delete;
    auto operator=(const Progress&) -> Progress& = delete;
    auto operator=(Progress&&) -> Progress& = delete;
};
}  // namespace opentxs::blockchain::node::wallet
