// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/headeroracle/Shared.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <compare>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>

#include "blockchain/node/UpdateTransaction.hpp"
#include "blockchain/node/headeroracle/HeaderJob.hpp"
#include "internal/blockchain/block/Header.hpp"
#include "internal/blockchain/database/Header.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/headeroracle/HeaderJob.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/blockchain/node/headeroracle/Types.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/Header.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Header.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/network/otdht/Block.hpp"
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/network/otdht/State.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.internal.hpp"

namespace opentxs::blockchain::node::internal
{
HeaderOracle::Shared::Shared(
    const api::Session& api,
    const blockchain::Type chain,
    const node::Endpoints& endpoints,
    database::Header& database,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Allocated(alloc)
    , batch_(batch)
    , parent_(nullptr)
    , data_(api, chain, endpoints, database)
{
}

auto HeaderOracle::Shared::Ancestors(
    const block::Position& start,
    const block::Position& target,
    const std::size_t limit,
    alloc::Default alloc) const noexcept(false) -> Positions
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return ancestors(data, start, target, limit, alloc);
}

auto HeaderOracle::Shared::Ancestors(
    const block::Position& start,
    const std::size_t limit,
    alloc::Default alloc) const noexcept(false) -> Positions
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return ancestors(data, start, best_chain(data), limit, alloc);
}

auto HeaderOracle::Shared::ancestors(
    const HeaderOraclePrivate& data,
    const block::Position& start,
    const block::Position& target,
    const std::size_t limit,
    alloc::Default alloc) const noexcept(false) -> Positions
{
    const auto check =
        std::max<block::Height>(std::min(start.height_, target.height_), 0);
    const auto fast = is_in_best_chain(data, target.hash_).first &&
                      is_in_best_chain(data, start.hash_).first &&
                      (start.height_ < target.height_);

    if (fast) {
        auto output = best_chain(data, start, limit, alloc);

        while ((1 < output.size()) &&
               (output.back().height_ > target.height_)) {
            output.pop_back();
        }

        assert_true(0 < output.size());
        assert_true(output.front().height_ <= check);

        return output;
    }

    // TODO monotonic allocator
    auto cache = Deque<block::Position>{alloc};
    auto current = data.database_.LoadHeader(target.hash_);
    auto sibling = data.database_.LoadHeader(start.hash_);

    while (sibling.Height() > current.Height()) {
        sibling = data.database_.TryLoadHeader(sibling.ParentHash());

        if (false == sibling.IsValid()) {
            sibling = data.database_.TryLoadHeader(
                params::get(data.chain_).GenesisHash());

            assert_true(sibling.IsValid());

            break;
        }
    }

    assert_true(sibling.Height() <= current.Height());

    while (current.Height() >= 0) {
        cache.emplace_front(current.Position());

        if (current.Position() == sibling.Position()) {
            break;
        } else if (current.Height() == sibling.Height()) {
            sibling = data.database_.TryLoadHeader(sibling.ParentHash());

            if (false == sibling.IsValid()) {
                sibling = data.database_.TryLoadHeader(
                    params::get(data.chain_).GenesisHash());

                assert_true(sibling.IsValid());
            }
        }

        current = data.database_.TryLoadHeader(current.ParentHash());

        if (false == current.IsValid()) { break; }
    }

    assert_true(0 < cache.size());

    auto output = Positions{alloc};
    std::ranges::move(cache, std::back_inserter(output));

    assert_true(output.front().height_ <= check);

    return output;
}

auto HeaderOracle::Shared::AddCheckpoint(
    const block::Height position,
    const block::Hash& requiredHash) noexcept -> bool
{
    auto handle = data_.lock();
    auto& data = *handle;

    return add_checkpoint(data, position, requiredHash);
}

