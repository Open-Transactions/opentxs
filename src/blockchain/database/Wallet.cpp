// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "blockchain/database/Wallet.hpp"  // IWYU pragma: associated

#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <algorithm>
#include <iterator>
#include <map>
#include <numeric>
#include <tuple>
#include <type_traits>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD "opentxs::blockchain::database::Wallet::"

template <typename Key>
auto delete_from_vector(std::vector<Key>& vector, const Key& key) noexcept
    -> bool
{
    auto found{false};

    for (auto i{vector.begin()}; i != vector.end();) {
        if (key == *i) {
            i = vector.erase(i);
            found = true;
        } else {
            ++i;
        }
    }

    return found;
}

namespace opentxs::blockchain::database
{
Wallet::Wallet(
    const api::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const blockchain::Type chain) noexcept
    : api_(api)
    , blockchain_(blockchain)
    , chain_(chain)
    , lock_()
    , patterns_()
    , subchain_pattern_index_()
    , subchain_last_indexed_()
    , subchain_version_()
    , subchain_last_scanned_()
    , subchain_last_processed_()
    , match_index_()
    , outputs_()
    , output_positions_()
    , output_states_()
    , transactions_()
    , tx_to_block_()
    , block_to_tx_()
{
}

auto Wallet::add_transaction(
    const Lock& lock,
    const block::Hash& block,
    const block::bitcoin::Transaction& transaction) const noexcept -> bool
{
    if (auto it = transactions_.find(transaction.ID());
        transactions_.end() != it) {
        auto& serialized = it->second;
        transaction.MergeMetadata(serialized);
        auto updated = transaction.Serialize();

        OT_ASSERT(updated.has_value());

        serialized = std::move(updated.value());
    } else {
        auto serialized = transaction.Serialize();

        OT_ASSERT(serialized.has_value());

        transactions_.emplace(transaction.ID(), std::move(serialized.value()));
    }

    {
        auto& index = tx_to_block_[transaction.ID()];
        index.emplace_back(block);
        dedup(index);
    }

    {
        auto& index = block_to_tx_[block];
        index.emplace_back(transaction.ID());
        dedup(index);
    }

    return true;
}

auto Wallet::AddConfirmedTransaction(
    const block::Position& block,
    const std::vector<std::uint32_t> outputIndices,
    const block::bitcoin::Transaction& transaction) const noexcept -> bool
{
    Lock lock(lock_);
    const auto& [height, blockHash] = block;

    if (false == add_transaction(lock, blockHash, transaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error adding transaction to database")
            .Flush();

        return false;
    }

    for (const auto& input : transaction.Inputs()) {
        const auto& outpoint = input.PreviousOutput();

        if (auto out = find_output(lock, outpoint); out.has_value()) {
            if (false ==
                change_state(lock, outpoint, State::ConfirmedSpend, block)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error updating consumed output state")
                    .Flush();

                return false;
            }
        }

        // NOTE consider the case of parallel chain scanning where one
        // transaction spends inputs that belong to two different subchains.
        // The first subchain to find the transaction will recognize the inputs
        // belonging to itself but might miss the inputs belonging to the other
        // subchain if the other subchain's scanning process has not yet
        // discovered those outputs.
        // This is fine. The other scanning process will parse this transaction
        // again and at that point all inputs will be recognized. The only
        // impact is that net balance change of the transaction will
        // underestimated temporarily until scanning is complete for all
        // subchains.
    }

    for (const auto index : outputIndices) {
        const auto outpoint =
            block::bitcoin::Outpoint{transaction.ID().Bytes(), index};

        if (auto out = find_output(lock, outpoint); out.has_value()) {
            if (false ==
                change_state(lock, outpoint, State::ConfirmedNew, block)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error updating created output state")
                    .Flush();

                return false;
            }
        } else {
            if (false == create_state(
                             lock,
                             outpoint,
                             State::ConfirmedNew,
                             block,
                             transaction.Outputs().at(index))) {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Error created new output state")
                    .Flush();

                return false;
            }
        }
    }

    blockchain_.UpdateBalance(chain_, get_balance(lock));

    return true;
}

auto Wallet::change_state(
    const Lock& lock,
    const block::bitcoin::Outpoint& id,
    const State newState,
    const block::Position newPosition) const noexcept -> bool
{
    auto itOutput = outputs_.find(id);

    if (outputs_.end() == itOutput) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Outpoint does not exist in db")
            .Flush();

        return false;
    }

    auto& [outpointState, outpointPosition, data] = itOutput->second;

    if (false == delete_from_vector(output_states_[outpointState], id)) {
        // Repair database inconsistency

        for (auto& [state, vector] : output_states_) {
            if (state == outpointState) { continue; }

            delete_from_vector(vector, id);
        }
    }

    {
        outpointState = newState;
        auto& vector = output_states_[outpointState];
        vector.emplace_back(id);
        dedup(vector);
    }

    if (false == delete_from_vector(output_positions_[outpointPosition], id)) {
        // Repair database inconsistency

        for (auto& [position, vector] : output_positions_) {
            if (position == outpointPosition) { continue; }

            delete_from_vector(vector, id);
        }
    }

    {
        outpointPosition = effective_position(newState, newPosition);
        auto& vector = output_positions_[outpointPosition];
        vector.emplace_back(id);
        dedup(vector);
    }

    return true;
}

auto Wallet::create_state(
    const Lock& lock,
    const block::bitcoin::Outpoint& id,
    const State state,
    const block::Position position,
    const block::bitcoin::Output& output) const noexcept -> bool
{
    if (0 < outputs_.count(id)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Outpoint already exists in db")
            .Flush();

        return false;
    }

    auto data = block::bitcoin::Output::SerializeType{};

    if (false == output.Serialize(data)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to serialize output")
            .Flush();

        return false;
    }

    const auto& effectivePosition = effective_position(state, position);
    outputs_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple(state, effectivePosition, std::move(data)));

