// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/Downloader.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstddef>
#include <iterator>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Thread.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Log.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node
{
using namespace std::literals;

Downloader::Downloader(
    const Log& log,
    std::string_view name,
    SetTipFunction setTip,
    AdjustTipFunction adjustTip,
    allocator_type alloc) noexcept
    : log_(log)
    , name_(name)
    , set_tip_(std::move(setTip))
    , adjust_tip_(std::move(adjustTip))
    , tip_()
    , index_(alloc)
    , downloading_(alloc)
    , ready_(alloc)
{
    OT_ASSERT(set_tip_);
    OT_ASSERT(adjust_tip_);
}

auto Downloader::AddBlocks(
    const HeaderOracle& oracle,
    alloc::Strategy monotonic) noexcept(false)
    -> std::tuple<block::Height, Vector<block::Hash>, bool>
{
    const auto& log = log_;
    const auto& next = next_position();
    log(OT_PRETTY_CLASS())(name_)(": current best position is ")(next).Flush();
    const auto data = oracle.Ancestors(next, 0_uz, monotonic.work_);

    OT_ASSERT(false == data.empty());

    const auto& ancestor = data.front();
    log(OT_PRETTY_CLASS())(name_)(": common ancestor is ")(ancestor).Flush();
    adjust_tip(ancestor);
    std::invoke(adjust_tip_, ancestor);
    const auto count = data.size();
    static const auto limit =
        std::min<std::size_t>(250_uz * MaxJobs(), 1001_uz);
    const auto effective = std::min(limit, count);
    auto out = Vector<block::Hash>{monotonic.work_};
    out.reserve((effective < 2_uz) ? 1_uz : effective - 1_uz);
    const auto height = [&]() -> block::Height {
        if (count < 2_uz) {

            return {};
        } else {

            return data[1_uz].height_;
        }
    }();

    for (auto n = 1_uz; n < effective; ++n) {
        const auto& position = data[n];
        log(OT_PRETTY_CLASS())(name_)(": adding block ")(
            position)(" to download queue")
            .Flush();
        out.emplace_back(position.hash_);
        index_[position.hash_] = position.height_;
        const auto [_, added] =
            downloading_.try_emplace(position.height_, position);

        OT_ASSERT(added);
    }

    return std::make_tuple(height, out, effective != count);
}

auto Downloader::adjust_tip(const block::Position& tip) noexcept -> void
{
    const auto& log = log_;
    const auto reset = [&] {
        const auto& existing = tip_;

        if (tip.height_ == existing.height_) {
            if (tip.hash_ == existing.hash_) {
                log(OT_PRETTY_CLASS())(name_)(": ancestor block ")(
                    tip)(" matches existing tip ")(existing)
                    .Flush();

                return false;
            } else {
                log(OT_PRETTY_CLASS())(name_)(": ancestor block ")(
                    tip)(" does not match existing tip ")(existing)
                    .Flush();

                return true;
            }
        } else if (tip.height_ < existing.height_) {

            return true;
        } else {

            return false;
        }
    }();

    if (reset) {
        std::invoke(set_tip_, tip);
        downloading_.clear();
        ready_.clear();
    } else {
        const auto download = [](auto i) -> const auto& { return i->second; };
        const auto ready = [](auto i) -> const auto& {
            return i->second.first;
        };
        const auto clean = [&](auto& map, auto& getPosition, auto name) {
            if (auto i = map.lower_bound(tip.height_); map.end() != i) {
                if (const auto& pos = getPosition(i); pos == tip) {
                    log(OT_PRETTY_CLASS())(name_)(": position ")(
                        pos)(" in the ")(
                        name)(" queue matches incoming ancestor")
                        .Flush();
                    ++i;
                }

                while (map.end() != i) {
                    const auto& stale = getPosition(i);
                    log(OT_PRETTY_CLASS())(name_)(": erasing stale position ")(
                        stale)(" from ")(name)(" queue")
                        .Flush();
                    index_.erase(stale.hash_);
                    i = map.erase(i);
                }
            } else {
                log(OT_PRETTY_CLASS())(name_)(": all ")(map.size())(
                    " blocks in the ")(name)(" queue are below height ")(
                    tip.height_)
                    .Flush();
            }
        };
        clean(downloading_, download, "download"sv);
        clean(ready_, ready, "ready"sv);
    }
}

auto Downloader::get_allocator() const noexcept -> allocator_type
{
    return index_.get_allocator();
}

auto Downloader::next_position() const noexcept -> const block::Position&
{
    const auto downloading = [this]() -> const auto& {
        return downloading_.crbegin()->second;
    };
    const auto ready = [this]() -> const auto& {
        return ready_.crbegin()->second.first;
    };

    if (downloading_.empty()) {
        if (ready_.empty()) {

            return tip_;
        } else {

            return ready();
        }
    } else {
        if (ready_.empty()) {

            return downloading();
        } else {

            return std::max(downloading(), ready());
        }
    }
}

auto Downloader::ReceiveBlock(
    const block::Hash& id,
    const BlockLocation& block,
    const ReceiveFunction& receive) noexcept(false) -> bool
{
    const auto& log = log_;

    if (receive) { std::invoke(receive, id, block); }

    if (auto i = index_.find(id); index_.end() != i) {
        const auto& height = i->second;

        return receive_block(id, block, {}, height, i);
    } else {
        log(OT_PRETTY_CLASS())(name_)(": block ")
            .asHex(id)(" is not in cache")
            .Flush();
    }

    return false;
}

auto Downloader::ReceiveBlock(
    const block::Hash& id,
    const BlockLocation& block,
    const ReceiveFunction& receive,
    block::Height height) noexcept(false) -> bool
{
    return receive_block(id, block, receive, height, std::nullopt);
}

auto Downloader::receive_block(
    const block::Hash& id,
    const BlockLocation& block,
    const ReceiveFunction& receive,
    block::Height blockheight,
    std::optional<Index::iterator> index) noexcept(false) -> bool
{
    const auto& log = log_;

    if (receive) { std::invoke(receive, id, block); }

    if (auto i = downloading_.find(blockheight); downloading_.end() != i) {
        const auto& [height, position] = *i;

        if (position.hash_ == id) {
            auto post = ScopeGuard{[&] { downloading_.erase(i); }};
            log(OT_PRETTY_CLASS())(name_)(": block ")
                .asHex(id)(" at height ")(height)(" downloaded")
                .Flush();
            const auto [j, added] =
                ready_.try_emplace(height, position, std::move(block));

            OT_ASSERT(added);

            if (index.has_value()) {
                index_.erase(*index);
            } else {
                index_.erase(id);
            }

            return true;
        } else {
            log(OT_PRETTY_CLASS())(name_)(": block ")
                .asHex(id)(" at height ")(
                    height)(" does not match expected hash for this height (")
                .asHex(position.hash_)(")")
                .Flush();

            return false;
        }
    } else {
        log(OT_PRETTY_CLASS())(name_)(": failed to locate block ")
            .asHex(id)(" in download queue")
            .Flush();

        return false;
    }
}

auto Downloader::Update() noexcept -> void
{
    const auto& log = log_;
    auto tip{tip_};

    for (auto i = ready_.begin(); i != ready_.end();) {
        auto& [height, data] = *i;
        auto& [position, bytes] = data;

        if (const auto target = tip.height_ + 1; height < target) {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": block at height ")(
                height)(" is in the ready queue even though it should have "
                        "been processed already since the current tip height "
                        "is ")(tip.height_)
                .Abort();
        } else if (height > target) {
            if (downloading_.contains(target)) {
                log(OT_PRETTY_CLASS())(name_)(": next block at height ")(
                    target)(" is downloading")
                    .Flush();

                break;
            } else {
                LogAbort()(OT_PRETTY_CLASS())(name_)(": next block at height ")(
                    target)(" does not exist in any queue")
                    .Abort();
            }
        } else {
            log(OT_PRETTY_CLASS())(name_)(": next block at height ")(
                target)(" is downloaded")
                .Flush();
        }

        tip = std::move(position);
        i = ready_.erase(i);
    }

    if (downloading_.empty() && ready_.empty()) { index_.clear(); }

    if (tip != tip_) { std::invoke(set_tip_, tip); }
}

Downloader::~Downloader() = default;
}  // namespace opentxs::blockchain::node