auto HeaderOracle::Shared::add_checkpoint(
    HeaderOraclePrivate& data,
    const block::Height position,
    const block::Hash& requiredHash) noexcept -> bool
{
    auto update = UpdateTransaction{data.api_, data.database_};

    if (update.EffectiveCheckpoint()) {
        LogError()()("Checkpoint already exists").Flush();

        return false;
    }

    if (2 > position) {
        LogError()()("Invalid position").Flush();

        return false;
    }

    update.SetCheckpoint({position, requiredHash});

    if (apply_checkpoint(data, position, update)) {

        return apply_update(data, update);
    } else {

        return false;
    }
}

auto HeaderOracle::Shared::AddHeader(block::Header header) noexcept -> bool
{
    return AddHeaders({std::addressof(header), 1_uz});
}

auto HeaderOracle::Shared::AddHeaders(std::span<block::Header> headers) noexcept
    -> bool
{
    if (headers.empty()) { return false; }

    auto handle = data_.lock();
    auto& data = *handle;
    auto update = UpdateTransaction{data.api_, data.database_};

    for (auto& header : headers) {
        if (false == header.IsValid()) {
            LogError()()("Invalid header").Flush();

            return false;
        }

        if (false == add_header(data, update, std::move(header))) {

            return false;
        }
    }

    return apply_update(data, update);
}

auto HeaderOracle::Shared::add_header(
    const HeaderOraclePrivate& data,
    UpdateTransaction& update,
    block::Header pHeader) noexcept -> bool
{
    if (update.EffectiveHeaderExists(pHeader.Hash())) {
        LogVerbose()()("Header already processed").Flush();

        return true;
    }

    auto& header = update.Stage(std::move(pHeader));
    const auto& current = update.Stage();
    const auto* pParent = is_disconnected(header.ParentHash(), update);

    if (nullptr == pParent) {
        LogVerbose()()("Adding disconnected header").Flush();
        header.Internal().SetDisconnectedState();
        update.DisconnectBlock(header);

        return true;
    }

    assert_false(nullptr == pParent);

    const auto& parent = *pParent;

    if (update.EffectiveIsSibling(header.ParentHash())) {
        update.RemoveSibling(header.ParentHash());
    }

    auto candidates = Candidates{};

    try {
        auto& candidate = initialize_candidate(
            data, current, parent, update, candidates, header);
        connect_children(data, header, candidates, candidate, update);
    } catch (...) {
        LogError()()("Failed to connect children").Flush();

        return false;
    }

    return choose_candidate(current, candidates, update).first;
}

auto HeaderOracle::Shared::apply_checkpoint(
    const HeaderOraclePrivate& data,
    const block::Height position,
    UpdateTransaction& update) noexcept -> bool
{
    auto& best = update.Stage();

    if (position > best.Height()) { return true; }

    try {
        const auto& siblings = update.EffectiveSiblingHashes();
        auto count = std::atomic<std::size_t>{siblings.size()};
        LogConsole()("* Comparing current chain and ")(
            count)(" sibling chains to checkpoint")
            .Flush();
        const auto& ancestor = update.Stage(position - 1);
        auto candidates = Candidates{};
        candidates.reserve(count + 1u);
        stage_candidate(data, ancestor, candidates, update, best);
        LogConsole()("  * ")(count)(" remaining").Flush();

        for (const auto& hash : siblings) {
            stage_candidate(
                data, ancestor, candidates, update, update.Stage(hash));
            LogConsole()("  * ")(--count)(" remaining").Flush();
        }

        for (auto& [invalid, chain] : candidates) {
            const block::Header* pParent = &ancestor;

            for (const auto& [height, hash] : chain) {
                auto& child = update.Header(hash);
                invalid = connect_to_parent(data, update, *pParent, child);
                pParent = &child;
            }
        }

        const auto [success, found] =
            choose_candidate(ancestor, candidates, update);

        if (false == success) { return false; }

        if (false == found) {
            const auto fallback = ancestor.Position();
            update.SetReorgParent(fallback);
            update.AddToBestChain(fallback);
        }

        return true;
    } catch (...) {
        LogError()()("Failed to process sibling chains").Flush();

        return false;
    }
}