    {
        auto& vector = output_states_[state];
        vector.emplace_back(id);
        dedup(vector);
    }

    {
        auto& vector = output_positions_[effectivePosition];
        vector.emplace_back(id);
        dedup(vector);
    }

    return true;
}

auto Wallet::effective_position(
    const State state,
    const block::Position& position) const noexcept -> const block::Position&
{
    static const auto blankPosition = make_blank<block::Position>::value(api_);

    return ((State::UnconfirmedNew == state) ||
            (State::UnconfirmedSpend == state))
               ? blankPosition
               : position;
}

auto Wallet::find_output(const Lock& lock, const block::bitcoin::Outpoint& id)
    const noexcept -> std::optional<OutputMap::iterator>
{
    auto result = outputs_.find(id);

    if (outputs_.end() == result) {

        return {};
    } else {

        return result;
    }
}

auto Wallet::get_balance(const Lock&) const noexcept -> Balance
{
    auto output = Balance{};
    auto& [confirmed, unconfirmed] = output;
    auto cb = [this](const auto previous, const auto& outpoint) -> auto
    {
        const auto& [state, position, data] = outputs_.at(outpoint);

        return previous + data.value();
    };
    const auto& confirmedNew = output_states_[State::ConfirmedNew];
    confirmed = std::accumulate(
        std::begin(confirmedNew), std::end(confirmedNew), std::uint64_t{0}, cb);
    const auto& unconfirmedNew = output_states_[State::UnconfirmedNew];
    unconfirmed = std::accumulate(
        std::begin(unconfirmedNew), std::end(unconfirmedNew), confirmed, cb);

    return output;
}

