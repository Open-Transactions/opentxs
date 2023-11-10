// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>

#include "blockchain/database/wallet/OutputCache.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionOutput.pb.h>  // IWYU pragma: keep
#include <boost/container/vector.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/unordered/unordered_node_map.hpp>
#include <algorithm>
#include <chrono>  // IWYU pragma: keep
#include <cstring>
#include <functional>
#include <iterator>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

#include "blockchain/database/wallet/Position.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/TSV.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"  // IWYU pragma: keep
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Pattern.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"  // IWYU pragma: keep
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::database::wallet
{
template <typename MapKeyType, typename MapType>
auto OutputCache::load_output_index(
    const MapKeyType& key,
    MapType& map) noexcept -> Outpoints&
{
    if (auto it = map.find(key); map.end() != it) { return it->second; }

    auto [row, added] = map.try_emplace(key, Outpoints{});

    assert_true(added);

    return row->second;
}

template <typename MapKeyType, typename MapType>
auto OutputCache::load_output_index(const MapKeyType& key, MapType& map)
    const noexcept -> const Outpoints&
{
    if (auto it = map.find(key); map.end() != it) { return it->second; }

    return empty_outputs_;
}
}  // namespace opentxs::blockchain::database::wallet

namespace opentxs::blockchain::database::wallet
{
using namespace std::literals;
using enum storage::lmdb::Dir;

const Outpoints OutputCache::empty_outputs_{};
const Nyms OutputCache::empty_nyms_{};

OutputCache::OutputCache(
    const api::Session& api,
    const storage::lmdb::Database& lmdb,
    const blockchain::Type chain,
    const block::Position& blank) noexcept
    : api_(api)
    , lmdb_(lmdb)
    , chain_(chain)
    , blank_(blank)
    , position_()
    , outputs_()
    , accounts_()
    , keys_()
    , nyms_()
    , nym_list_()
    , positions_()
    , states_()
    , subchains_()
    , generation_outputs_()
    , output_to_proposal_()
    , created_by_proposal_()
    , consumed_by_proposal_()
    , populated_(false)
{
    outputs_.reserve(reserve_);
    keys_.reserve(reserve_);
    positions_.reserve(reserve_);
}

auto OutputCache::AddGenerationOutput(
    block::Height height,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept(false) -> void
{
    auto key = static_cast<std::size_t>(height);
    boost::endian::native_to_little_inplace(key);
    const auto rc = lmdb_.Store(generation_, key, output.Bytes(), tx).first;

    if (false == rc) {
        throw std::runtime_error{
            "failed to save generation index for "s + output.str() +
            " at height "s + std::to_string(height)};
    }

    generation_outputs_.emplace(height, output);
}

auto OutputCache::AddOutput(
    const block::Outpoint& id,
    storage::lmdb::Transaction& tx,
    protocol::bitcoin::base::block::Output output) noexcept -> bool
{
    if (write_output(id, output, tx)) {
        outputs_.try_emplace(id, std::move(output));

        return true;
    } else {
        LogError()()("failed to write output").Flush();

        return false;
    }
}

auto OutputCache::AddOutput(
    const block::Outpoint& id,
    const node::TxoState state,
    const block::Position& position,
    const AccountID& account,
    const SubchainID& subchain,
    storage::lmdb::Transaction& tx,
    protocol::bitcoin::base::block::Output output) noexcept -> bool
{
    assert_false(account.empty());
    assert_false(subchain.empty());

    auto rc = AddOutput(id, tx, std::move(output));

    if (false == rc) { return false; }

    rc &= AddToState(state, id, tx);
    rc &= AddToPosition(position, id, tx);
    rc &= AddToAccount(account, id, tx);
    rc &= AddToSubchain(subchain, id, tx);

    return rc;
}

auto OutputCache::AddToAccount(
    const AccountID& id,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    assert_true(0 < outputs_.count(output));

    try {
        auto& set = load_output_index(id, accounts_);
        auto rc = lmdb_.Store(wallet::accounts_, id.Bytes(), output.Bytes(), tx)
                      .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to update account index"};
        }

        set.emplace(output);

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto OutputCache::AddToKey(
    const crypto::Key& id,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    assert_true(0 < outputs_.count(output));

    const auto key = serialize(id);

    try {
        auto& set = load_output_index(id, keys_);
        auto rc =
            lmdb_.Store(wallet::keys_, reader(key), output.Bytes(), tx).first;

        if (false == rc) {
            throw std::runtime_error{"Failed to update key index"};
        }

        set.emplace(output);

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto OutputCache::AddToNym(
    const identifier::Nym& id,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept(false) -> void
{
    if (id.empty()) { throw std::runtime_error{"invalid nym"}; }

    if (false == outputs_.contains(output)) {
        throw std::runtime_error{
            "output "s.append(output.str()).append(" does not exists")};
    }

    auto& index = load_output_index(id, nyms_);
    auto& list = nym_list_;
    auto rc = lmdb_.Store(wallet::nyms_, id.Bytes(), output.Bytes(), tx).first;

    if (false == rc) { throw std::runtime_error{"Failed to update nym index"}; }

    index.emplace(output);
    list.emplace(id);
}

auto OutputCache::AddToPosition(
    const block::Position& id,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    assert_true(0 < outputs_.count(output));

    const auto key = db::Position{id};

    try {
        auto& set = load_output_index(id, positions_);
        auto rc =
            lmdb_
                .Store(
                    wallet::positions_, reader(key.data_), output.Bytes(), tx)
                .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to update key index"};
        }

        set.emplace(output);

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto OutputCache::AddToState(
    const node::TxoState id,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    assert_true(0 < outputs_.count(output));

    try {
        auto& set = load_output_index(id, states_);
        auto rc = lmdb_
                      .Store(
                          wallet::states_,
                          static_cast<std::size_t>(id),
                          output.Bytes(),
                          tx)
                      .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to update key index"};
        }

        set.emplace(output);

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto OutputCache::AddToSubchain(
    const SubchainID& id,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    assert_true(0 < outputs_.count(output));

    try {
        auto& set = load_output_index(id, subchains_);
        auto rc =
            lmdb_.Store(wallet::subchains_, id.Bytes(), output.Bytes(), tx)
                .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to update subchain index"};
        }

        set.emplace(output);

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto OutputCache::associate_proposal(
    const Log& log,
    const block::Outpoint& output,
    const identifier::Generic& proposal,
    Kind kind,
    storage::lmdb::Transaction& tx) noexcept(false) -> void
{
    const auto& crypto = api_.Crypto();
    auto& map = output_to_proposal_;

    if (const auto i = map.find(output); map.end() != i) {
        const auto allowed = [&] {
            if (Kind::consume != kind) { return false; }

            const auto& other = i->second;
            const auto& index = created_by_proposal_;

            if (const auto j = index.find(other); index.end() != j) {
                const auto created = j->second;

                return created.contains(output);
            } else {

                return false;
            }
        }();

        if (false == allowed) {
            throw std::runtime_error{"output "s.append(output.str())
                                         .append(" already associated with ")
                                         .append(i->second.asBase58(crypto))};
        }
    }

    const auto rc =
        lmdb_.Store(output_proposal_, output.Bytes(), proposal.Bytes(), tx)
            .first;

    if (false == rc) {
        throw std::runtime_error{"failed to associate "s.append(output.str())
                                     .append(" to ")
                                     .append(proposal.asBase58(crypto))};
    }

    map.emplace(output, proposal);
    log()("associated ")((kind == Kind::create) ? "created" : "consumed")(
        " output ")(output.str())(" to proposal ")(proposal, crypto)
        .Flush();
}

auto OutputCache::ChangePosition(
    const block::Position& oldPosition,
    const block::Position& newPosition,
    const block::Outpoint& id,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    try {
        const auto oldP = db::Position{oldPosition};
        const auto newP = db::Position{newPosition};

        if (lmdb_.Exists(wallet::positions_, reader(oldP.data_), id.Bytes())) {
            auto rc = lmdb_.Delete(
                wallet::positions_, reader(oldP.data_), id.Bytes(), tx);

            if (false == rc) {
                throw std::runtime_error{"Failed to remove old position index"};
            }
        } else {
            LogError()()("Warning: position index for ")(id.str())(
                " already removed")
                .Flush();
        }

        auto rc =
            lmdb_.Store(wallet::positions_, reader(newP.data_), id.Bytes(), tx)
                .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to add position state index"};
        }

        auto& from = positions_[oldPosition];
        auto& to = positions_[newPosition];
        from.erase(id);
        to.emplace(id);

        if (0u == from.size()) { positions_.erase(oldPosition); }

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto OutputCache::ChangeState(
    const node::TxoState oldState,
    const node::TxoState newState,
    const block::Outpoint& id,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    try {
        for (const auto state : all_states()) { GetState(state); }

        auto deleted = UnallocatedVector<node::TxoState>{};

        for (const auto state : all_states()) {
            if (lmdb_.Delete(
                    wallet::states_,
                    static_cast<std::size_t>(state),
                    id.Bytes(),
                    tx)) {
                deleted.emplace_back(state);
            }
        }

        if ((0u == deleted.size()) || (oldState != deleted.front())) {
            LogError()()("Warning: state index for ")(id.str())(
                " did not match expected value")
                .Flush();
        }

        if (1u != deleted.size()) {
            LogError()()("Warning: output ")(id.str())(
                " found in multiple state indices")
                .Flush();
        }

        auto rc = lmdb_
                      .Store(
                          wallet::states_,
                          static_cast<std::size_t>(newState),
                          id.Bytes(),
                          tx)
                      .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to add new state index"};
        }

        for (const auto& state : all_states()) {
            if (auto it = states_.find(state); states_.end() != it) {
                auto& from = it->second;
                from.erase(id);

                if (0u == from.size()) { states_.erase(it); }
            }
        }

        auto& to = states_[newState];
        to.emplace(id);

        return rc;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto OutputCache::CheckProposals(
    const FlatSet<identifier::Generic>& proposals,
    alloc::Strategy alloc) noexcept(false) -> Vector<identifier::Generic>
{
    auto out = Vector<identifier::Generic>{alloc.result_};
    out.reserve(proposals.size());
    out.clear();
    const auto is_finished = [this](const auto& id) {
        return this->is_finished(id);
    };
    std::ranges::copy_if(proposals, std::back_inserter(out), is_finished);

    return out;
}

auto OutputCache::Clear() noexcept -> void
{
    position_ = std::nullopt;
    nym_list_.clear();
    outputs_.clear();
    accounts_.clear();
    keys_.clear();
    nyms_.clear();
    positions_.clear();
    states_.clear();
    subchains_.clear();
    populated_ = false;
}

auto OutputCache::ConsumeOutput(
    const Log& log,
    const identifier::Generic& proposal,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept(false) -> void
{
    auto& set = consumed_by_proposal_[proposal];

    if (set.contains(output)) {
        throw std::runtime_error{
            "proposal "s.append(proposal.asBase58(api_.Crypto()))
                .append(" already contains consumed output ")
                .append(output.str())};
    }

    const auto rc =
        lmdb_.Store(proposal_spent_, proposal.Bytes(), output.Bytes(), tx)
            .first;

    if (false == rc) {
        throw std::runtime_error{
            "failed to save consumed output "s.append(output.str())
                .append(" association to proposal ")
                .append(proposal.asBase58(api_.Crypto()))};
    }

    set.emplace(output);
    associate_proposal(log, output, proposal, Kind::consume, tx);
}

auto OutputCache::CreateOutput(
    const Log& log,
    const identifier::Generic& proposal,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept(false) -> void
{
    auto& set = created_by_proposal_[proposal];

    if (set.contains(output)) {
        throw std::runtime_error{
            "proposal "s.append(proposal.asBase58(api_.Crypto()))
                .append(" already contains created output ")
                .append(output.str())};
    }

    const auto rc =
        lmdb_.Store(proposal_created_, proposal.Bytes(), output.Bytes(), tx)
            .first;

    if (false == rc) {
        throw std::runtime_error{
            "failed to save created output "s.append(output.str())
                .append(" association to proposal ")
                .append(proposal.asBase58(api_.Crypto()))};
    }

    set.emplace(output);
    associate_proposal(log, output, proposal, Kind::create, tx);
}

auto OutputCache::DeleteGenerationAbove(
    block::Height good,
    OrphanedGeneration& out,
    storage::lmdb::Transaction& tx) noexcept(false) -> void
{
    auto& map = generation_outputs_;

    if (map.empty()) { return; }

    const auto first = [&]() -> std::optional<Generation::iterator> {
        auto i = map.lower_bound(good);

        if (map.end() == i) { return std::nullopt; }

        if (i->first == good) {

            return std::next(i);
        } else {

            return i;
        }
    }();

    if (false == first.has_value()) { return; }

    const auto delete_output = [&](const auto& item) {
        auto key = static_cast<std::size_t>(item.first);
        boost::endian::native_to_little_inplace(key);

        if (false == lmdb_.Delete(generation_, key, tx)) {
            throw std::runtime_error{
                "failed to delete generation output index for height "s +
                std::to_string(item.first)};
        }

        out.emplace(item.second);
    };
    std::for_each(*first, map.end(), delete_output);
    map.erase(*first, map.end());
}

auto OutputCache::dissociate_proposal(
    const Log& log,
    const block::Outpoint& output,
    const identifier::Generic& proposal,
    storage::lmdb::Transaction& tx) noexcept(false) -> void
{
    const auto& crypto = api_.Crypto();
    auto& map = output_to_proposal_;

    if (const auto i = map.find(output); map.end() != i) {
        const auto& id = i->second;

        if (id != proposal) {
            throw std::runtime_error{
                "expected output "s.append(output.str())
                    .append(" to be associated with ")
                    .append(proposal.asBase58(crypto))
                    .append(" however it is currently registered with ")
                    .append(id.asBase58(crypto))};
        }
    } else {
        throw std::runtime_error{
            output.str().append(" is not associated with any proposal")};
    }

    const auto rc = lmdb_.Delete(output_proposal_, output.Bytes(), tx);

    if (false == rc) {
        throw std::runtime_error{"failed to update outpoint - proposal index"};
    }

    map.erase(output);
    log()("dissociating output ")(output.str())(" from proposal ")(
        proposal, crypto)
        .Flush();
}

auto OutputCache::Exists(const block::Outpoint& id) const noexcept -> bool
{
    return 0 < outputs_.count(id);
}

auto OutputCache::Exists(const SubchainID& subchain, const block::Outpoint& id)
    const noexcept -> bool
{
    if (auto it = subchains_.find(subchain); subchains_.end() != it) {
        const auto& set = it->second;

        return 0 < set.count(id);
    } else {

        return false;
    }
}

auto OutputCache::FinishProposal(
    const Log& log,
    const identifier::Generic& proposal,
    storage::lmdb::Transaction& tx) noexcept(false) -> void
{
    auto& createdMap = created_by_proposal_;
    auto& createdSet = createdMap[proposal];
    auto& consumedMap = consumed_by_proposal_;
    auto& consumedSet = consumedMap[proposal];
    auto purge = [&, this](auto table, const auto& output) {
        const auto rc =
            lmdb_.Delete(table, proposal.Bytes(), output.Bytes(), tx);

        if (false == rc) {
            throw std::runtime_error{
                "failed to dissociate output "s.append(output.str())
                    .append(" from proposal ")
                    .append(proposal.asBase58(api_.Crypto()))};
        }

        dissociate_proposal(log, output, proposal, tx);
    };

    for (auto i = createdSet.begin(); i != createdSet.end();) {
        purge(proposal_created_, *i);
        i = createdSet.erase(i);
    }

    for (auto i = consumedSet.begin(); i != consumedSet.end();) {
        purge(proposal_spent_, *i);
        i = consumedSet.erase(i);
    }

    if (false == createdSet.empty()) {
        throw std::runtime_error{
            "proposal "s.append(proposal.asBase58(api_.Crypto()))
                .append(" still has ")
                .append(std::to_string(createdSet.size()))
                .append("created outputs unaccounted for")};
    }

    if (false == createdSet.empty()) {
        throw std::runtime_error{
            "proposal "s.append(proposal.asBase58(api_.Crypto()))
                .append(" still has ")
                .append(std::to_string(createdSet.size()))
                .append("consumed outputs unaccounted for")};
    }

    createdMap.erase(proposal);
    consumedMap.erase(proposal);
}

auto OutputCache::GetAccount(const AccountID& id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, accounts_);
}

auto OutputCache::GetAssociation(
    const identifier::Generic& proposal,
    const block::Outpoint& output) const noexcept -> ProposalAssociation
{
    constexpr auto check = [](const auto& map, const auto& p, const auto& o) {
        if (const auto i = map.find(p); map.end() != i) {

            return i->second.contains(o);
        } else {

            return false;
        }
    };
    const auto spends = check(consumed_by_proposal_, proposal, output);
    const auto creates = check(created_by_proposal_, proposal, output);
    using enum ProposalAssociation;

    if ((false == spends) && (false == creates)) {

        return none;
    } else if (spends && (false == creates)) {

        return consumed;
    } else if ((false == spends) && creates) {

        return created;
    } else {
        LogAbort()()("irrecoverable database corruption: proposal ")(
            proposal,
            api_.Crypto())(" both creates and consumes output ")(output)
            .Abort();
    }
}

auto OutputCache::GetConsumed(const identifier::Generic& id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, consumed_by_proposal_);
}

auto OutputCache::GetCreated(const identifier::Generic& id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, created_by_proposal_);
}

auto OutputCache::GetKey(const crypto::Key& id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, keys_);
}

auto OutputCache::GetHeight() const noexcept -> block::Height
{
    return get_position().Height();
}

auto OutputCache::GetMatured(
    block::Height first,
    block::Height last,
    alloc::Strategy alloc) const noexcept -> Vector<block::Outpoint>
{
    const auto start = generation_outputs_.lower_bound(first);
    const auto limit = generation_outputs_.upper_bound(last);
    auto out = Vector<block::Outpoint>{alloc.result_};
    out.clear();
    constexpr auto get_value = [](const auto& data) { return data.second; };
    using namespace std::ranges;
    transform(subrange{start, limit}, std::back_inserter(out), get_value);

    return out;
}

auto OutputCache::GetNym(const identifier::Nym& id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, nyms_);
}

auto OutputCache::GetNyms() const noexcept -> const Nyms& { return nym_list_; }

auto OutputCache::GetOutput(const block::Outpoint& id) const noexcept(false)
    -> const protocol::bitcoin::base::block::Output&
{
    return load_output(id);
}

auto OutputCache::GetOutput(const block::Outpoint& id) noexcept(false)
    -> protocol::bitcoin::base::block::Output&
{
    return load_output(id);
}

auto OutputCache::GetOutput(
    const SubchainID& subchain,
    const block::Outpoint& id) noexcept(false)
    -> protocol::bitcoin::base::block::Output&
{
    const auto& relevant = GetSubchain(subchain);

    if (0u == relevant.count(id)) {
        throw std::out_of_range{"outpoint not found in this subchain"};
    }

    return GetOutput(id);
}

auto OutputCache::GetPosition() const noexcept -> const db::Position&
{
    return get_position();
}

auto OutputCache::GetPosition(const block::Position& id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, positions_);
}

auto OutputCache::get_position() const noexcept -> const db::Position&
{
    if (position_.has_value()) {

        return position_.value();
    } else {
        static const auto null = db::Position{blank_};

        return null;
    }
}

auto OutputCache::GetProposals(const block::Outpoint& id, alloc::Strategy alloc)
    const noexcept -> Vector<identifier::Generic>
{
    const auto [start, limit] = output_to_proposal_.equal_range(id);
    auto out = Vector<identifier::Generic>{alloc.result_};
    out.reserve(std::distance(start, limit));
    out.clear();
    constexpr auto get_value = [](const auto& item) { return item.second; };
    std::transform(start, limit, std::back_inserter(out), get_value);

    return out;
}

auto OutputCache::GetReserved(alloc::Strategy alloc) const noexcept -> Reserved
{
    const auto& map = consumed_by_proposal_;
    auto out = Reserved{alloc.result_};
    out.clear();
    const auto get_values = [&](const auto& item) {
        const auto& [key, value] = item;
        std::ranges::copy(value, std::inserter(out, out.end()));
    };
    std::ranges::for_each(map, get_values);

    return out;
}

auto OutputCache::GetReserved(
    const identifier::Generic& proposal,
    alloc::Strategy alloc) const noexcept -> Vector<UTXO>
{
    auto out = Vector<UTXO>{alloc.result_};
    const auto& map = consumed_by_proposal_;

    if (auto i = map.find(proposal); map.end() == i) {
        out.clear();
    } else {
        try {
            const auto& set = i->second;
            out.reserve(set.size());
            out.clear();
            const auto make_utxo = [&, this](const auto& id) {
                const auto& output = outputs_.at(id);

                return std::make_pair(id, output);
            };
            std::ranges::transform(set, std::back_inserter(out), make_utxo);
        } catch (const std::exception& e) {
            LogAbort()()(
                "fatal database corruption: an output referenced by proposal ")(
                proposal, api_.Crypto())(" does not exist in index: ")(e.what())
                .Abort();
        }
    }

    return out;
}

auto OutputCache::GetState(const node::TxoState id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, states_);
}

auto OutputCache::GetSubchain(const SubchainID& id) const noexcept
    -> const Outpoints&
{
    return load_output_index(id, subchains_);
}

auto OutputCache::is_finished(const identifier::Generic& proposal) noexcept
    -> bool
{
    const auto check = [](auto& map, const auto& id) {
        if (auto i = map.find(id); map.end() != i) {

            return i->second.empty();
        } else {

            return true;
        }
    };
    const auto consumed = check(consumed_by_proposal_, proposal);
    const auto created = check(created_by_proposal_, proposal);

    if (consumed && created) {
        consumed_by_proposal_.erase(proposal);
        created_by_proposal_.erase(proposal);

        return true;
    } else {

        return false;
    }
}

auto OutputCache::load_output(const block::Outpoint& id) const noexcept(false)
    -> const protocol::bitcoin::base::block::Output&
{
    auto it = outputs_.find(id);

    if (outputs_.end() != it) {
        auto& out = it->second;

        assert_true(0 < out.Keys({}).size());  // TODO monotonic allocator

        return out;
    }

    throw std::out_of_range{"output "s + id.str() + " not found"};
}

auto OutputCache::Populate() noexcept -> void
{
    if (populated_) { return; }

    auto outputCount = 0_uz;
    const auto outputs = [&](const auto key, const auto value) {
        ++outputCount;
        outputs_.try_emplace(
            key,
            factory::BitcoinTransactionOutput(
                api_.Crypto().Blockchain(),
                api_.Factory(),
                chain_,
                proto::Factory<proto::BlockchainTransactionOutput>(value),
                {}  // TODO allocator
                ));

        return true;
    };
    const auto accounts = [&](const auto key, const auto value) {
        auto& map = accounts_;
        auto id = [&] {
            auto out = identifier::Generic{};
            out.Assign(key);

            return out;
        }();
        auto& set = map[std::move(id)];
        set.emplace(value);

        return true;
    };
    const auto keys = [&](const auto key, const auto value) {
        auto& map = keys_;
        auto& set = map[deserialize(key)];
        set.emplace(value);

        return true;
    };
    const auto nyms = [&](const auto key, const auto value) {
        auto& map = nyms_;
        auto id = [&] {
            auto out = identifier::Nym{};
            out.Assign(key);

            return out;
        }();
        nym_list_.emplace(id);
        auto& set = map[std::move(id)];
        set.emplace(value);

        return true;
    };
    const auto positions = [&](const auto key, const auto value) {
        auto& map = positions_;
        auto& set = map[db::Position{key}.Decode(api_)];
        set.emplace(value);

        return true;
    };
    const auto states = [&](const auto key, const auto value) {
        auto& map = states_;
        auto id = [&] {
            auto out = 0_uz;
            std::memcpy(&out, key.data(), std::min(key.size(), sizeof(out)));

            return static_cast<node::TxoState>(out);
        }();
        auto& set = map[std::move(id)];
        set.emplace(value);

        return true;
    };
    const auto subchains = [&](const auto key, const auto value) {
        auto& map = subchains_;
        auto id = [&] {
            auto out = identifier::Generic{};
            out.Assign(key);

            return out;
        }();
        auto& set = map[std::move(id)];
        set.emplace(value);

        return true;
    };
    const auto generation = [&](const auto key, const auto value) {
        auto& map = generation_outputs_;
        const auto height = [&] {
            auto out = block::Height{};
            boost::endian::little_to_native_inplace(out);
            std::memcpy(
                std::addressof(out),
                key.data(),
                std::min(key.size(), sizeof(out)));

            return out;
        }();
        map.emplace(height, value);

        return true;
    };
    const auto output_proposal = [&](const auto key, const auto value) {
        auto& map = output_to_proposal_;
        map.emplace(key, api_.Factory().IdentifierFromHash(value));

        return true;
    };
    const auto proposal_created = [&](const auto key, const auto value) {
        auto& map = created_by_proposal_;
        auto id = [&] {
            auto out = identifier::Generic{};
            out.Assign(key);

            return out;
        }();
        auto& set = map[std::move(id)];
        set.emplace(value);

        return true;
    };
    const auto proposal_consumed = [&](const auto key, const auto value) {
        auto& map = consumed_by_proposal_;
        auto id = [&] {
            auto out = identifier::Generic{};
            out.Assign(key);

            return out;
        }();
        auto& set = map[std::move(id)];
        set.emplace(value);

        return true;
    };
    auto tx = lmdb_.TransactionRO();
    auto rc = lmdb_.Read(wallet::outputs_, outputs, Forward, tx);

    assert_true(rc);

    rc = lmdb_.Read(wallet::accounts_, accounts, Forward, tx);

    assert_true(rc);

    rc = lmdb_.Read(wallet::keys_, keys, Forward, tx);

    assert_true(rc);

    rc = lmdb_.Read(wallet::nyms_, nyms, Forward, tx);

    assert_true(rc);

    rc = lmdb_.Read(wallet::positions_, positions, Forward, tx);

    assert_true(rc);

    rc = lmdb_.Read(wallet::states_, states, Forward, tx);

    assert_true(rc);

    rc = lmdb_.Read(wallet::subchains_, subchains, Forward, tx);

    assert_true(rc);

    rc = lmdb_.Read(wallet::generation_, generation, Forward, tx);

    assert_true(rc);

    rc = lmdb_.Read(wallet::output_proposal_, output_proposal, Forward, tx);

    assert_true(rc);

    rc = lmdb_.Read(wallet::proposal_created_, proposal_created, Forward, tx);

    assert_true(rc);

    rc = lmdb_.Read(wallet::proposal_spent_, proposal_consumed, Forward, tx);

    assert_true(rc);

    if (lmdb_.Exists(
            wallet::output_config_, tsv(database::Key::WalletPosition), tx)) {
        rc = lmdb_.Load(
            wallet::output_config_,
            tsv(database::Key::WalletPosition),
            [&](const auto bytes) { position_.emplace(bytes); },
            tx);

        assert_true(rc);
    }

    assert_true(outputs_.size() == outputCount);

    populated_ = true;
}

auto OutputCache::ProposalConfirmConsumed(
    const Log& log,
    const identifier::Generic& proposal,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept(false) -> void
{
    auto& set = [&, this]() -> auto& {
        auto& map = consumed_by_proposal_;

        if (auto i = map.find(proposal); map.end() != i) {

            return i->second;
        } else {

            throw std::runtime_error{
                "consumed outputs for proposal "s
                    .append(proposal.asBase58(api_.Crypto()))
                    .append(" not found")};
        }
    }();

    if (false == set.contains(output)) {
        throw std::runtime_error{"output "s.append(output.str())
                                     .append(" is not consumed by proposal ")
                                     .append(proposal.asBase58(api_.Crypto()))};
    }

    const auto rc =
        lmdb_.Delete(proposal_spent_, proposal.Bytes(), output.Bytes(), tx);

    if (false == rc) {
        throw std::runtime_error{
            "failed to delete consumed output "s.append(output.str())
                .append(" association to proposal ")
                .append(proposal.asBase58(api_.Crypto()))};
    }

    dissociate_proposal(log, output, proposal, tx);
    set.erase(output);
}

auto OutputCache::ProposalConfirmCreated(
    const Log& log,
    const identifier::Generic& proposal,
    const block::Outpoint& output,
    storage::lmdb::Transaction& tx) noexcept(false) -> void
{
    auto& set = [&, this]() -> auto& {
        auto& map = created_by_proposal_;

        if (auto i = map.find(proposal); map.end() != i) {

            return i->second;
        } else {

            throw std::runtime_error{
                "created outputs for proposal "s
                    .append(proposal.asBase58(api_.Crypto()))
                    .append(" not found")};
        }
    }();

    if (false == set.contains(output)) {
        throw std::runtime_error{"output "s.append(output.str())
                                     .append(" is not created by proposal ")
                                     .append(proposal.asBase58(api_.Crypto()))};
    }

    const auto rc =
        lmdb_.Delete(proposal_created_, proposal.Bytes(), output.Bytes(), tx);

    if (false == rc) {
        throw std::runtime_error{
            "failed to delete created output "s.append(output.str())
                .append(" association to proposal ")
                .append(proposal.asBase58(api_.Crypto()))};
    }

    dissociate_proposal(log, output, proposal, tx);
    set.erase(output);
}

auto OutputCache::Print() const noexcept -> void
{
    struct PrintOutput {
        std::stringstream text_{};
        Amount total_{};
    };
    auto output = UnallocatedMap<node::TxoState, PrintOutput>{};

    const auto& definition = blockchain::GetDefinition(chain_);

    for (const auto& data : outputs_) {
        const auto& outpoint = data.first;
        const auto& item = data.second.Internal();
        auto& out = output[item.State()];
        out.text_ << "\n * " << outpoint.str() << ' ';
        out.text_ << " value: " << definition.Format(item.Value());
        out.total_ += item.Value();
        const auto& script = item.Script();
        out.text_ << ", type: ";
        using enum protocol::bitcoin::base::block::script::Pattern;

        switch (script.Type()) {
            case PayToMultisig: {
                out.text_ << "P2MS";
            } break;
            case PayToPubkey: {
                out.text_ << "P2PK";
            } break;
            case PayToPubkeyHash: {
                out.text_ << "P2PKH";
            } break;
            case PayToScriptHash: {
                out.text_ << "P2SH";
            } break;
            default: {
                out.text_ << "unknown";
            }
        }

        out.text_ << ", state: " << print(item.State());
    }

    const auto& unconfirmed = output[node::TxoState::UnconfirmedNew];
    const auto& confirmed = output[node::TxoState::ConfirmedNew];
    const auto& pending = output[node::TxoState::UnconfirmedSpend];
    const auto& spent = output[node::TxoState::ConfirmedSpend];
    const auto& orphan = output[node::TxoState::OrphanedNew];
    const auto& outgoingOrphan = output[node::TxoState::OrphanedSpend];
    const auto& immature = output[node::TxoState::Immature];
    auto& log = LogConsole();
    log()("Instance ")(api_.Instance())(" TXO database contents:").Flush();
    log()("Unconfirmed available value: ")(unconfirmed.total_)(
        unconfirmed.text_.str())
        .Flush();
    log()("Confirmed available value: ")(confirmed.total_)(
        confirmed.text_.str())
        .Flush();
    log()("Unconfirmed spent value: ")(pending.total_)(pending.text_.str())
        .Flush();
    log()("Confirmed spent value: ")(spent.total_)(spent.text_.str()).Flush();
    log()("Orphaned incoming value: ")(orphan.total_)(orphan.text_.str())
        .Flush();
    log()("Orphaned spend value: ")(outgoingOrphan.total_)(
        outgoingOrphan.text_.str())
        .Flush();
    log()("Immature value: ")(immature.total_)(immature.text_.str()).Flush();

    log()("Outputs by block:\n");

    for (const auto& [position, outputs] : positions_) {
        log("  * block ")(position)("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log()("Outputs by nym:\n");

    for (const auto& [id, outputs] : nyms_) {
        log("  * ")(id, api_.Crypto())("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log()("Outputs by subaccount:\n");

    for (const auto& [id, outputs] : accounts_) {
        log("  * ")(id, api_.Crypto())("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log()("Outputs by subchain:\n");

    for (const auto& [id, outputs] : subchains_) {
        log("  * ")(id, api_.Crypto())("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log()("Outputs by key:\n");

    for (const auto& [key, outputs] : keys_) {
        log("  * ")(print(key, api_.Crypto()))("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log()("Outputs by state:\n");

    for (const auto& [state, outputs] : states_) {
        log("  * ")(print(state))("\n");

        for (const auto& outpoint : outputs) {
            log("    * ")(outpoint.str())("\n");
        }
    }

    log.Flush();
    log()("Generation outputs:\n");

    for (const auto& [height, outpoint] : generation_outputs_) {
        log("  *  ")(outpoint.str())(" at height ")(height)("\n");
    }

    log.Flush();
}

auto OutputCache::Release(
    const Log& log,
    const block::Outpoint& output,
    const identifier::Generic& proposal,
    storage::lmdb::Transaction& tx) noexcept(false) -> void
{
    auto& set = consumed_by_proposal_[proposal];

    if (set.contains(output)) {
        throw std::runtime_error{
            "proposal "s.append(proposal.asBase58(api_.Crypto()))
                .append(" does not contain consumed output ")
                .append(output.str())};
    }

    const auto rc =
        lmdb_.Delete(proposal_spent_, proposal.Bytes(), output.Bytes(), tx);

    if (false == rc) {
        throw std::runtime_error{
            "failed to delete consumed output "s.append(output.str())
                .append(" association to proposal ")
                .append(proposal.asBase58(api_.Crypto()))};
    }

    dissociate_proposal(log, output, proposal, tx);
}

auto OutputCache::UpdateOutput(
    const block::Outpoint& id,
    const protocol::bitcoin::base::block::Output& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    return write_output(id, output, tx);
}

auto OutputCache::UpdatePosition(
    const block::Position& pos,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    try {
        const auto serialized = db::Position{pos};
        const auto rc = lmdb_
                            .Store(
                                wallet::output_config_,
                                tsv(database::Key::WalletPosition),
                                reader(serialized.data_),
                                tx)
                            .first;

        if (false == rc) {
            throw std::runtime_error{"Failed to update wallet position"};
        }

        position_.emplace(pos);

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto OutputCache::write_output(
    const block::Outpoint& id,
    const protocol::bitcoin::base::block::Output& output,
    storage::lmdb::Transaction& tx) noexcept -> bool
{
    try {
        // TODO monotonic allocator
        for (const auto& key : output.Keys({})) {
            const auto sKey = serialize(key);
            auto rc =
                lmdb_.Store(wallet::keys_, reader(sKey), id.Bytes(), tx).first;

            if (false == rc) {
                throw std::runtime_error{"update to key index"};
            }

            auto& cache = [&]() -> Outpoints& {
                if (auto it = keys_.find(key); keys_.end() != it) {

                    return it->second;
                }

                return keys_[key];
            }();
            cache.emplace(id);
        }

        const auto serialized = [&] {
            auto out = Space{};
            const auto data = [&] {
                auto proto = protocol::bitcoin::base::block::internal::Output::
                    SerializeType{};
                const auto rc = output.Internal().Serialize(api_, proto);

                if (false == rc) {
                    throw std::runtime_error{"failed to serialize as protobuf"};
                }

                return proto;
            }();

            if (false == proto::write(data, writer(out))) {
                throw std::runtime_error{"failed to serialize as bytes"};
            }

            return out;
        }();

        return lmdb_.Store(wallet::outputs_, id.Bytes(), reader(serialized), tx)
            .first;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

OutputCache::~OutputCache() = default;
}  // namespace opentxs::blockchain::database::wallet