auto HeaderOracle::Shared::apply_update(
    HeaderOraclePrivate& data,
    UpdateTransaction& update) noexcept -> bool
{
    const auto before = data.best_;
    const auto out = data.database_.ApplyUpdate(update);
    data.best_ = data.database_.CurrentBest().Position();
    data.PruneKnownHashes();

    if (before != data.best_) {
        LogVerbose()()(print(data.chain_))(" block header chain updated to ")(
            best_chain(data))
            .Flush();
        data.to_parent_.SendDeferred([&] {
            using Job = ManagerJobs;
            auto work = MakeWork(Job::statemachine);

            return work;
        }());
        data.to_actor_.SendDeferred([&] {
            using Job = headeroracle::Job;
            auto work = MakeWork(Job::statemachine);

            return work;
        }());
    }

    return out;
}

auto HeaderOracle::Shared::BestChain() const noexcept -> block::Position
{
    auto handle = data_.lock_shared();

    return best_chain(*handle);
}

auto HeaderOracle::Shared::BestChain(
    const block::Position& tip,
    const std::size_t limit,
    alloc::Default alloc) const noexcept(false) -> Positions
{
    auto handle = data_.lock_shared();

    return best_chain(*handle, tip, limit, alloc);
}

auto HeaderOracle::Shared::best_chain(
    const HeaderOraclePrivate& data) const noexcept -> block::Position
{
    return data.best_;
}

auto HeaderOracle::Shared::best_chain(
    const HeaderOraclePrivate& data,
    const block::Position& tip,
    const std::size_t limit,
    alloc::Default alloc) const noexcept -> Positions
{
    const auto [youngest, best] = common_parent(data, tip);
    static const auto blank = block::Hash{};
    auto height = std::max<block::Height>(youngest.height_, 0);
    auto output = Positions{};

    for (auto& hash : best_hashes(data, height, blank, 0, alloc)) {
        output.emplace_back(height++, std::move(hash));

        if ((0_uz < limit) && (output.size() == limit)) { break; }
    }

    assert_true(0 < output.size());

    return output;
}

auto HeaderOracle::Shared::BestHash(const block::Height height) const noexcept
    -> block::Hash
{
    auto handle = data_.lock_shared();

    return best_hash(*handle, height);
}

auto HeaderOracle::Shared::BestHash(
    const block::Height height,
    const block::Position& check) const noexcept -> block::Hash
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    if (is_in_best_chain(data, check)) {

        return data.database_.BestBlock(height);
    } else {

        return blank_hash();
    }
}

auto HeaderOracle::Shared::best_hash(
    const HeaderOraclePrivate& data,
    const block::Height height) const noexcept -> block::Hash
{
    try {
        return data.database_.BestBlock(height);
    } catch (...) {
        return blank_hash();
    }
}

auto HeaderOracle::Shared::BestHashes(
    const block::Height start,
    const std::size_t limit,
    alloc::Default alloc) const noexcept -> Hashes
{
    static const auto blank = block::Hash{};

    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return best_hashes(data, start, blank, limit, alloc);
}

auto HeaderOracle::Shared::BestHashes(
    const block::Height start,
    const block::Hash& stop,
    const std::size_t limit,
    alloc::Default alloc) const noexcept -> Hashes
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return best_hashes(data, start, stop, limit, alloc);
}

auto HeaderOracle::Shared::BestHashes(
    const std::span<const block::Hash> previous,
    const block::Hash& stop,
    const std::size_t limit,
    alloc::Default alloc) const noexcept -> Hashes
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;
    auto start = block::Height{};

    for (const auto& hash : previous) {
        const auto [best, height] = is_in_best_chain(data, hash);

        if (best) {
            start = height;
            break;
        }
    }

    return best_hashes(data, start, stop, limit, alloc);
}

