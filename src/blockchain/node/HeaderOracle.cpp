// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "blockchain/node/HeaderOracle.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <functional>
#include <iterator>
#include <map>
#include <stdexcept>
#include <type_traits>

#include "blockchain/node/UpdateTransaction.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/node/Factory.hpp"
#include "internal/core/Core.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/p2p/Block.hpp"
#include "opentxs/network/p2p/Data.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto HeaderOracle(
    const api::Session& api,
    const blockchain::node::internal::HeaderDatabase& database,
    const blockchain::Type type) noexcept
    -> std::unique_ptr<blockchain::node::HeaderOracle>
{
    using ReturnType = blockchain::node::implementation::HeaderOracle;

    return std::make_unique<ReturnType>(api, database, type);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::node
{
auto HeaderOracle::GenesisBlockHash(const blockchain::Type type)
    -> const block::Hash&
{
    static std::mutex lock_{};
    static auto cache = std::pmr::map<blockchain::Type, block::pHash>{};

    try {
        auto lock = Lock{lock_};
        {
            auto it = cache.find(type);

            if (cache.end() != it) { return it->second; }
        }

        const auto& data = params::Data::Chains().at(type);
        const auto [it, added] = cache.emplace(
            type, Data::Factory(data.genesis_hash_hex_, Data::Mode::Hex));

        return it->second;
    } catch (...) {
        LogError()("opentxs::factory::")(__func__)(": Genesis hash not found")
            .Flush();

        throw;
    }
}
}  // namespace opentxs::blockchain::node

namespace opentxs::blockchain::node::implementation
{
HeaderOracle::HeaderOracle(
    const api::Session& api,
    const internal::HeaderDatabase& database,
    const blockchain::Type type) noexcept
    : internal::HeaderOracle()
    , api_(api)
    , database_(database)
    , chain_(type)
    , lock_()
{
    auto lock = Lock{lock_};
    const auto best = best_chain(lock);

    OT_ASSERT(0 <= best.first);
}

auto HeaderOracle::Ancestors(
    const block::Position& start,
    const block::Position& target,
    const std::size_t limit) const noexcept(false) -> Positions
{
    auto lock = Lock{lock_};
    const auto check =
        std::max<block::Height>(std::min(start.first, target.first), 0);
    const auto fast = is_in_best_chain(lock, target.second).first &&
                      is_in_best_chain(lock, start.second).first &&
                      (start.first < target.first);

    if (fast) {
        auto output = best_chain(lock, start, limit);
        lock.unlock();

        while ((1 < output.size()) && (output.back().first > target.first)) {
            output.pop_back();
        }

        OT_ASSERT(0 < output.size());
        OT_ASSERT(output.front().first <= check);

        return output;
    }

    auto cache = std::pmr::deque<block::Position>{};
    auto current = database_.LoadHeader(target.second);
    auto sibling = database_.LoadHeader(start.second);

    while (sibling->Height() > current->Height()) {
        sibling = database_.TryLoadHeader(sibling->ParentHash());

        if (false == bool(sibling)) {
            sibling = database_.TryLoadHeader(GenesisBlockHash(chain_));

            OT_ASSERT(sibling);

            break;
        }
    }

    OT_ASSERT(sibling->Height() <= current->Height());

    while (current->Height() >= 0) {
        cache.emplace_front(current->Position());

        if (current->Position() == sibling->Position()) {
            break;
        } else if (current->Height() == sibling->Height()) {
            sibling = database_.TryLoadHeader(sibling->ParentHash());

            if (false == bool(sibling)) {
                sibling = database_.TryLoadHeader(GenesisBlockHash(chain_));

                OT_ASSERT(sibling);
            }
        }

        current = database_.TryLoadHeader(current->ParentHash());

        if (false == bool(current)) { break; }
    }

    OT_ASSERT(0 < cache.size());

    auto output = Positions{};
    std::move(cache.begin(), cache.end(), std::back_inserter(output));

    OT_ASSERT(output.front().first <= check);

    return output;
}

auto HeaderOracle::AddCheckpoint(
    const block::Height position,
    const block::Hash& requiredHash) noexcept -> bool
{
    auto lock = Lock{lock_};
    auto update = UpdateTransaction{api_, database_};

    if (update.EffectiveCheckpoint()) {
        LogError()(OT_PRETTY_CLASS())("Checkpoint already exists").Flush();

        return false;
    }

    if (2 > position) {
        LogError()(OT_PRETTY_CLASS())("Invalid position").Flush();

        return false;
    }

    update.SetCheckpoint({position, requiredHash});

    if (apply_checkpoint(lock, position, update)) {

        return database_.ApplyUpdate(update);
    } else {

        return false;
    }
}

auto HeaderOracle::AddHeader(std::unique_ptr<block::Header> header) noexcept
    -> bool
{
    auto headers = std::pmr::vector<std::unique_ptr<block::Header>>{};
    headers.emplace_back(std::move(header));

    return AddHeaders(headers);
}

auto HeaderOracle::AddHeaders(
    std::pmr::vector<std::unique_ptr<block::Header>>& headers) noexcept -> bool
{
    if (0 == headers.size()) { return false; }

    auto lock = Lock{lock_};
    auto update = UpdateTransaction{api_, database_};

    for (auto& header : headers) {
        if (false == bool(header)) {
            LogError()(OT_PRETTY_CLASS())("Invalid header").Flush();

            return false;
        }

        if (false == add_header(lock, update, std::move(header))) {

            return false;
        }
    }

    return database_.ApplyUpdate(update);
}

auto HeaderOracle::add_header(
    const Lock& lock,
    UpdateTransaction& update,
    std::unique_ptr<block::Header> pHeader) noexcept -> bool
{
    if (update.EffectiveHeaderExists(pHeader->Hash())) {
        LogVerbose()(OT_PRETTY_CLASS())("Header already processed").Flush();

        return true;
    }

    auto& header = update.Stage(std::move(pHeader));
    const auto& current = update.Stage();
    const auto [currentHeight, currentHash] = current.Position();
    const auto* pParent = is_disconnected(header.ParentHash(), update);

    if (nullptr == pParent) {
        LogVerbose()(OT_PRETTY_CLASS())("Adding disconnected header").Flush();
        header.SetDisconnectedState();
        update.DisconnectBlock(header);

        return true;
    }

    OT_ASSERT(nullptr != pParent);

    const auto& parent = *pParent;

    if (update.EffectiveIsSibling(header.ParentHash())) {
        update.RemoveSibling(header.ParentHash());
    }

    auto candidates = Candidates{};

    try {
        auto& candidate = initialize_candidate(
            lock, current, parent, update, candidates, header);
        connect_children(lock, header, candidates, candidate, update);
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Failed to connect children").Flush();

        return false;
    }

    return choose_candidate(current, candidates, update).first;
}

auto HeaderOracle::apply_checkpoint(
    const Lock& lock,
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
        stage_candidate(lock, ancestor, candidates, update, best);
        LogConsole()("  * ")(count)(" remaining").Flush();

        for (const auto& hash : siblings) {
            stage_candidate(
                lock, ancestor, candidates, update, update.Stage(hash));
            LogConsole()("  * ")(--count)(" remaining").Flush();
        }

        for (auto& [invalid, chain] : candidates) {
            const block::Header* pParent = &ancestor;

            for (const auto& [height, hash] : chain) {
                auto& child = update.Header(hash);
                invalid = connect_to_parent(lock, update, *pParent, child);
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
        LogError()(OT_PRETTY_CLASS())("Failed to process sibling chains")
            .Flush();

        return false;
    }
}

auto HeaderOracle::best_chain(const Lock& lock) const noexcept
    -> block::Position
{
    return database_.CurrentBest()->Position();
}

auto HeaderOracle::BestChain() const noexcept -> block::Position
{
    auto lock = Lock{lock_};

    return best_chain(lock);
}

auto HeaderOracle::BestChain(
    const block::Position& tip,
    const std::size_t limit) const noexcept(false) -> Positions
{
    auto lock = Lock{lock_};

    return best_chain(lock, tip, limit);
}

auto HeaderOracle::best_chain(
    const Lock& lock,
    const block::Position& tip,
    const std::size_t limit) const noexcept -> Positions
{
    const auto [youngest, best] = common_parent(lock, tip);
    static const auto blank = api_.Factory().Data();
    auto height{youngest.first};
    auto output = Positions{};

    for (auto& hash : best_hashes(lock, height, blank, 0)) {
        output.emplace_back(height++, std::move(hash));

        if ((0u < limit) && (output.size() == limit)) { break; }
    }

    return output;
}

auto HeaderOracle::BestHash(const block::Height height) const noexcept
    -> block::pHash
{
    auto lock = Lock{lock_};

    return best_hash(lock, height);
}

auto HeaderOracle::best_hash(const Lock& lock, const block::Height height)
    const noexcept -> block::pHash
{
    try {
        return database_.BestBlock(height);
    } catch (...) {
        return blank_hash();
    }
}

auto HeaderOracle::BestHash(
    const block::Height height,
    const block::Position& check) const noexcept -> block::pHash
{
    auto lock = Lock{lock_};

    if (is_in_best_chain(lock, check)) {

        return database_.BestBlock(height);
    } else {

        return blank_hash();
    }
}

auto HeaderOracle::BestHashes(
    const block::Height start,
    const std::size_t limit) const noexcept -> Hashes
{
    static const auto blank = api_.Factory().Data();

    auto lock = Lock{lock_};

    return best_hashes(lock, start, blank, limit);
}

auto HeaderOracle::BestHashes(
    const block::Height start,
    const block::Hash& stop,
    const std::size_t limit) const noexcept -> Hashes
{
    auto lock = Lock{lock_};

    return best_hashes(lock, start, stop, limit);
}

auto HeaderOracle::BestHashes(
    const Hashes& previous,
    const block::Hash& stop,
    const std::size_t limit) const noexcept -> Hashes
{
    auto lock = Lock{lock_};
    auto start = std::size_t{0};

    for (const auto& hash : previous) {
        const auto [best, height] = is_in_best_chain(lock, hash);

        if (best) {
            start = height;
            break;
        }
    }

    return best_hashes(lock, start, stop, limit);
}

auto HeaderOracle::best_hashes(
    const Lock& lock,
    const block::Height start,
    const block::Hash& stop,
    const std::size_t limit) const noexcept -> Hashes
{
    auto output = Hashes{};
    const auto limitIsZero = (0 == limit);
    auto current{start};
    const auto last{
        current + static_cast<block::Height>(limit) -
        static_cast<block::Height>(1)};

    while (limitIsZero || (current <= last)) {
        try {
            auto hash = database_.BestBlock(current++);

            // TODO this check shouldn't be necessary but BestBlock doesn't
            // throw the exception documented in its declaration.
            if (hash->empty()) { break; }

            const auto stopHere = stop.empty() ? false : (stop == hash);
            output.emplace_back(std::move(hash));

            if (stopHere) { break; }
        } catch (...) {
            break;
        }
    }

    return output;
}

auto HeaderOracle::blank_hash() const noexcept -> const block::pHash&
{
    static const auto blank = make_blank<block::pHash>::value(api_);

    OT_ASSERT(0 == blank->size());
    OT_ASSERT(blank->empty());

    return blank;
}

auto HeaderOracle::blank_position() const noexcept -> const block::Position&
{
    static const auto blank = make_blank<block::Position>::value(api_);

    return blank;
}

auto HeaderOracle::CalculateReorg(const block::Position& tip) const
    noexcept(false) -> Positions
{
    auto lock = Lock{lock_};

    return calculate_reorg(lock, tip);
}

auto HeaderOracle::calculate_reorg(const Lock& lock, const block::Position& tip)
    const noexcept(false) -> Positions
{
    auto output = Positions{};

    if (is_in_best_chain(lock, tip)) { return output; }

    output.emplace_back(tip);

    for (auto height{tip.first}; height >= 0; --height) {
        if (0 == height) {
            throw std::runtime_error(
                "Provided tip does not connect to genesis block");
        }

        const auto& child = *output.crbegin();
        const auto pHeader = database_.TryLoadHeader(child.second);

        if (false == bool(pHeader)) {
            throw std::runtime_error("Failed to load block header");
        }

        const auto& header = *pHeader;

        if (height != header.Height()) {
            throw std::runtime_error("Wrong height specified for block hash");
        }

        auto parent = block::Position{height - 1, header.ParentHash()};

        if (is_in_best_chain(lock, parent)) { break; }

        output.emplace_back(std::move(parent));
    }

    return output;
}

auto HeaderOracle::choose_candidate(
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

            OT_ASSERT(0 < candidate.chain_.size());

            const auto& position = *candidate.chain_.crbegin();
            const auto& tip = update.Header(position.second);

            if (evaluate_candidate(*pBest, tip)) { pBest = &tip; }
        }

        OT_ASSERT(nullptr != pBest);

        const auto& best = *pBest;

        for (const auto& candidate : candidates) {
            OT_ASSERT(0 < candidate.chain_.size());

            const auto& position = *candidate.chain_.crbegin();
            const auto& tip = update.Header(position.second);

            if (tip.Hash() == best.Hash()) {
                found = true;
                auto reorg{false};

                for (const auto& segment : candidate.chain_) {
                    const auto& [height, hash] = segment;

                    if ((height <= current.Height()) && (false == reorg)) {
                        if (hash.get() == update.EffectiveBestBlock(height)) {
                            continue;
                        } else {
                            reorg = true;
                            const auto parent = block::Position{
                                height - 1,
                                update.EffectiveBestBlock(height - 1)};
                            update.SetReorgParent(parent);
                            update.AddToBestChain(segment);
                            update.AddSibling(current.Position());
                            LogVerbose()(OT_PRETTY_CLASS())("Block ")(
                                hash->asHex())(" at position ")(
                                height)(" causes a chain reorg.")
                                .Flush();
                        }
                    } else {
                        update.AddToBestChain(segment);
                        LogVerbose()(OT_PRETTY_CLASS())("Adding block ")(
                            hash->asHex())(" to best chain at position ")(
                            height)
                            .Flush();
                    }
                }
            } else {
                const auto orphan = tip.Position();
                update.AddSibling(orphan);
                const auto& [height, hash] = orphan;
                LogVerbose()(OT_PRETTY_CLASS())("Adding block ")(hash->asHex())(
                    " as an orphan at position ")(height)
                    .Flush();
            }
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Error evaluating candidates").Flush();

        return output;
    }

    success = true;

    return output;
}

auto HeaderOracle::CommonParent(const block::Position& position) const noexcept
    -> std::pair<block::Position, block::Position>
{
    auto lock = Lock{lock_};

    return common_parent(lock, position);
}

auto HeaderOracle::common_parent(
    const Lock& lock,
    const block::Position& position) const noexcept
    -> std::pair<block::Position, block::Position>
{
    const auto& database = database_;
    std::pair<block::Position, block::Position> output{
        {0, GenesisBlockHash(chain_)}, best_chain(lock)};
    auto& [parent, best] = output;
    auto test{position};
    auto pHeader = database.TryLoadHeader(test.second);

    if (false == bool(pHeader)) { return output; }

    while (0 < test.first) {
        if (is_in_best_chain(lock, test.second).first) {
            parent = test;

            return output;
        }

        pHeader = database.TryLoadHeader(pHeader->ParentHash());

        if (pHeader) {
            test = pHeader->Position();
        } else {
            return output;
        }
    }

    return output;
}

auto HeaderOracle::connect_children(
    const Lock& lock,
    block::Header& parent,
    Candidates& candidates,
    Candidate& candidate,
    UpdateTransaction& update) -> void
{
    auto& chain = candidate.chain_;
    auto& end = *chain.crbegin();

    OT_ASSERT(end.first + 1 == parent.Position().first);

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
        candidate.blacklisted_ = connect_to_parent(lock, update, parent, child);
        // The first child block extends the current candidate. Subsequent child
        // blocks create a new candidate to extend. This transforms the tree
        // of disconnected blocks into a table of candidates.
        auto& chainToExtend = firstChild.exchange(false)
                                  ? candidate
                                  : candidates.emplace_back(original);
        connect_children(lock, child, candidates, chainToExtend, update);
    });
}

