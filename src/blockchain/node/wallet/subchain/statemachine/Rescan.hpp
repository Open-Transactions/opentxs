// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>
#include <queue>

#include "blockchain/node/wallet/subchain/statemachine/Scan.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace blockchain
{
namespace node
{
namespace wallet
{
class Process;
class Progress;
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Rescan final : public Scan
{
public:
    auto Queue(const block::Position& position) noexcept -> void;
    auto Reorg(const block::Position& parent) noexcept -> void final;
    auto Run() noexcept -> bool final;
    auto SetCeiling(const block::Position& ceiling) noexcept -> void;

    Rescan(
        SubchainStateData& parent,
        Process& process,
        Progress& progress) noexcept;

    ~Rescan() final = default;

private:
    Progress& progress_;
    std::queue<block::Position> rescan_requests_;
    block::Position ceiling_;

    auto type() const noexcept -> const char* final { return "rescan"; }

    auto finish(Lock& lock) noexcept -> void final;
    auto flush(const Lock& lock) noexcept -> void;
    auto process(const Lock& lock, const block::Position& position) noexcept
        -> void;

    Rescan() = delete;
    Rescan(const Rescan&) = delete;
    Rescan(Rescan&&) = delete;
    auto operator=(const Rescan&) -> Rescan& = delete;
    auto operator=(Rescan&&) -> Rescan& = delete;
};
}  // namespace opentxs::blockchain::node::wallet