auto HeaderOracle::Shared::best_hashes(
    const HeaderOraclePrivate& data,
    const block::Height start,
    const block::Hash& stop,
    const std::size_t limit,
    alloc::Default alloc) const noexcept -> Hashes
{
    auto output = Hashes{alloc};
    const auto limitIsZero = (0 == limit);
    auto current{start};
    const auto tip = best_chain(data);
    const auto last = [&] {
        if (limitIsZero) {

            return tip.height_;
        } else {
            const auto requestedEnd = block::Height{
                current + static_cast<block::Height>(limit) -
                static_cast<block::Height>(1)};

            return std::min<block::Height>(requestedEnd, tip.height_);
        }
    }();

    while (current <= last) {
        auto hash = data.database_.BestBlock(current++);

        // TODO this check shouldn't be necessary but BestBlock doesn't
        // throw the exception documented in its declaration.
        if (hash.IsNull()) { break; }

        const auto stopHere = stop.IsNull() ? false : (stop == hash);
        output.emplace_back(std::move(hash));

        if (stopHere) { break; }
    }

    return output;
}

auto HeaderOracle::Shared::blank_hash() const noexcept -> const block::Hash&
{
    static const auto blank = block::Hash{};

    assert_true(blank.IsNull());

    return blank;
}

auto HeaderOracle::Shared::blank_position() const noexcept
    -> const block::Position&
{
    static const auto blank = block::Position{};

    return blank;
}

auto HeaderOracle::Shared::CalculateReorg(
    const block::Position& tip,
    alloc::Default alloc) const noexcept(false) -> Positions
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return calculate_reorg(data, tip, alloc);
}

auto HeaderOracle::Shared::CalculateReorg(
    const HeaderOraclePrivate& data,
    const block::Position& tip,
    alloc::Default alloc) const noexcept(false) -> Positions
{
    return calculate_reorg(data, tip, alloc);
}

auto HeaderOracle::Shared::calculate_reorg(
    const HeaderOraclePrivate& data,
    const block::Position& tip,
    alloc::Default alloc) const noexcept(false) -> Positions
{
    auto output = Positions{alloc};

    if (is_in_best_chain(data, tip)) { return output; }

    output.emplace_back(tip);

    for (auto height{tip.height_}; height >= 0; --height) {
        if (0 == height) {
            throw std::runtime_error(
                "Provided tip does not connect to genesis block");
        }

        const auto& child = *output.crbegin();
        const auto header = data.database_.TryLoadHeader(child.hash_);

        if (false == header.IsValid()) {
            throw std::runtime_error("Failed to load block header");
        }

        if (height != header.Height()) {
            throw std::runtime_error("Wrong height specified for block hash");
        }

        auto parent = block::Position{height - 1, header.ParentHash()};

        if (is_in_best_chain(data, parent)) { break; }

        output.emplace_back(std::move(parent));
    }

    return output;
}

auto HeaderOracle::Shared::choose_candidate(
    const block::Header& current,
    const Candidates& candidates,
    UpdateTransaction& update) noexcept(false) -> std::pair<bool, bool>
{
    auto output = std::pair<bool, bool>{false, false};
    auto& [success, found] = output;

    try {
        const block::Header* pBest{&current};

        for (const auto& candidate : candidates) {
            if (candidate.blacklisted_) { continue; }

            assert_true(0 < candidate.chain_.size());

            const auto& position = *candidate.chain_.crbegin();
            const auto& tip = update.Header(position.hash_);

            if (evaluate_candidate(*pBest, tip)) { pBest = &tip; }
        }

        assert_false(nullptr == pBest);

        const auto& best = *pBest;

        for (const auto& candidate : candidates) {
            assert_true(0 < candidate.chain_.size());

            const auto& position = *candidate.chain_.crbegin();
            const auto& tip = update.Header(position.hash_);

            if (tip.Hash() == best.Hash()) {
                found = true;
                auto reorg{false};

                for (const auto& segment : candidate.chain_) {
                    const auto& [height, hash] = segment;

                    if ((height <= current.Height()) && (false == reorg)) {
                        if (hash == update.EffectiveBestBlock(height)) {
                            continue;
                        } else {
                            reorg = true;
                            const auto parent = block::Position{
                                height - 1,
                                update.EffectiveBestBlock(height - 1)};
                            update.SetReorgParent(parent);
                            update.AddToBestChain(segment);
                            update.AddSibling(current.Position());
                            LogVerbose()()("Block ")(hash.asHex())(
                                " at position ")(
                                height)(" causes a chain reorg.")
                                .Flush();
                        }
                    } else {
                        update.AddToBestChain(segment);
                        LogVerbose()()("Adding block ")(hash.asHex())(
                            " to best chain at position ")(height)
                            .Flush();
                    }
                }
            } else {
                const auto orphan = tip.Position();
                update.AddSibling(orphan);
                const auto& [height, hash] = orphan;
                LogVerbose()()("Adding block ")(hash.asHex())(
                    " as an orphan at position ")(height)
                    .Flush();
            }
        }
    } catch (...) {
        LogError()()("Error evaluating candidates").Flush();

        return output;
    }

    success = true;

    return output;
}

