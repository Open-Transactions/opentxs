// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>

#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"
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
class Rescan;
class SubchainStateData;
class Work;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Scan : public Job
{
public:
    auto Position() const noexcept -> block::Position;

    auto Reorg(const block::Position& parent) noexcept -> void override;
    using Job::Run;
    auto Run() noexcept -> bool override;
    auto Run(const block::Position& filterTip) noexcept -> bool;
    auto SetReady() noexcept -> void;
    auto UpdateTip(const block::Position& tip) noexcept -> void;

    Scan(SubchainStateData& parent, Process& process, Rescan& rescan) noexcept;

    ~Scan() override = default;

protected:
    std::optional<block::Position> last_scanned_;
    std::optional<block::Position> filter_tip_;
    bool ready_;

    auto Do(
        const block::Position best,
        const block::Height stop,
        block::Position highestTested) noexcept -> void;
    auto set_ready(const Lock& lock) noexcept -> void;
    auto update_tip() noexcept -> block::Position;
    auto update_tip(const Lock& lock) noexcept -> block::Position;

private:
    Process& process_;
    Rescan& rescan_;

    auto type() const noexcept -> const char* override { return "scan"; }

    auto finish(Lock& lock) noexcept -> void override;

    Scan() = delete;
    Scan(const Scan&) = delete;
    Scan(Scan&&) = delete;
    auto operator=(const Scan&) -> Scan& = delete;
    auto operator=(Scan&&) -> Scan& = delete;
};
}  // namespace opentxs::blockchain::node::wallet