auto HeaderOracle::connect_to_parent(
    const Lock& lock,
    const UpdateTransaction& update,
    const block::Header& parent,
    block::Header& child) noexcept -> bool
{
    child.InheritWork(parent.Work());
    child.InheritState(parent);
    child.InheritHeight(parent);
    child.CompareToCheckpoint(update.Checkpoint());

    return child.IsBlacklisted();
}

auto HeaderOracle::DeleteCheckpoint() noexcept -> bool
{
    auto lock = Lock{lock_};
    auto update = UpdateTransaction{api_, database_};

    if (false == update.EffectiveCheckpoint()) {
        LogError()(OT_PRETTY_CLASS())("No checkpoint").Flush();

        return false;
    }

    const auto position = update.Checkpoint().first;
    update.ClearCheckpoint();

    if (apply_checkpoint(lock, position, update)) {

        return database_.ApplyUpdate(update);
    } else {

        return false;
    }
}

auto HeaderOracle::evaluate_candidate(
    const block::Header& current,
    const block::Header& candidate) noexcept -> bool
{
    return candidate.Work() > current.Work();
}

auto HeaderOracle::GetDefaultCheckpoint() const noexcept -> CheckpointData
{
    const auto& checkpoint = params::Data::Chains().at(chain_).checkpoint_;

    return CheckpointData{
        checkpoint.height_,
        api_.Factory().Data(checkpoint.block_hash_, StringStyle::Hex),
        api_.Factory().Data(checkpoint.previous_block_hash_, StringStyle::Hex),
        api_.Factory().Data(checkpoint.filter_header_, StringStyle::Hex)};
}