auto HeaderOracle::Shared::CommonParent(const block::Position& position)
    const noexcept -> std::pair<block::Position, block::Position>
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return common_parent(data, position);
}

auto HeaderOracle::Shared::common_parent(
    const HeaderOraclePrivate& data,
    const block::Position& position) const noexcept
    -> std::pair<block::Position, block::Position>
{
    const auto& database = data.database_;
    auto output = std::pair<block::Position, block::Position>{
        {0, params::get(data.chain_).GenesisHash()}, best_chain(data)};
    auto& [parent, best] = output;
    auto test{position};
    auto header = database.TryLoadHeader(test.hash_);

    if (false == header.IsValid()) { return output; }

    while (0 < test.height_) {
        if (is_in_best_chain(data, test.hash_).first) {
            parent = test;

            return output;
        }

        header = database.TryLoadHeader(header.ParentHash());

        if (header.IsValid()) {
            test = header.Position();
        } else {
            return output;
        }
    }

    return output;
}

auto HeaderOracle::Shared::connect_children(
    const HeaderOraclePrivate& data,
    block::Header& parent,
    Candidates& candidates,
    Candidate& candidate,
    UpdateTransaction& update) -> void
{
    auto& chain = candidate.chain_;
    const auto& end = *chain.crbegin();

    assert_true(end.height_ + 1 == parent.Position().height_);

    chain.emplace_back(parent.Position());

    if (false == update.EffectiveHasDisconnectedChildren(parent.Hash())) {
        return;
    }

    const auto disconnected = update.EffectiveDisconnectedHashes();
    const auto [first, last] = disconnected.equal_range(parent.Hash());
    std::atomic<bool> firstChild{true};
    const auto original{candidate};
    std::for_each(first, last, [&](const auto& in) -> void {
        const auto& [parentHash, childHash] = in;
        update.ConnectBlock({parentHash, childHash});
        auto& child = update.Stage(childHash);
        candidate.blacklisted_ = connect_to_parent(data, update, parent, child);
        // The first child block extends the current candidate. Subsequent child
        // blocks create a new candidate to extend. This transforms the tree
        // of disconnected blocks into a table of candidates.
        auto& chainToExtend = firstChild.exchange(false)
                                  ? candidate
                                  : candidates.emplace_back(original);
        connect_children(data, child, candidates, chainToExtend, update);
    });
}

auto HeaderOracle::Shared::connect_to_parent(
    const HeaderOraclePrivate& data,
    const UpdateTransaction& update,
    const block::Header& parent,
    block::Header& child) noexcept -> bool
{
    child.Internal().InheritWork(parent.Work());
    child.Internal().InheritState(parent);
    child.Internal().InheritHeight(parent);
    child.Internal().CompareToCheckpoint(update.Checkpoint());

    return child.Internal().IsBlacklisted();
}