auto Wallet::get_patterns(
    const Lock& lock,
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept(false) -> const IDSet&
{
    return subchain_pattern_index_.at(
        subchain_id(balanceNode, subchain, type, version));
}

auto Wallet::GetBalance() const noexcept -> Balance
{
    Lock lock(lock_);

    return get_balance(lock);
}

auto Wallet::GetPatterns(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> Patterns
{
    Lock lock(lock_);

    try {
        const auto& patterns =
            get_patterns(lock, balanceNode, subchain, type, version);

        return load_patterns(lock, balanceNode, subchain, patterns);
    } catch (...) {

        return {};
    }
}

auto Wallet::GetUnspentOutputs() const noexcept -> std::vector<UTXO>
{
    Lock lock(lock_);

    return get_unspent_outputs(lock);
}

auto Wallet::get_unspent_outputs(const Lock& lock) const noexcept
    -> std::vector<UTXO>
{
    auto retrieve = std::vector<block::bitcoin::Outpoint>{};

    for (const auto& outpoint : output_states_[State::UnconfirmedNew]) {
        retrieve.emplace_back(outpoint);
    }

    for (const auto& outpoint : output_states_[State::ConfirmedNew]) {
        retrieve.emplace_back(outpoint);
    }

    for (const auto& outpoint : output_states_[State::UnconfirmedSpend]) {
        retrieve.emplace_back(outpoint);
    }

    dedup(retrieve);
    auto output = std::vector<UTXO>{};

    for (const auto& outpoint : retrieve) {
        const auto& [state, position, data] = outputs_.at(outpoint);
        output.emplace_back(outpoint, data);
    }

    return output;
}

auto Wallet::GetUntestedPatterns(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const ReadView blockID,
    const VersionNumber version) const noexcept -> Patterns
{
    Lock lock(lock_);

    try {
        const auto& allPatterns =
            get_patterns(lock, balanceNode, subchain, type, version);
        auto effectiveIDs = std::vector<pPatternID>{};

        try {
            const auto& matchedPatterns =
                match_index_.at(api_.Factory().Data(blockID));
            std::set_difference(
                std::begin(allPatterns),
                std::end(allPatterns),
                std::begin(matchedPatterns),
                std::end(matchedPatterns),
                std::back_inserter(effectiveIDs));
        } catch (...) {

            return load_patterns(lock, balanceNode, subchain, allPatterns);
        }

        return load_patterns(lock, balanceNode, subchain, effectiveIDs);
    } catch (...) {

        return {};
    }
}

auto Wallet::pattern_id(const SubchainID& subchain, const Bip32Index index)
    const noexcept -> pPatternID
{
    auto preimage = OTData{subchain};
    preimage->Concatenate(&index, sizeof(index));
    auto output = api_.Factory().Identifier();
    output->CalculateDigest(preimage->Bytes());

    return output;
}

auto Wallet::SubchainAddElements(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const ElementMap& elements,
    const VersionNumber version) const noexcept -> bool
{
    Lock lock(lock_);
    subchain_version_[subchain_version_index(balanceNode, subchain, type)] =
        version;
    auto subchainID = subchain_id(balanceNode, subchain, type, version);
    auto newIndices = std::vector<OTIdentifier>{};
    auto highest = Bip32Index{};

    for (const auto& [index, patterns] : elements) {
        auto patternID = pattern_id(subchainID, index);
        auto& vector = patterns_[patternID];
        newIndices.emplace_back(std::move(patternID));
        highest = std::max(highest, index);

        for (const auto& pattern : patterns) {
            vector.emplace_back(index, pattern);
        }
    }

    subchain_last_indexed_[subchainID] = highest;
    auto& index = subchain_pattern_index_[subchainID];

    for (auto& id : newIndices) { index.emplace(std::move(id)); }

    return true;
}

auto Wallet::SubchainDropIndex(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> bool
{
    Lock lock(lock_);
    const auto subchainID = subchain_id(balanceNode, subchain, type, version);

    try {
        for (const auto& patternID : subchain_pattern_index_.at(subchainID)) {
            patterns_.erase(patternID);

            for (auto& [block, set] : match_index_) { set.erase(patternID); }
        }
    } catch (...) {
    }

    subchain_pattern_index_.erase(subchainID);
    subchain_last_indexed_.erase(subchainID);
    subchain_version_.erase(
        subchain_version_index(balanceNode, subchain, type));

    return true;
}

auto Wallet::SubchainIndexVersion(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> VersionNumber
{
    Lock lock(lock_);
    const auto id = subchain_version_index(balanceNode, subchain, type);

    try {

        return subchain_version_.at(id);
    } catch (...) {

        return 0;
    }
}

auto Wallet::SubchainLastIndexed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> std::optional<Bip32Index>
{
    Lock lock(lock_);
    const auto subchainID = subchain_id(balanceNode, subchain, type, version);

    try {
        return subchain_last_indexed_.at(subchainID);
    } catch (...) {
        return {};
    }
}

auto Wallet::SubchainLastProcessed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> block::Position
{
    Lock lock(lock_);

    try {
        return subchain_last_processed_.at(
            subchain_version_index(balanceNode, subchain, type));
    } catch (...) {
        return make_blank<block::Position>::value(api_);
    }
}

auto Wallet::SubchainLastScanned(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> block::Position
{
    Lock lock(lock_);

    try {
        return subchain_last_scanned_.at(
            subchain_version_index(balanceNode, subchain, type));
    } catch (...) {
        return make_blank<block::Position>::value(api_);
    }
}

auto Wallet::SubchainSetLastProcessed(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const block::Position& position) const noexcept -> bool
{
    Lock lock(lock_);
    auto& map = subchain_last_processed_;
    auto id = subchain_version_index(balanceNode, subchain, type);
    auto it = map.find(id);

    if (map.end() == it) {
        map.emplace(std::move(id), position);

        return true;
    } else {
        it->second = position;

        return true;
    }
}

auto Wallet::SubchainMatchBlock(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const MatchingIndices& indices,
    const ReadView blockID,
    const VersionNumber version) const noexcept -> bool
{
    Lock lock(lock_);
    auto& matchSet = match_index_[api_.Factory().Data(blockID)];

    for (const auto& index : indices) {
        matchSet.emplace(pattern_id(
            subchain_id(balanceNode, subchain, type, version), index));
    }

    return true;
}

auto Wallet::SubchainSetLastScanned(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const block::Position& position) const noexcept -> bool
{
    Lock lock(lock_);
    auto& map = subchain_last_scanned_;
    auto id = subchain_version_index(balanceNode, subchain, type);
    auto it = map.find(id);

    if (map.end() == it) {
        map.emplace(std::move(id), position);

        return true;
    } else {
        it->second = position;

        return true;
    }
}

auto Wallet::subchain_version_index(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type) const noexcept -> pSubchainID
{
    auto preimage = OTData{balanceNode};
    preimage->Concatenate(&subchain, sizeof(subchain));
    preimage->Concatenate(&type, sizeof(type));
    auto output = api_.Factory().Identifier();
    output->CalculateDigest(preimage->Bytes());

    return output;
}

auto Wallet::subchain_id(
    const NodeID& balanceNode,
    const Subchain subchain,
    const FilterType type,
    const VersionNumber version) const noexcept -> pSubchainID
{
    auto preimage = OTData{balanceNode};
    preimage->Concatenate(&subchain, sizeof(subchain));
    preimage->Concatenate(&type, sizeof(type));
    preimage->Concatenate(&version, sizeof(version));
    auto output = api_.Factory().Identifier();
    output->CalculateDigest(preimage->Bytes());

    return output;
}
}  // namespace opentxs::blockchain::database