auto HeaderOracle::GetCheckpoint() const noexcept -> block::Position
{
    auto lock = Lock{lock_};

    return database_.CurrentCheckpoint();
}

auto HeaderOracle::GetPosition(const block::Height height) const noexcept
    -> block::Position
{
    auto lock = Lock{lock_};

    return get_position(lock, height);
}

auto HeaderOracle::get_position(const Lock& lock, const block::Height height)
    const noexcept -> block::Position
{
    auto hash = best_hash(lock, height);

    if (hash == blank_hash()) {

        return blank_position();
    } else {

        return {height, std::move(hash)};
    }
}

auto HeaderOracle::Init() noexcept -> void
{
    const auto& null = blank_position();
    const auto existingCheckpoint = GetCheckpoint();
    const auto& [existingHeight, existingBlockHash] = existingCheckpoint;
    const auto defaultCheckpoint = GetDefaultCheckpoint();
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
    if (existingHeight != null.first) {
        LogConsole()(DisplayString(chain_))(
            ": Removing obsolete checkpoint at height ")(existingHeight)
            .Flush();
        const auto deleted = DeleteCheckpoint();

        OT_ASSERT(deleted);
    }

    if (1 < defaultHeight) {
        LogConsole()(DisplayString(chain_))(": Updating checkpoint to hash ")(
            defaultBlockhash->asHex())(" at height ")(defaultHeight)
            .Flush();

        const auto added = AddCheckpoint(defaultHeight, defaultBlockhash);

        OT_ASSERT(added);
    }
}