auto HeaderOracle::Shared::DeleteCheckpoint() noexcept -> bool
{
    auto handle = data_.lock();

    return delete_checkpoint(*handle);
}

auto HeaderOracle::Shared::delete_checkpoint(HeaderOraclePrivate& data) noexcept
    -> bool
{
    auto update = UpdateTransaction{data.api_, data.database_};

    if (false == update.EffectiveCheckpoint()) {
        LogError()()("No checkpoint").Flush();

        return false;
    }

    const auto position = update.Checkpoint().height_;
    update.ClearCheckpoint();

    if (apply_checkpoint(data, position, update)) {

        return apply_update(data, update);
    } else {

        return false;
    }
}

auto HeaderOracle::Shared::evaluate_candidate(
    const block::Header& current,
    const block::Header& candidate) noexcept -> bool
{
    return candidate.Work() > current.Work();
}

auto HeaderOracle::Shared::Execute(Vector<ReorgTask>&& jobs) const noexcept
    -> bool
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    for (auto& job : jobs) {
        if (false == std::invoke(job, *parent_, data)) { return false; }
    }

    return true;
}

auto HeaderOracle::Shared::Exists(const block::Hash& hash) const noexcept
    -> bool
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return data.database_.HeaderExists(hash);
}

auto HeaderOracle::Shared::GetCheckpoint() const noexcept -> block::Position
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return get_checkpoint(data);
}

auto HeaderOracle::Shared::get_checkpoint(
    const HeaderOraclePrivate& data) const noexcept -> block::Position
{
    return data.database_.CurrentCheckpoint();
}

auto HeaderOracle::Shared::GetDefaultCheckpoint() const noexcept
    -> CheckpointData
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return get_default_checkpoint(data);
}

auto HeaderOracle::Shared::get_default_checkpoint(
    const HeaderOraclePrivate& data) const noexcept -> CheckpointData
{
    return get_default_checkpoint(data.chain_);
}

auto HeaderOracle::Shared::get_default_checkpoint(
    const blockchain::Type chain) const noexcept -> CheckpointData
{
    const auto& data = params::get(chain);
    const auto& position = data.CheckpointPosition();

    return CheckpointData{
        position.height_,
        position.hash_,
        data.CheckpointPrevious().hash_,
        data.CheckpointCfheader()};
}

auto HeaderOracle::Shared::GetJob(alloc::Default alloc) const noexcept
    -> HeaderJob
{
    auto handle = data_.lock();
    auto& data = *handle;

    if (data.JobIsAvailable()) {
        data.have_outstanding_job_ = true;

        return std::make_unique<HeaderJob::Imp>(
            true,
            recent_hashes(data, alloc),
            std::addressof(data.api_),
            data.endpoint_);
    } else {

        return {};
    }
}

auto HeaderOracle::Shared::GetPosition(
    const block::Height height) const noexcept -> block::Position
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return get_position(data, height);
}

auto HeaderOracle::Shared::GetPosition(
    const HeaderOraclePrivate& data,
    const block::Height height) const noexcept -> block::Position
{
    return get_position(data, height);
}

auto HeaderOracle::Shared::get_position(
    const HeaderOraclePrivate& data,
    const block::Height height) const noexcept -> block::Position
{
    auto hash = best_hash(data, height);

    if (hash == blank_hash()) {

        return blank_position();
    } else {

        return {height, std::move(hash)};
    }
}

