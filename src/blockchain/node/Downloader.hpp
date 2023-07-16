// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace node
{
class HeaderOracle;
}  // namespace node
}  // namespace blockchain

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node
{
class Downloader final : public opentxs::Allocated
{
public:
    using BlockLocation = blockoracle::BlockLocation;
    using ReceiveFunction =
        std::function<void(const block::Hash&, const BlockLocation&)>;
    using SetTipFunction = std::function<void(const block::Position&)>;
    using AdjustTipFunction = SetTipFunction;

    auto get_allocator() const noexcept -> allocator_type final;
    auto Tip() const noexcept -> const block::Position& { return tip_; }

    auto AddBlocks(
        const HeaderOracle& oracle,
        allocator_type monotonic) noexcept(false)
        -> std::tuple<block::Height, Vector<block::Hash>, bool>;
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    auto ReceiveBlock(
        const block::Hash& id,
        const BlockLocation& block,
        const ReceiveFunction& receive) noexcept(false) -> bool;
    auto ReceiveBlock(
        const block::Hash& id,
        const BlockLocation& block,
        const ReceiveFunction& receive,
        block::Height height) noexcept(false) -> bool;
    auto SetTip(const block::Position& tip) noexcept -> void { tip_ = tip; }
    auto Update() noexcept -> void;

    Downloader(
        const Log& log,
        std::string_view name,
        SetTipFunction setTip,
        AdjustTipFunction adjustTip,
        allocator_type alloc) noexcept;
    Downloader() = delete;
    Downloader(const Downloader&) = delete;
    Downloader(Downloader&&) = delete;
    auto operator=(const Downloader&) -> Downloader& = delete;
    auto operator=(Downloader&&) -> Downloader& = delete;

    ~Downloader() final;

private:
    using Index = Map<block::Hash, block::Height>;
    using Downloading = Map<block::Height, block::Position>;
    using Ready = Map<block::Height, std::pair<block::Position, BlockLocation>>;

    const Log& log_;
    const std::string_view name_;
    const SetTipFunction set_tip_;
    const SetTipFunction adjust_tip_;
    block::Position tip_;
    Index index_;
    Downloading downloading_;
    Ready ready_;

    auto next_position() const noexcept -> const block::Position&;

    auto adjust_tip(const block::Position& tip) noexcept -> void;
    auto receive_block(
        const block::Hash& id,
        const BlockLocation& block,
        const ReceiveFunction& receive,
        block::Height blockheight,
        std::optional<Index::iterator> index) noexcept(false) -> bool;
};
}  // namespace opentxs::blockchain::node