auto HeaderOracle::initialize_candidate(
    const Lock& lock,
    const block::Header& best,
    const block::Header& parent,
    UpdateTransaction& update,
    Candidates& candidates,
    block::Header& child,
    const block::Hash& stopHash) noexcept(false) -> Candidate&
{
    const auto blacklisted = connect_to_parent(lock, update, parent, child);
    auto position{parent.Position()};
    auto& output = candidates.emplace_back(Candidate{blacklisted, {}});
    auto& chain = output.chain_;
    const block::Header* grandparent = &parent;
    using StopFunction = std::function<bool(const block::Position&)>;
    auto run =
        stopHash.empty() ? StopFunction{[&update](const auto& in) -> bool {
            return update.EffectiveBestBlock(in.first) != in.second;
        }}
                         : StopFunction{[&stopHash](const auto& in) -> bool {
                               return stopHash != in.second;
                           }};

    while (run(position)) {
        OT_ASSERT(0 <= position.first);
        OT_ASSERT(grandparent);

        chain.insert(chain.begin(), position);
        grandparent = &update.Stage(grandparent->ParentHash());
        position = grandparent->Position();
    }

    if (0 == chain.size()) { chain.emplace_back(position); }

    OT_ASSERT(0 < chain.size());

    return output;
}

auto HeaderOracle::IsInBestChain(const block::Hash& hash) const noexcept -> bool
{
    auto lock = Lock{lock_};

    return is_in_best_chain(lock, hash).first;
}