auto HeaderOracle::Shared::Init() noexcept -> void
{
    auto handle = data_.lock();
    auto& data = *handle;
    const auto& null = blank_position();
    const auto existingCheckpoint = get_checkpoint(data);
    const auto& [existingHeight, existingBlockHash] = existingCheckpoint;
    const auto defaultCheckpoint = get_default_checkpoint(data);
    const auto& [defaultHeight, defaultBlockhash, defaultParenthash, defaultFilterhash] =
        defaultCheckpoint;

    // A checkpoint has been set that is newer than the default
    if (existingHeight > defaultHeight) { return; }

    // The existing checkpoint matches the default checkpoint
    if ((existingHeight == defaultHeight) &&
        (existingBlockHash == defaultBlockhash)) {
        return;
    }

    // Remove existing checkpoint if it is set
    if (existingHeight != null.height_) {
        LogConsole()(print(data.chain_))(
            ": Removing obsolete checkpoint at height ")(existingHeight)
            .Flush();
        const auto deleted = delete_checkpoint(data);

        assert_true(deleted);
    }

    if (1 < defaultHeight) {
        LogConsole()(print(data.chain_))(": Updating checkpoint to hash ")(
            defaultBlockhash.asHex())(" at height ")(defaultHeight)
            .Flush();

        const auto added =
            add_checkpoint(data, defaultHeight, defaultBlockhash);

        assert_true(added);
    }
}

auto HeaderOracle::Shared::initialize_candidate(
    const HeaderOraclePrivate& data,
    const block::Header& best,
    const block::Header& parent,
    UpdateTransaction& update,
    Candidates& candidates,
    block::Header& child,
    const block::Hash& stopHash) noexcept(false) -> Candidate&
{
    const auto blacklisted = connect_to_parent(data, update, parent, child);
    auto position{parent.Position()};
    auto& output = candidates.emplace_back(Candidate{blacklisted, {}});
    auto& chain = output.chain_;
    const block::Header* grandparent = &parent;
    using StopFunction = std::function<bool(const block::Position&)>;
    auto run =
        stopHash.IsNull() ? StopFunction{[&update](const auto& in) -> bool {
            return update.EffectiveBestBlock(in.height_) != in.hash_;
        }}
                          : StopFunction{[&stopHash](const auto& in) -> bool {
                                return stopHash != in.hash_;
                            }};

    while (run(position)) {
        assert_true(0 <= position.height_);
        assert_true(grandparent);

        chain.insert(chain.begin(), position);
        grandparent = &update.Stage(grandparent->ParentHash());
        position = grandparent->Position();
    }

    if (0 == chain.size()) { chain.emplace_back(position); }

    assert_true(0 < chain.size());

    return output;
}

auto HeaderOracle::Shared::is_disconnected(
    const block::Hash& parent,
    UpdateTransaction& update) noexcept -> const block::Header*
{
    try {
        const auto& header = update.Stage(parent);

        if (header.Internal().IsDisconnected()) {

            return nullptr;
        } else {

            return &header;
        }
    } catch (...) {

        return nullptr;
    }
}

auto HeaderOracle::Shared::IsInBestChain(const block::Hash& hash) const noexcept
    -> bool
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return is_in_best_chain(data, hash).first;
}

auto HeaderOracle::Shared::IsInBestChain(
    const block::Position& position) const noexcept -> bool
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return is_in_best_chain(data, position.height_, position.hash_);
}

auto HeaderOracle::Shared::is_in_best_chain(
    const HeaderOraclePrivate& data,
    const block::Hash& hash) const noexcept -> std::pair<bool, block::Height>
{
    const auto header = data.database_.TryLoadHeader(hash);

    if (false == header.IsValid()) { return {false, -1}; }

    return {is_in_best_chain(data, header.Height(), hash), header.Height()};
}

auto HeaderOracle::Shared::is_in_best_chain(
    const HeaderOraclePrivate& data,
    const block::Position& position) const noexcept -> bool
{
    return is_in_best_chain(data, position.height_, position.hash_);
}

auto HeaderOracle::Shared::is_in_best_chain(
    const HeaderOraclePrivate& data,
    const block::Height height,
    const block::Hash& hash) const noexcept -> bool
{
    try {
        return hash == data.database_.BestBlock(height);

    } catch (...) {

        return false;
    }
}

auto HeaderOracle::Shared::IsSynchronized() const noexcept -> bool
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return is_synchronized(data);
}

auto HeaderOracle::Shared::is_synchronized(
    const HeaderOraclePrivate& data) const noexcept -> bool
{
    return data.IsSynchronized();
}

