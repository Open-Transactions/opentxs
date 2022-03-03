// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/node/UpdateTransaction.hpp"  // IWYU pragma: associated

#include <iterator>
#include <tuple>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::blockchain::node
{
UpdateTransaction::UpdateTransaction(
    const api::Session& api,
    const internal::HeaderDatabase& db)
    : api_(api)
    , db_(db)
    , have_reorg_(false)
    , have_checkpoint_(false)
    , reorg_from_(make_blank<block::Position>::value(api))
    , checkpoint_(make_blank<block::Position>::value(api))
    , headers_()
    , best_()
    , add_sib_()
    , delete_sib_()
    , connect_()
    , disconnected_()
    , cached_disconnected_()
    , cached_siblings_()
{
}

auto UpdateTransaction::AddSibling(const block::Position& position) -> void
{
    {
        auto& cached = siblings();
        cached.emplace(position.second);
    }

    add_sib_.emplace(position.second);
    delete_sib_.erase(position.second);
}

auto UpdateTransaction::AddToBestChain(const block::Position& position) -> void
{
    RemoveSibling(position.second);
    best_.emplace(position);
}

auto UpdateTransaction::Checkpoint() const -> block::Position
{
    if (have_checkpoint_) {

        return checkpoint_;
    } else {

        return db_.CurrentCheckpoint();
    }
}

auto UpdateTransaction::ClearCheckpoint() -> void
{
    have_checkpoint_ = true;
    checkpoint_ = make_blank<block::Position>::value(api_);
}

auto UpdateTransaction::ConnectBlock(ChainSegment&& segment) -> void
{
    {
        auto& cached = disconnected();
        auto [first, last] = cached.equal_range(segment.first);

        for (auto it{first}; it != last;) {
            if (it->second == segment.second) {
                it = cached.erase(it);
            } else {
                ++it;
            }
        }
    }

    if (0 < disconnected_.count(segment)) {
        disconnected_.erase(segment);
    } else {
        connect_.emplace(std::move(segment));
    }
}

auto UpdateTransaction::DisconnectBlock(const block::Header& header) -> void
{
    ChainSegment item{header.ParentHash(), header.Hash()};

    {
        auto& cached = disconnected();
        cached.emplace(item);
    }

    connect_.erase(item);
    disconnected_.emplace(std::move(item));
}

auto UpdateTransaction::disconnected() const noexcept -> DisconnectedList&
{
    if (false == cached_disconnected_.has_value()) {
        cached_disconnected_ = db_.DisconnectedHashes();
    }

    return cached_disconnected_.value();
}

auto UpdateTransaction::EffectiveBestBlock(const block::Height height) const
    noexcept(false) -> block::pHash
{
    try {

        return best_.at(height);
    } catch (...) {
    }

    return db_.BestBlock(height);
}

auto UpdateTransaction::EffectiveCheckpoint() const noexcept -> bool
{
    static const auto blank = make_blank<block::Position>::value(api_);

    if (have_checkpoint_) {

        return blank.first != checkpoint_.first;
    } else {

        return db_.HaveCheckpoint();
    }
}

auto UpdateTransaction::EffectiveHasDisconnectedChildren(
    const block::Hash& hash) const noexcept -> bool
{
    const auto& cached = disconnected();
    const auto [first, last] = cached.equal_range(hash);

    return 0 < std::distance(first, last);
}

auto UpdateTransaction::EffectiveHeaderExists(
    const block::Hash& hash) const noexcept -> bool
{
    if (0 < headers_.count(hash)) { return true; }

    return db_.HeaderExists(hash);
}

auto UpdateTransaction::Header(const block::Hash& hash) noexcept(false)
    -> block::Header&
{
    auto& output = headers_.at(hash).first;

    OT_ASSERT(output);

    return *output;
}

auto UpdateTransaction::RemoveSibling(const block::Hash& hash) -> void
{
    {
        auto& cached = siblings();
        cached.erase(hash);
    }

    delete_sib_.emplace(hash);
    add_sib_.erase(hash);
}

auto UpdateTransaction::SetCheckpoint(block::Position&& checkpoint) -> void
{
    have_checkpoint_ = true;
    checkpoint_ = std::move(checkpoint);
}

auto UpdateTransaction::SetReorgParent(const block::Position& pos) noexcept
    -> void
{
    for (auto it{best_.begin()}; it != best_.end();) {
        if (it->first > pos.first) {
            it = best_.erase(it);
        } else {
            ++it;
        }
    }

    have_reorg_ = true;
    static const auto blank = make_blank<block::Position>::value(api_);
    const auto set =
        (blank.first == reorg_from_.first) || (pos.first <= reorg_from_.first);

    if (set) { reorg_from_ = pos; }
}

auto UpdateTransaction::siblings() const noexcept -> Hashes&
{
    if (false == cached_siblings_.has_value()) {
        cached_siblings_ = db_.SiblingHashes();
    }

    return cached_siblings_.value();
}

auto UpdateTransaction::Stage() noexcept -> block::Header&
{
    if (0 == best_.size()) {
        auto best = db_.CurrentBest();

        OT_ASSERT(best);

        {
            auto it = headers_.find(best->Hash());

            if (headers_.end() != it) { return *it->second.first; }
        }

        return stage(false, std::move(best));
    }

    return Stage(best_.crbegin()->second);
}

auto UpdateTransaction::Stage(std::unique_ptr<block::Header> header) noexcept
    -> block::Header&
{
    return stage(true, std::move(header));
}

auto UpdateTransaction::Stage(const block::Hash& hash) noexcept(false)
    -> block::Header&
{
    {
        auto it = headers_.find(hash);

        if (headers_.end() != it) { return *it->second.first; }
    }

    return stage(false, db_.LoadHeader(hash));
}

auto UpdateTransaction::Stage(const block::Height& height) noexcept(false)
    -> block::Header&
{
    return Stage(EffectiveBestBlock(height));
}

auto UpdateTransaction::stage(
    const bool newHeader,
    std::unique_ptr<block::Header> header) noexcept -> block::Header&
{
    OT_ASSERT(header);

    auto [it, added] = headers_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(header->Hash()),
        std::forward_as_tuple(std::move(header), newHeader));

    OT_ASSERT(added);

    return *it->second.first;
}
}  // namespace opentxs::blockchain::node