auto HeaderOracle::IsInBestChain(const block::Position& position) const noexcept
    -> bool
{
    auto lock = Lock{lock_};

    return is_in_best_chain(lock, position.first, position.second);
}

auto HeaderOracle::is_disconnected(
    const block::Hash& parent,
    UpdateTransaction& update) noexcept -> const block::Header*
{
    try {
        const auto& header = update.Stage(parent);

        if (header.IsDisconnected()) {

            return nullptr;
        } else {

            return &header;
        }
    } catch (...) {

        return nullptr;
    }
}

auto HeaderOracle::is_in_best_chain(const Lock& lock, const block::Hash& hash)
    const noexcept -> std::pair<bool, block::Height>
{
    const auto pHeader = database_.TryLoadHeader(hash);

    if (false == bool(pHeader)) { return {false, -1}; }

    const auto& header = *pHeader;

    return {is_in_best_chain(lock, header.Height(), hash), header.Height()};
}

auto HeaderOracle::is_in_best_chain(
    const Lock& lock,
    const block::Position& position) const noexcept -> bool
{
    return is_in_best_chain(lock, position.first, position.second);
}

auto HeaderOracle::is_in_best_chain(
    const Lock& lock,
    const block::Height height,
    const block::Hash& hash) const noexcept -> bool
{
    try {
        return hash == database_.BestBlock(height);

    } catch (...) {

        return false;
    }
}