auto HeaderOracle::Shared::LoadHeader(const block::Hash& hash) const noexcept
    -> block::Header
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return data.database_.TryLoadHeader(hash);
}

auto HeaderOracle::Shared::ProcessSyncData(
    block::Hash& prior,
    Vector<block::Hash>& hashes,
    const network::otdht::Data& in) noexcept -> std::size_t
{
    auto handle = data_.lock();
    auto& data = *handle;
    auto output = 0_uz;
    auto update = UpdateTransaction{data.api_, data.database_};
    data.UpdateRemoteHeight(in.State().Position().height_);

    try {
        const auto& blocks = in.Blocks();

        if (blocks.empty()) { std::runtime_error{"No blocks in sync data"}; }

        auto previous = [&]() -> block::Hash {
            const auto& first = blocks.front();
            const auto height = first.Height();

            if (0 >= height) {

                return block::Hash{};
            } else {
                const auto rc =
                    prior.Assign(data.database_.BestBlock(height - 1));

                assert_true(rc);

                return prior;
            }
        }();

        for (const auto& block : blocks) {
            auto header = data.api_.Factory().BlockHeaderFromNative(
                block.Chain(), block.Header(), get_allocator());

            if (false == header.IsValid()) {
                throw std::runtime_error{"Invalid header"};
            }

            if (header.ParentHash() != previous) {
                throw std::runtime_error{"Non-contiguous headers"};
            }

            auto hash = block::Hash{header.Hash()};

            if (false == is_in_best_chain(data, hash).first) {
                if (false == add_header(data, update, std::move(header))) {
                    throw std::runtime_error{"Failed to process header"};
                }
            }

            ++output;
            hashes.emplace_back(hash);
            previous = std::move(hash);
        }
    } catch (const std::exception& e) {
        LogVerbose()()(e.what()).Flush();
    }

    if ((0_uz < output) && apply_update(data, update)) {
        assert_true(output == hashes.size());

        return output;
    } else {

        return 0;
    }
}

auto HeaderOracle::Shared::RecentHashes(alloc::Default alloc) const noexcept
    -> Hashes
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return recent_hashes(data, alloc);
}

auto HeaderOracle::Shared::recent_hashes(
    const HeaderOraclePrivate& data,
    alloc::Default alloc) const noexcept -> Hashes
{
    return data.database_.RecentHashes(alloc);
}

auto HeaderOracle::Shared::Report() noexcept -> void
{
    data_.lock()->database_.ReportHeaderTip();
}

auto HeaderOracle::Shared::Siblings() const noexcept
    -> UnallocatedSet<block::Hash>
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;

    return data.database_.SiblingHashes();
}

auto HeaderOracle::Shared::stage_candidate(
    const HeaderOraclePrivate& data,
    const block::Header& best,
    Candidates& candidates,
    UpdateTransaction& update,
    block::Header& child) noexcept(false) -> void
{
    const auto position = best.Height() + 1;

    if (child.Height() < position) {

        return;
    } else if (child.Height() == position) {
        candidates.emplace_back(Candidate{false, {child.Position()}});
    } else {
        auto& candidate = initialize_candidate(
            data,
            best,
            update.Stage(child.ParentHash()),
            update,
            candidates,
            child,
            best.Hash());
        candidate.chain_.emplace_back(child.Position());
        const auto first = candidate.chain_.cbegin()->height_;

        assert_true(position == first);
    }
}

auto HeaderOracle::Shared::SubmitBlock(const ReadView in) noexcept -> void
{
    auto handle = data_.lock_shared();
    const auto& data = *handle;
    AddHeader(data.api_.Factory().BlockHeaderFromNative(
        data.chain_, in, get_allocator()));
}

auto HeaderOracle::Shared::Target() const noexcept -> block::Height
{
    return data_.lock_shared()->Target();
}

HeaderOracle::Shared::~Shared() = default;
}  // namespace opentxs::blockchain::node::internal