auto HeaderOracle::LoadBitcoinHeader(const block::Hash& hash) const noexcept
    -> std::unique_ptr<block::bitcoin::Header>
{
    return database_.TryLoadBitcoinHeader(hash);
}

auto HeaderOracle::LoadHeader(const block::Hash& hash) const noexcept
    -> std::unique_ptr<block::Header>
{
    return database_.TryLoadHeader(hash);
}

auto HeaderOracle::ProcessSyncData(
    block::Hash& prior,
    std::pmr::vector<block::pHash>& hashes,
    const network::p2p::Data& data) noexcept -> std::size_t
{
    auto output = std::size_t{0};
    auto update = UpdateTransaction{api_, database_};

    try {
        const auto& blocks = data.Blocks();

        if (0u == blocks.size()) {
            std::runtime_error{"No blocks in sync data"};
        }

        auto lock = Lock{lock_};
        auto previous = [&]() -> block::pHash {
            const auto& first = blocks.front();
            const auto height = first.Height();

            if (0 >= height) {

                return block::BlankHash();
            } else {
                prior.Assign(database_.BestBlock(height - 1));

                return prior;
            }
        }();

        for (const auto& block : blocks) {
            auto pHeader = factory::BitcoinBlockHeader(
                api_, block.Chain(), block.Header());

            if (false == bool(pHeader)) {
                throw std::runtime_error{"Invalid header"};
            }

            const auto& header = *pHeader;

            if (header.ParentHash() != previous) {
                throw std::runtime_error{"Non-contiguous headers"};
            }

            auto hash = block::pHash{header.Hash()};

            if (false == is_in_best_chain(lock, hash).first) {
                if (false == add_header(lock, update, std::move(pHeader))) {
                    throw std::runtime_error{"Failed to process header"};
                }
            }

            ++output;
            hashes.emplace_back(hash);
            previous = std::move(hash);
        }
    } catch (const std::exception& e) {
        LogVerbose()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    if ((0u < output) && database_.ApplyUpdate(update)) {
        OT_ASSERT(output == hashes.size());

        return output;
    } else {

        return 0;
    }
}

auto HeaderOracle::stage_candidate(
    const Lock& lock,
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
            lock,
            best,
            update.Stage(child.ParentHash()),
            update,
            candidates,
            child,
            best.Hash());
        candidate.chain_.emplace_back(child.Position());
        const auto first = candidate.chain_.cbegin()->first;

        OT_ASSERT(position == first);
    }
}

auto HeaderOracle::Siblings() const noexcept -> std::pmr::set<block::pHash>
{
    auto lock = Lock{lock_};

    return database_.SiblingHashes();
}
}  // namespace opentxs::blockchain::node::implementation
